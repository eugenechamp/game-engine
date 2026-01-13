#pragma once
#include "fwd.hpp"

class Subsystem {
  friend App;

public: // it was protected initially
  Subsystem() = delete;
  explicit Subsystem(App& app);
  Subsystem(const Subsystem&) = delete;
  Subsystem(Subsystem&&) = delete;
  
  auto operator=(const Subsystem&) = delete;
  auto operator=(Subsystem&&) = delete;

protected:
  virtual ~Subsystem() = default;

private:
  virtual void init(App&)             = 0;
  virtual void fixed_update(float [[maybe_unused]] dt)   {}
  virtual void update(float dt)       = 0;
  virtual void shutdown(App&)         = 0;
  
  virtual void serialize()              {}
  virtual void deserialize()            {}
  
  virtual bool run_condition(App&) { return true; };
  virtual bool shutdown_condition(App&) { return false; }

protected:
  auto world() -> World& {
    return world_;
  }
  
private:
  World& world_;
};