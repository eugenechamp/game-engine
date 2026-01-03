#pragma once
#include "def.hpp"
#include "file_watcher.hpp"
#include "handle.hpp"
#include "loader.hpp"

template<typename AssetType>
struct AssetCache {
  using Id = std::string;
  using IndexType   = u16;
  using VersionType = u16;
  using HandleType = Handle<AssetType>;
public:
  AssetCache() = default;

  template<typename Filepath, typename... Filepaths>
  auto load(const Id& key, Filepath&& path, Filepaths&&... paths) -> Handle<AssetType> {
    
    auto it = lookup_.find(key);
    if (it != lookup_.end())
    {
      return HandleType{ it->second, registry_[it->second].version };
    }

    HandleType handle;

    LoadContext load_ctx{ .paths = { path, std::forward<Filepaths>(paths)... } };

    if (!freelist_.empty())
    {
      auto index = freelist_.back();
      // should not be invalid ever
      auto& entry = registry_[index];

      entry = AssetData{
        .asset = Loader<AssetType>{}.load(load_ctx),
        .meta = AssetMeta{},
        .id = key,
        .version = VersionType(entry.version + 1)
      };

      lookup_.emplace(std::make_pair(key, index));

      freelist_.resize(freelist_.size() - 1);

      handle = HandleType{ index,  entry.version };
    }
    else
    {
      registry_.emplace_back(AssetData{ Loader<AssetType>{}.load(load_ctx), {}, key, 1 });

      auto index = static_cast<IndexType>(registry_.size() - 1);
      lookup_.emplace(std::make_pair(key, index));

      handle = HandleType{ index, 1 };
    }

    for (auto& file : load_ctx.paths) {
      FileWatcher::watch(file, [this, handle, ctx = load_ctx](const std::fs::path& path) {
        auto& asset = this->get(handle);
        asset = std::move(Loader<AssetType>{}.load(ctx));
      });
    }

    return handle;
  }

  auto add(const Id& key, AssetType&& asset) -> HandleType {
    auto it = lookup_.find(key);

    if (it != lookup_.end())
    {
      return HandleType{ it->second, registry_[it->second].version };
    }

    if (!freelist_.empty())
    {
      auto index = freelist_.back();
      // should not be invalid ever
      auto& entry = registry_[index];

      entry = AssetData{ 
        .asset   = std::forward<AssetType>(asset), 
        .meta    = AssetMeta{}, 
        .id      = key,
        .version = entry.version + 1
      };

      lookup_.emplace(std::make_pair(key, index));
      
      freelist_.resize(freelist_.size() - 1);

      return HandleType{ index,  entry.version };
    }
    else
    {
      registry_.emplace_back(std::move(asset), {}, key, 1);
      
      auto index = static_cast<IndexType>(registry_.size() - 1);
      lookup_.emplace(std::make_pair(key, index));
      
      return HandleType{ index, 1};
    }
  }

  void remove(HandleType handle) {
    if (handle.index_ < registry_.size() && handle.version_ == registry_[handle.index_].version)
    {
      lookup_.erase(handle.id_);
      freelist_.push_back(handle.index_);
    }
  }

  auto find(const Id& key) -> HandleType {
    auto it = lookup_.find(key);

    if(it != lookup_.end())
      return { it->second, registry_[it->second].version };
    
    return HandleType{};
  }


  auto get(HandleType handle) -> AssetType& {
    assert(handle);
    assert(registry_.at(handle.index_).version == handle.version_);

    return registry_[handle.index_].asset;
  }

  auto try_get(HandleType handle) -> AssetType* {
    if (!handle)
      return nullptr;

    auto version = registry_[handle.index_].version;
    
    if(version != handle.version_)
      return nullptr;
    
    return &registry_[handle.index_].asset;
  }

private:
  struct AssetMeta {
  #ifndef _NDEBUG  

  #endif
  };
  
  struct AssetData {
    AssetType      asset;
    AssetMeta       meta;
    Id                id;
    VersionType  version;
  };

  Vec<AssetData> registry_;
  HashMap<Id, IndexType> lookup_;
  Vec<IndexType> freelist_;
};