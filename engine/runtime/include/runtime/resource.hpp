#pragma once
#include "def.hpp"

struct Resource {};

template<typename T>
concept ResourceType = std::derived_from<T, Resource>;

/// TODO: encapsulate it better 
struct ResourceStorageBase {
  ResourceStorageBase() = default;
  virtual ~ResourceStorageBase() = default;
};

template<typename T>
struct ResourceStorage : ResourceStorageBase {
  ResourceStorage()  = default;
  ~ResourceStorage() = default;
  
  explicit ResourceStorage(T&& r) 
    : resource(std::move(r))
  { }

  T resource;
};

enum class ResourceID : size_t {};
using Resources = HashMap<ResourceID, Unique<ResourceStorageBase>>;