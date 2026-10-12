#pragma once
#include "def.hpp"

enum class TimerHandle : u32 {};

struct TimerInfo {
  Duration remaining;
  bool loop;
};

class TimerScheduler {
public:
  void execute() {}

  auto set_timer(TimerInfo) -> TimerHandle;
  void unset_timer(TimerHandle);
  auto get(TimerHandle) -> TimerInfo;

  bool is_finished(TimerHandle);

private:
  std::multimap<TimePoint, Fn<void()>> schedule_;
};