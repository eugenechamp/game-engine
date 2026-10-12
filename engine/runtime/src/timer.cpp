#include <runtime/timer.hpp>
/// TODO: move all of the method to .hpp in order to make them inlinable

void Timer::start() {
  stopped_ = false;
}

void Timer::stop() {
  stopped_ = true;
}

void Timer::reset() {
  time_   = Time::now();
  passed_ = Duration::zero();
}

auto Timer::elapsed() -> Duration {
  if(!is_stopped())
  {
    passed_ = Time::now() - time_;
  }
  return passed_;
}

auto Timer::duration() const -> Duration {
  return timeout_;
}

bool Timer::reset_on_expired() {
  if(is_expired())
  {
    reset();
    return true;
  }

  return false;
}

auto Timer::restart() -> Duration {
  Duration passed = elapsed();
  reset();
  return passed;
}

void Timer::expires_after(Duration timeout) {
  time_ = Time::now();
  timeout_ = timeout;
}

bool Timer::is_expired() {
  return elapsed() >= duration();
}

bool Timer::is_stopped() const {
  return stopped_;
}

void Stopwatch::start() {
  stopped_ = false;
}

void Stopwatch::stop() {
  stopped_ = true;
}

void Stopwatch::reset() {
  start_   = Time::now();
  elapsed_ = Duration::zero();
}

auto Stopwatch::elapsed() -> Duration {
  if (!is_stopped())
  {
    elapsed_ = Time::now() - start_;
  }
  return elapsed_;
}

auto Stopwatch::restart() -> Duration {
  Duration passed = elapsed();
  reset();
  return passed;
}

bool Stopwatch::is_stopped() const {
  return stopped_;
}