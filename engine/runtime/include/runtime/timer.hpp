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
    return elapsed_ >= timeout_;
  }
  
  bool is_stopped() const {
    return stopped_;
  }

private:
  Duration elapsed_{ Duration::zero() };
  Duration timeout_{ Duration::zero() };
  bool stopped_{ false };
};

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