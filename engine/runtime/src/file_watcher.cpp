#include "runtime/file_watcher.hpp"
#include <thread>
#include <chrono>

using namespace std::literals::chrono_literals;

struct FileData {
  FileWatcher::Callback callback;
  std::fs::file_time_type last_write;
};

static HashMap<std::fs::path, FileData> g_files;

void FileWatcher::watch(const std::fs::path &file, Callback callback) {
  if (!g_files.contains(file))
  {
    g_files[file] = FileData {
      .callback = std::move(callback),
      .last_write = std::fs::last_write_time(file)
    };
  }
  else
  {
    g_files[file].callback = std::move(callback);
  }
}

void FileWatcher::run() {
  /*std::thread([] {*/
    for (auto& [key, value] : g_files)
    {
      auto last_write = std::fs::last_write_time(key);
      if (value.last_write < last_write)
      {
        value.callback(key);
        value.last_write = last_write;
      }
    }
    
    //std::this_thread::sleep_for(/* hardoced for now */ 300ms);
  
  //}).detach();
}