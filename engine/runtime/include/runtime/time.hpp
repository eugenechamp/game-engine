#pragma once
#include "def.hpp"

// class that represents a time interval
class Duration {
public:
  using Repr    = i64;
  using SecondsRepr = f64;
public:
  constexpr Duration()                 = default;
  constexpr ~Duration()                = default;
  
  constexpr Duration(const Duration&)  = default;
  constexpr Duration(Duration&&)       = default;
  
  constexpr auto operator=(const Duration&) -> Duration&     = default;
  constexpr auto operator=(Duration&&) noexcept -> Duration& = default;

  static constexpr auto seconds(SecondsRepr secs) -> Duration {
    constexpr auto ratio = 1'000'000'000;
    return Duration(secs * ratio);
  }

  static constexpr auto milliseconds(Repr ms) -> Duration {
    constexpr auto ratio = 1'000'000;
    return Duration(ms * ratio);
  }

  static constexpr auto microseconds(Repr us) -> Duration {
    constexpr auto ratio = 1'000;
    return Duration(us * ratio);
  }

  static constexpr auto nanoseconds(Repr ns) -> Duration {
    return Duration(ns);
  }

  static constexpr auto zero() -> Duration {
    return Duration(0);
  }

private: 
  constexpr explicit Duration(Repr ns) 
    : interval_ns_{ns}
  {}

public:
  constexpr auto sec() const -> SecondsRepr {
    // we need a higher precision for seconds
    return interval_ns_ * 0.001 * 0.001 * 0.001;
  }
  constexpr auto ms() const  -> Repr {
    return us() * 0.001;
  }
  constexpr auto us() const  -> Repr {
    return ns() * 0.001;
  }
  constexpr auto ns() const  -> Repr {
    return interval_ns_;
  }

  constexpr auto operator+=(Duration other) -> Duration& {
    interval_ns_ += other.interval_ns_;
    return *this;
  }
  
  constexpr auto operator-=(Duration other) -> Duration& {
    interval_ns_ -= other.interval_ns_;
    return *this;
  }
  
  constexpr auto operator*=(Repr multiplier) -> Duration& {
    interval_ns_ *= multiplier;
    return *this;
  }

  constexpr auto operator*=(f64 multiplier) -> Duration& {
    interval_ns_ *= multiplier;
    return *this;
  }

  constexpr auto operator/=(Repr divisor) -> Duration& {
    interval_ns_ /= divisor;
    return *this;
  }

  constexpr auto operator/=(f64 divisor) -> Duration& {
    interval_ns_ /= divisor;
    return *this;
  }

  constexpr auto operator%=(Repr divisor) {
    interval_ns_ %= divisor;
    return *this;
  }

  constexpr auto operator%=(Duration duration) {
    interval_ns_ %= duration.ns();
    return *this;
  }

  friend constexpr auto operator<=>(Duration lhs, Duration rhs) {
    return lhs.ns() <=> rhs.ns();
  }

private:
  Repr interval_ns_{};
};

namespace literals {
  constexpr auto operator""_sec(long double val) noexcept -> Duration {
    return Duration::seconds(static_cast<Duration::SecondsRepr>(val));
  }

  constexpr auto operator""_sec(u64 val) noexcept -> Duration {
    return Duration::seconds(static_cast<Duration::SecondsRepr>(val));
  }

  constexpr auto operator""_ms(u64 val) noexcept -> Duration {
    return Duration::milliseconds(static_cast<Duration::Repr>(val));
  }

  constexpr auto operator""_us(u64 val) noexcept -> Duration {
    return Duration::microseconds(static_cast<Duration::Repr>(val));
  }

  constexpr auto operator""_ns(u64 val) noexcept -> Duration {
    return Duration::nanoseconds(static_cast<Duration::Repr>(val));
  }
}

class TimePoint {
public:
  using Repr        = i64;
  using SecondsRepr = f64;
  
public:
  constexpr TimePoint()                 = default;
  constexpr ~TimePoint()                = default;
  constexpr TimePoint(const TimePoint&) = default;
  constexpr TimePoint(TimePoint&&)      = default;
  constexpr auto operator=(const TimePoint&) -> TimePoint&     = default;
  constexpr auto operator=(TimePoint&&) noexcept -> TimePoint& = default;
  
  explicit constexpr TimePoint(Repr timepoint)
    : time_{timepoint}
  { }

public:
  auto operator-(const TimePoint& rhs) -> Duration {
    return Duration::nanoseconds(time_ - rhs.time_);
  }

  auto operator+(const TimePoint& rhs) -> TimePoint {
    return TimePoint(time_ + rhs.time_);
  }
  
  friend auto operator+(const TimePoint& lhs, const Duration& rhs) -> TimePoint {
    return TimePoint(lhs.time_ + rhs.ns());
  }

  auto operator+=(Duration rhs) -> TimePoint& {
    time_ += rhs.ns();
    return *this;
  }

  friend constexpr auto operator<=>(TimePoint lhs, TimePoint rhs) {
    return lhs.time_ <=> rhs.time_;
  }

private:
  Repr time_{};
};

struct Time {
  static auto since_startup() -> TimePoint;
  static auto elapsed_since_startup() -> Duration;
  static auto now() -> TimePoint;
};

struct SysClock {
  static auto now() -> TimePoint;
};

struct HighResClock {
  static auto now() -> TimePoint;
};