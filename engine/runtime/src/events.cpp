#include <runtime/events.hpp>

void Events::run() {
  
  std::swap(write_, read_);
  
  for (u32 i{}; auto& events : read_)
  {
    Vec<char>& vec = *(Vec<char>*)(events.buff); // UB btw
    if (!vec.empty())
    {
      auto& evt_data = evt_data_[i];
      for(auto& handler : evt_data.handlers)
      {
        auto count = u32(vec.size());
        handler(*this, (void*)vec.data(), count);
      }
    
      evt_data.clear(&events);
    }
    
    i++;
  }
}

auto Events::get_event_data(StringID sid) -> EventData& {
  assert(evt_table_.find(sid) != evt_table_.end());

  u32 index = evt_table_.at(sid);

  return evt_data_[index];
}

void Events::trigger_impl(Span<EventHandlerType> handlers, const void* events, u32 size) {
  for (auto& handler : handlers)
  {
    handler(*this, events, size);
  }
}

auto Events::register_event(EventData&& event_data) -> Pair<UntypedEvents&, UntypedEvents&> {
  auto it = evt_table_.find(event_data.sid);

  if (it != evt_table_.end())
  {
    log_error("tried to register {} event more than once", event_data.name);
  }

  u32 index = (u32)evt_data_.size();
  evt_table_.emplace(std::pair{ event_data.sid, index });
  evt_data_.emplace_back(std::move(event_data));

  return { write_.emplace_back(), read_.emplace_back() };
}