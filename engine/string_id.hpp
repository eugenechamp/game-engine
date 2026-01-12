#pragma once
#include"runtime/include/runtime/def.hpp"

class StringID {
public:
  using Id = u32;

public:
  constexpr StringID()  = default;
  constexpr ~StringID() = default;
  
  constexpr explicit StringID(std::string_view str) 
    : type_{  }
  { }

  static constexpr auto from_string(std::string_view str) -> StringID {
    return {};
  }

  static constexpr auto id_from_string(std::string_view str) -> Id {
    return {};
  }

  constexpr StringID(const StringID&)                    = default;
  constexpr StringID(StringID&&)                         = default;
  constexpr auto operator=(const StringID&) -> StringID& = default;
  constexpr auto operator=(StringID&&) -> StringID&      = default;
  
  constexpr bool operator==(const StringID&) const = default;

  constexpr explicit operator Type() {
    return type_;
  }

  constexpr auto id() -> Id {
    return type_;
  }

private:
  Id type_;
};

#define SID(str) StringID::from_string(str).id()