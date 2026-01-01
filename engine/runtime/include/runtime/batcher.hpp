#pragma once
#include "def.hpp"

template<typename T>
struct Batcher {
public:
  Batcher(Vec<T>& vec)
    : vec_{vec} 
  { }
  
  using element_type = T;

  void reserve(u32 amount) {
    vec_.reserve(amount);
  }
  
  auto size() const { return vec_.size(); }
  
  void push_back(T&& element) {
    return vec_.push_back(std::move(element));
  }
  void push_back(const T& element) {
    return vec_.push_back(element);
  }

  auto& emplace_back(auto&&...args) {
    return vec_.emplace_back(std::forward<decltype(args)>()...);
  }

private:
  Vec<T>& vec_;
};