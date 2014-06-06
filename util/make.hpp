#pragma once
#include <utility>

template<template <typename...> class T, typename... Args>
inline T<Args...> make(Args&&... args) {
    return T<Args...>{std::forward<Args>(args)...};
}
