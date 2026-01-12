#pragma once
#include <chrono>
#include "time.hpp"
#include "def.hpp"
#include "type_traits.hpp"

enum class TimerMode : i8 {
  Once,
  Repeat,
};

class Timer {
public:
  Timer() = default;
  Timer(Duration timeout)
    : timeout_{ timeout }, 
      stopped_{ false }
  { }

  void start();
  void stop();
  void reset();
  void expires_after(Duration);
  
  [[nodiscard]] auto elapsed() -> Duration;
  [[nodiscard]] auto restart() -> Duration;
  [[nodiscard]] auto duration() const -> Duration;

  bool is_expired();
  bool is_stopped() const;

private:
  TimePoint time_   = {};
  Duration passed_  = {};
  Duration timeout_ = {};
  u32 expire_count_ = {};
  TimerMode mode_   = {};
  bool stopped_     = false;
};

/// TODO: implement
class TickTimer;

class Stopwatch {
public:
  void start();
  void stop();
  void reset();
  
  [[nodiscard]] auto elapsed() -> Duration;
  [[nodiscard]] auto restart() -> Duration;
  
  bool is_stopped() const;

private:
  TimePoint start_  = {};
  Duration elapsed_ = {};
  bool stopped_     = false;
};

template<Callable<void(Duration)> Callback>
class [[nodiscard]] ScopedTimer {
  /// NOTE: you could generalize this class (turn into ScopedCallback for example)
public:
  ScopedTimer(Callback&& callback)
    : callback_{callback}
  { }
  
  ~ScopedTimer() {
    callback_(clock_.elapsed());
  }
private:
  Stopwatch clock_;
  Callback callback_;
};