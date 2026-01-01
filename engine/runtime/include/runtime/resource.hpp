#pragma once
#include "def.hpp"

struct ResourceStorageBase {
  virtual ~ResourceStorageBase() = default;
};

template<typename T>
struct ResourceStorage : ResourceStorageBase {
  explicit ResourceStorage(T&& r) 
    : resource(std::move(r)) 
  { }

  T resource;
  ~ResourceStorage() = default;
};

using ResourceID = size_t;
using Resources  = HashMap<ResourceID, Unique<ResourceStorageBase>>;