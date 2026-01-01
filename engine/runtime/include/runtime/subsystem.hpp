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
  virtual void fixed_update(float dt)   {}
  virtual void update(float dt)       = 0;
  virtual void shutdown(App&)         = 0;
  
  virtual void serialize()              {}
  virtual void deserialize()            {}
  
  virtual bool run_condition(App&)      = 0;
  virtual bool shutdown_condition(App&) = 0;

protected:
  auto world() -> World& {
    return world_;
  }
  
private:
  World& world_;
};