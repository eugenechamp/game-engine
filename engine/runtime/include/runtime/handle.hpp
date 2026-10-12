#pragma once
#include "def.hpp"
#include "fwd.hpp"

template<typename Res>
class Handle {
  /// NOTE: rename to AssetHandle 
  friend AssetCache<Res>;
public:
  using ElementType  = Res;
  using IndexType    = u16;
  using VersionType  = u16;

public:
  Handle()  = default;
  ~Handle() = default;
  
  Handle(const Handle&)                = default;
  Handle(Handle&&) noexcept            = default;
  auto operator=(const Handle&)     -> Handle& = default;
  auto operator=(Handle&&) noexcept -> Handle& = default;

  explicit operator bool() const { return valid(); }
  bool valid() const { return index_ != invalid && version_ != 0; }

  friend bool operator==(const Handle lhs, const Handle rhs) noexcept {
    return lhs.index_ == rhs.index_ && lhs.version_ == rhs.version_;
  }

  friend bool operator<(const Handle lhs, const Handle rhs) noexcept {
    return lhs.index_ < rhs.index_;
  }

  auto index() const -> IndexType { return index_; }

private:
  Handle(IndexType offset, VersionType version)
    : index_{offset}, version_{version} 
  { }
 
private:
  static constexpr IndexType invalid = IndexType(-1);

private:
  IndexType index_     = invalid;
  VersionType version_ = {};
};
