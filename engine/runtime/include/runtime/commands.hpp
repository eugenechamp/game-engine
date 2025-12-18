#pragma once

class Commands {
  friend class App;
private:
  Commands()                      = default;
  Commands(const Commands&)       = delete;
  Commands(Commands&&)            = delete;
  auto operator=(const Commands&) = delete;
  auto operator=(Commands&&)      = delete;

public:
  void execute();
private: // methods
 
private: // members
};