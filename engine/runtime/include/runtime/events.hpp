#pragma once
#include "def.hpp"

class Events {
public:
  template<typename EventType>  
  auto add_event(u32 reserved_amount = {}) -> Events& {
  
  }

  auto listen(auto handler) -> Events& {

  }

  auto unlisten(auto handler) -> Events& {

  }

  template<typename EventCollection>
  void trigger(const EventCollection& events) {
  }

  template<typename Iterator>
  void trigger(Iterator first, Iterator last) {
    
  }

  template<typename EventType>
  auto view() -> Viewer<EventType> {

  }

  template<typename EventType>
  auto batch() -> Batcher<EventType> {

  }

private:

};