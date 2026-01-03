#pragma once
#include<unordered_map>
#include<map>
#include<vector>
#include<memory>
#include<span>
#include<functional>
#include<string_view>

#include <cstddef>
#include <cstdint>

#include "log.hpp"
namespace std {
  namespace filesystem {

  }
  namespace fs = std::filesystem;
}

using i8 = int8_t;
using u8 = int8_t;
 
using i16 = int16_t;
using u16 = uint16_t;

using i32 = int32_t; 
using u32 = uint32_t;

using i64 = int64_t;
using u64 = uint64_t;

using f32 = float; 
using f64 = double;

#ifndef NO_UNIQUE_ADDRESS
  #if defined(__clang__) && defined(_MSC_VER)
    #define NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
  #elif defined(_MSC_VER)
    #define NO_UNIQUE_ADDRESS [[msvc::no_unique_address]] [[no_unique_address]]
  #elif defined(__GNUC__) || defined(__clang__)
    #define NO_UNIQUE_ADDRESS [[no_unique_address]]
  #else
    #define NO_UNIQUE_ADDRESS
  #endif
#else
  #error "NO_UNIQUE_ADDRESS redefinition" 
#endif

#ifndef INL
  #if defined(__GNUC__) || defined(__clang__)
    #define INL __attribute__((always_inline)) inline
  #elif defined(_MSC_VER)
    #define INL [[msvc::forceinline]]
  #else
    #define INL inline
  #endif
#endif

#ifndef FORCE_INLINE
  #if defined(_MSC_VER)
    #define FORCE_INLINE [[msvc::forceinline]]
  #elif defined(__GNUC__) || defined(__clang__)
    #define FORCE_INLINE [[gnu::forceinline]] inline
  #endif
#else
  #error "FORCE_INLINE redefinition" 
#endif

template<typename T, typename A = std::allocator<T>>
using Vec = std::vector<T, A>;

template<typename T, typename D = std::default_delete<T>>
using Unique = std::unique_ptr<T, D>;

template<typename T>
using Shared = std::shared_ptr<T>;

template<typename K, typename V>
using HashMap = std::unordered_map<K, V>;

template<typename K, typename V>
using Map = std::map<K, V>;

template<typename F>
using Fn = std::function<F>;

template<typename T>
using Span = std::span<T>;