#pragma once
#include "def.hpp"
#include "meta.hpp"
#include "type_traits.hpp"

#include "app.hpp"
#include "batcher.hpp"

class Events final {
  friend class App;
private:
  Events(App& app) 
    : app_{app}
  { }

public:
  ~Events() = default;
public:
  template<typename EventType>
  auto add_event(u32 reserved_amount = {}) -> Events& 
  {
    constexpr auto sid = core::type_sid<EventType>();

    auto it = events_.find(sid);
    if (it != events_.end())
    {
      log_error("{} event was registered twice", core::nameof<EventType>());
    }

    Unique<EventStorageBase> storage(new EventStorage<EventType>());

    events_.emplace(std::pair{ 
      sid, 
      EventData {.events = std::move(storage) }
    });
    
    return *this;
  }
  
  template<typename Res>
  static auto get_resource(Events& events) -> Res {
    using namespace traits::_;

    // using ResourceType = std::remove_reference_t<Res>;
    if constexpr(is_batcher<Res>::value)
    {
      return events.batch<typename Res::element_type>();
    }
    else 
    {
      return events.app_.get<Res>();
    }
  }

  template<typename EventType, typename... Res>
  auto listen(void(*handler)(Span<const EventType>, Res...)) -> Events& {
    
    auto wrapper = [this, handler](const void* events) {
      Vec<EventType>& e = ((EventStorage<EventType>*)events)->read_buff_;
      handler(Span<const EventType>(e), get_resource<Res>(*this)...);
    };

    events_.at(core::type_sid<EventType>()).handlers.push_back(std::move(wrapper));
    
    return *this;
  }

  template<typename Instance, typename EventType, typename... Args>
  auto listen(void(*handler)(Instance*, Span<const EventType>, Args...)) -> Events& {

    auto wrapper = [handler](const void* events) {
      Vec<EventType>& e = ((EventStorage<EventType>*)events)->read_buff_;
      handler(Span<const EventType>(e));
    };

    return *this;
  }

  template<typename Instance, typename EventType, typename... Args>
  auto listen(void(Instance::*handler)(Span<const EventType>, Args...), Instance* instance) -> Events& {

    auto wrapper = [handler, instance](const void* events) {
      Vec<EventType>& e = ((EventStorage<EventType>*)events)->read_buff_;
      (instance->*handler)(Span<const EventType>(e), get_resource<Args>()...);
      //handler(Span<const EventType>(e));
    };
    events_.at(core::type_sid<EventType>()).handlers.push_back(std::move(wrapper));
    return *this;
  }

  auto unlisten(auto handler) -> Events& {
    // NOTE: with the current implementation it's not supported atm,
    // but will be in the future
    
    return *this;
  }

  template<typename EventCollection>
  void trigger(const EventCollection& events) {
    // TODO
    using event_type = typename EventCollection::value_type;
  }

  template<typename Iterator>
  void trigger(Iterator first, Iterator last) {
    // TODO
  }

  template<typename EventType>
  auto view() /* -> Viewer<EventType> */ {
    // TODO
  }

  template<typename EventType>
  auto batch() -> Batcher<EventType> {
    constexpr auto sid = core::type_sid<EventType>();
    auto* storage = events_.at(sid).events.get();
    EventStorage<EventType>* event_storage = (EventStorage<EventType>*)storage;
    Vec<EventType>& v = event_storage->write_buff_;
    return Batcher<EventType>{v};
  }

private:
  void run();

  template<typename EventType>
  auto secure() {

  }

private:
   using EventHandlerType = Fn<void(const void*)>;

   struct EventStorageBase {
     virtual void clear() = 0;
     virtual bool is_empty() = 0;
     virtual void swap_buffers() = 0;
     virtual ~EventStorageBase() = default;
   };
  
   template<typename EventType>
   struct EventStorage : EventStorageBase {
     Vec<EventType> write_buff_;
     Vec<EventType> read_buff_;

     void clear() override {
       read_buff_.clear();
     }
     
     bool is_empty() override {
       return read_buff_.empty();
     }

     void swap_buffers() override {
       std::swap(write_buff_, read_buff_);
     }
   };

  //struct EventStorage {
  //  using BufferPlacement = char[sizeof(Vec<char>)];
  //  BufferPlacement write_buff_;
  //  BufferPlacement read_buff_;
  //  
  //  /*char write_buff_[sizeof(Vec<char>)];
  //  char read_buff_[sizeof(Vec<char>)];*/

  //  void swap_buffers() {
  //    std::swap(write_buff_, read_buff_);
  //  }
  //};

  struct EventData {
    Unique<EventStorageBase> events;
    Vec<EventHandlerType> handlers;
  };
  
  using EventType = StringID;
  HashMap<EventType, EventData> events_;
  App& app_;
};