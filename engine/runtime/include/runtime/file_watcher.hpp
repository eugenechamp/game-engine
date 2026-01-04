#pragma once
#include "def.hpp"
#include "fwd.hpp"

struct FileWatcher {
  using Callback = Fn<void(const std::fs::path&)>;
  static void watch(const std::fs::path& file, Callback callback);

  static void run(App&);
};