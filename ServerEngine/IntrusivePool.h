#pragma once
#include <cstddef>
#include <new>
#include <vector>
#include <memory>
#include <utility>

/*-----------------
   IntrusivePool
-----------------*/
//
// IntrusivePool는 T 타입 객체들을 위한 메모리 풀입니다
// T 객체 내부에 별도의 메모리 관리용 필드를 추가하지 않고도 메모리 할당과 해제를 효율적으로 처리할 수 있습니다
//

template <typename T, std::size_t BlockSize = 256>
class IntrusivePool
{
public:
    IntrusivePool() = default;

	// copy constructor/assignment delete
    IntrusivePool(const IntrusivePool&) = delete;
    IntrusivePool& operator=(const IntrusivePool&) = delete;

    ~IntrusivePool() {
        // 아직 살아 있는 객체는 없다는 전제(게임 서버 패턴에선 보통 프로세스 종료 시점)
        for (void* block : blocks_) {
            ::operator delete(block, std::align_val_t(alignof(T)));
        }
    }

public:
    // T 객체를 생성해서 반환 (placement new 포함)
    template <typename... Args>
    T* Acquire(Args&&... args) {
        void* mem = AllocateRaw();
        // C++20
        return std::construct_at(static_cast<T*>(mem), std::forward<Args>(args)...);
    }

    // T 객체를 파괴하고 풀에 반환
    void Release(T* ptr) noexcept {
        if (!ptr) return;
        std::destroy_at(ptr);  // ptr->~T();
        auto* node = reinterpret_cast<FreeNode*>(ptr);
        node->next = freeList_;
        freeList_ = node;
    }

public:
    // 생성자/소멸자 없이 "원시 메모리"만 쓰고 싶을 때
    T* AcquireUninitialized() {
        return static_cast<T*>(AllocateRaw());
    }

    void ReleaseUninitialized(T* ptr) noexcept {
        auto* node = reinterpret_cast<FreeNode*>(ptr);
        node->next = freeList_;
        freeList_ = node;
    }

private:
    struct FreeNode {
        FreeNode* next;
    };

    FreeNode* freeList_ = nullptr;
    std::vector<void*> blocks_;  // 블록 주소들 보관 (나중에 전체 free용)

    void* AllocateRaw() {
        if (!freeList_) {
            AllocateBlock();
        }
        FreeNode* node = freeList_;
        freeList_ = node->next;
        return node;
    }

    void AllocateBlock() {
        constexpr std::size_t sz = sizeof(T);
        constexpr std::size_t align = alignof(T);

        // 정렬 맞춰서 큰 블럭 하나 할당
        std::byte* block = static_cast<std::byte*>(
            ::operator new(sz * BlockSize, std::align_val_t(align))
            );
        blocks_.push_back(block);

        // 이 블럭을 free list 노드들로 쪼개서 연결
        auto* first = reinterpret_cast<FreeNode*>(block);
        FreeNode* current = first;

        for (std::size_t i = 1; i < BlockSize; ++i) {
            auto* next = reinterpret_cast<FreeNode*>(block + i * sz);
            current->next = next;
            current = next;
        }

        // 새 블록 뒤에 기존 free list 연결
        current->next = freeList_;
        freeList_ = first;
    }
};

template <typename T, std::size_t BlockSize = 256>
class ThreadLocalIntrusivePool
{
public:
    template <typename... Args>
    static T* Acquire(Args&&... args) {
        return GetPool().Acquire(std::forward<Args>(args)...);
    }

    static void Release(T* ptr) noexcept {
        GetPool().Release(ptr);
    }

    static IntrusivePool<T, BlockSize>& GetPool() {
        thread_local IntrusivePool<T, BlockSize> pool;
        return pool;
    }
};
