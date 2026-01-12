#include "runtime/time.hpp"
#include <chrono>

static struct {
  const TimePoint startup = Time::now();
  TimePoint elapsed;
} s;

auto Time::since_startup() -> TimePoint {
  return s.startup;
}

auto Time::elapsed_since_startup() -> Duration {
  return Time::now() - s.startup;
}

auto Time::now() -> TimePoint {
  auto current = std::chrono::steady_clock::now();
  
  return TimePoint(current.time_since_epoch().count());
}
