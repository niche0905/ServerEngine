#pragma once
#include <concepts>

template <typename T, typename... Args>
concept PoolObject = requires(T a, Args... args) {
	// PoolObject는 Reset 메서드를 가져야 합니다
    { a.Reset(args...) } -> std::same_as<void>;
};
