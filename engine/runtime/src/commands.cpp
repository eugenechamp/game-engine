#include "runtime/commands.hpp"
#include <type_traits>

void Commands::execute() {
  swap_queue_buffers();
  // NOTE: try executing and erasing the task right after
  // implement a draining iterator or smth...
  for (const Fn<void()>& task : read_queue_)
  {
    task();
  }
  read_queue_.clear();
}

void Commands::swap_queue_buffers() {
  std::swap(write_queue_, read_queue_);
}