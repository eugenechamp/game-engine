#include "runtime/file_watcher.hpp"
#include "runtime/app.hpp"
#include "runtime/commands.hpp"
#include <thread>
#include <chrono>
#include <mutex>

using namespace std::literals::chrono_literals;

struct FileData {
  FileWatcher::Callback callback;
  std::fs::file_time_type last_write;
};

static HashMap<std::fs::path, FileData> g_files;
static std::mutex mutex;

void FileWatcher::watch(const std::fs::path &file, Callback callback) {
  std::scoped_lock lock(mutex);
  
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

void FileWatcher::run(App& app) {
  
  std::thread([&app] {
    while(true)
    {
      mutex.lock();
      for (auto& [key, value] : g_files)
      {
        auto last_write = std::fs::last_write_time(key);
        if (value.last_write < last_write)
        {
          value.last_write = last_write;

          app.commands().enqueue([key, &value] { value.callback(key); });
        }
      }
      mutex.unlock();
      std::this_thread::sleep_for(200ms);
    }
  }).detach();
  
}