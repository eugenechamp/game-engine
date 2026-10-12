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
#include <glm/fwd.hpp>

#include <core/function.hpp>
#include <core/vector.hpp>
#include <core/def.hpp>

#include "log.hpp"

namespace std {
  namespace filesystem {
  
  }
  
  namespace fs = std::filesystem;
}

using i8 = int8_t;
using u8 = int8_t;
 
using vec2 = glm::vec2;
using vec3 = glm::vec3;
using vec4 = glm::vec4;
using mat2 = glm::mat2;
using mat3 = glm::mat3;
using mat4 = glm::mat4;
using quat = glm::quat;

using au8  = std::atomic<u8>;
using au16 = std::atomic<u16>;
using au32 = std::atomic<u32>;
using au64 = std::atomic<u64>;

using ai8  = std::atomic<i8>;
using ai16 = std::atomic<i16>;
using ai32 = std::atomic<i32>;
using ai64 = std::atomic<i64>;

using abool = std::atomic_bool;

//template<typename T, typename A = std::allocator<T>>
//using Vec = std::vector<T, A>;
//
//template<typename F>
//using Fn = std::function<F>;

template<typename T, typename D = std::default_delete<T>>
using Unique = std::unique_ptr<T, D>;

template<typename T>
using Shared = std::shared_ptr<T>;

template<typename K, typename V>
using HashMap = std::unordered_map<K, V>;

template<typename K, typename V>
using Map = std::map<K, V>;

template<typename T>
using Span = std::span<T>;

template<typename T>
using Viewer = Span<const T>;

template<typename... Ts>
struct Pack {};