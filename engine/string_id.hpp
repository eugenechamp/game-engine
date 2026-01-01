#pragma once
#include"runtime/include/runtime/def.hpp"

class StringID {
public:
  using Type = u32;

public:
  constexpr StringID()  = default;
  constexpr ~StringID() = default;
  
  constexpr explicit StringID(std::string_view str) 
    : type_{  }
  { }

  constexpr StringID(const StringID&)                    = default;
  constexpr StringID(StringID&&)                         = default;
  constexpr auto operator=(const StringID&) -> StringID& = default;
  constexpr auto operator=(StringID&&) -> StringID&      = default;
  
  constexpr bool operator==(const StringID&) const = default;

  constexpr explicit operator Type() {
    return type_;
  }

  constexpr auto id() -> Type {
    return type_;
  }

private:
  Type type_;
};