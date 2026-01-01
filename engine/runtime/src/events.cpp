#include <runtime/events.hpp>

void Events::run() {
  for(auto& [type, data] : events_) 
  {
    data.events->swap_buffers();
    if(data.events->is_empty())
    {
      continue;
    }

    for(auto& handler : data.handlers) 
    {
      handler(data.events.get());
    }
    
    data.events->clear();
  }
}