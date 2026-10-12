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
  explicit Timer(Duration timeout)
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
  [[nodiscard]] bool reset_on_expired();

  bool is_expired();
  bool is_stopped() const;

private:
  TimePoint time_   = {};
  Duration passed_  = {};
  Duration timeout_ = {};
  [[maybe_unused]] u32 expire_count_ = {};
  [[maybe_unused]] TimerMode mode_   = {};
  bool stopped_     = false;
};

class TickTimer {
public:
  TickTimer()  = default;
  ~TickTimer() = default;
  
  explicit TickTimer(Duration timeout)
    : elapsed_{ Duration::zero() }, 
      timeout_{ timeout }, 
      stopped_{ false }
  { }

  void tick(f32 dt) {
    if(is_stopped())
    {
      return;
    }
    elapsed_ += Duration::seconds(dt);
  }

  void start() {
    stopped_ = false;
  }
  
  void stop() {
    stopped_ = true;
  }

  void reset() {
    elapsed_ = Duration::zero();
  }

  // working name
  [[nodiscard]] bool on_expired() {
    if (is_expired())
    {
      elapsed_ -= duration();
      return true;
    }

    return false;
  }

  bool reset_on_expired() {
    if(is_expired())
    {
      reset();
      return true;
    }
    return false;
  }

  void expires_after(Duration timeout) {
    reset();
    timeout_ = timeout;
  }
  
  [[nodiscard]] auto elapsed() const -> Duration {
    return elapsed_;
  }

  [[nodiscard]] auto restart() -> Duration {
    Duration passed = elapsed_;
    reset();
    return passed;
  }

  [[nodiscard]] auto duration() const -> Duration {
    return timeout_;
  }

  bool is_expired() const {
    assert(is_valid());
    return elapsed_ > timeout_;
  }
  
  bool is_valid() const {
    return timeout_ > Duration::zero();
  }

  auto expired_count() const -> u32 {
    assert(is_valid());
    return elapsed_.ns() / timeout_.ns();
  }
  
  bool is_stopped() const {
    return stopped_;
  }

private:
  Duration elapsed_{ Duration::zero() };
  Duration timeout_{ Duration::zero() };
  bool stopped_{ false };
};

/// that should be a minimal version of the Timer class that consumes less memory
/// can be used if there's no need to stop the timer
class MiniTimer {

};

// utility for measuring the elapsed time since start
class Stopwatch {
public:
  void start();
  void stop();
  void reset();
  
  [[nodiscard]] auto elapsed() -> Duration;
  [[nodiscard]] auto restart() -> Duration;
  
  bool is_stopped() const;

private:
  TimePoint start_  = Time::now();
  Duration elapsed_ = {};
  bool stopped_     = false;
};

// A timer that never stops
class Countdown {
public:
  void tick(f32 dt) {

  }

  bool is_expired() const {

  }

private:
  Duration elapsed_;
  Duration duration_;
};

// A tick timer that never stops
class TickCountdown {
public:

private:
};

//template<Callable<void(Duration)> Callback>
//class [[nodiscard]] ScopedTimer {
//  /// NOTE: you could generalize this class (turn into ScopedCallback for example)
//public:
//  ScopedTimer(Callback&& callback)
//    : callback_{callback}
//  { }
//  
//  ~ScopedTimer() {
//    callback_(clock_.elapsed());
//  }
//private:
//  Stopwatch clock_;
//  Callback callback_;
//};