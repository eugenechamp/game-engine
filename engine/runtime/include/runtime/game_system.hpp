#pragma once
#include "fwd.hpp"

class GameSystem {
public:
  GameSystem(App& app);

public:
  virtual void init()                 = 0;
  virtual void fixed_update(float dt) = 0;
  virtual void update(float dt)       = 0;
  virtual void shutdown()             = 0;

  auto world() -> World& {
    return world_;
  }

private:
  World& world_;
};