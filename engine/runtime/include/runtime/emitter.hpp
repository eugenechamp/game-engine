#pragma once
#include "def.hpp"

template<typename T>
struct Emitter {
public:
  using element_type = T;
  using size_type    = Vec<T>::size_type;

  explicit Emitter(Vec<T>& vec)
    : vec_{vec} 
  { }

  void reserve(u32 amount) {
    vec_.reserve(amount);
  }

  void shrink_to_fit() {
    vec_.shrink_to_fit();
  }
  
  auto size() const -> size_type { return vec_.size(); }
  
  void push_back(T&& element) {
    return vec_.push_back(std::move(element));
  }
  
  void push_back(const T& element) {
    return vec_.push_back(element);
  }

  auto& emplace_back(auto&&...args) {
    return vec_.emplace_back(std::forward<decltype(args)>(args)...);
  }

private:
  Vec<T>& vec_;
};