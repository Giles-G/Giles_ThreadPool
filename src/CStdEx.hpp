/***************************
@File: CStdEx.h
***************************/

#ifndef CSTDEX_HPP
#define CSTDEX_HPP

#include <memory>
#include <type_traits>

// 兼容 std::enable_if_t 的语法
template<bool B, typename T = void>
using c_enable_if_t = typename std::enable_if<B, T>::type;

// 兼容 std::make_unique 的语法
template<typename T, typename... Args>
typename std::unique_ptr<T> c_make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

#endif 
