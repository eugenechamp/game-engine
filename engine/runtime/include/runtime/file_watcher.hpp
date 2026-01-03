#pragma once
#include "def.hpp"

struct FileWatcher {
  using Callback = Fn<void(const std::fs::path&)>;
  static void watch(const std::fs::path& file, Callback callback);

  static void run();
};