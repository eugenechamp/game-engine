#pragma once
#include "def.hpp"

struct LoadContext {
  Vec<std::fs::path> paths;
};

template<typename T>
struct Loader {
  T load(const LoadContext&) = delete;
};