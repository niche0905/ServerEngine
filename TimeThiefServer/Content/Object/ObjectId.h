#pragma once

/*-------------
   ObjectId
-------------*/
//
// ObjectId는 오브젝트를 고유하게 식별하는 ID입니다.
// 상위 16비트는 생성 세대(Generation)를 나타내고, 하위 16비트는 인덱스(Index)를 나타냅니다.
//

struct ObjectId
{
    uint32 value = 0;
    
    static constexpr uint32 INDEX_BITS  = 16;
    static constexpr uint32 GEN_BITS    = 16;
    
    uint16 Index()  const { return static_cast<uint16>(value & 0xFFFFu); }
    uint16 Gen()    const { return static_cast<uint16>((value >> INDEX_BITS) & 0xFFFFu); }
    
    static ObjectId Make(uint16 index, uint16 gen)
    {
        ObjectId id;
        id.value = (static_cast<uint32>(gen) << INDEX_BITS) | static_cast<uint32>(index);
        return id;
    }
    
    explicit operator bool() const { return value != 0; }
    bool operator==(const ObjectId& other) const { return value == other.value; }
    bool operator!=(const ObjectId& other) const { return value != other.value; }
};

namespace std
{
    template <>
    struct hash<ObjectId>
    {
        size_t operator()(const ObjectId& id) const noexcept
        {
            return std::hash<uint32>{}(id.value);
        }
    };
}
