#pragma once
#include "def.hpp"
#include <mutex>

class Commands {
  friend class App;
public:
  ~Commands() = default;

private:
  Commands()                      = default;
  Commands(const Commands&)       = delete;
  Commands(Commands&&)            = delete;
  auto operator=(const Commands&) = delete;
  auto operator=(Commands&&)      = delete;

public:
  void enqueue(Fn<void()> task) {
    std::scoped_lock lock{ mutex_ };
    write_queue_.emplace_back(std::move(task));
  }
  
private: // methods
  void execute();
  void swap_queue_buffers();

private: /// NOTE: consider PIMPL
  std::mutex mutex_;
  Vec<Fn<void()>> write_queue_;
  Vec<Fn<void()>> read_queue_;
};