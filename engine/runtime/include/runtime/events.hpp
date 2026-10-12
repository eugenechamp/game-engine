#pragma once
#include "resource.hpp"
#include "type_traits.hpp"
#include "fwd.hpp"
#include "meta.hpp"
#include "emitter.hpp"
#include "core/pair.hpp"

struct Event {
protected:
  struct Triggerable {};
  struct DisallowTransfer {};
};

template<typename T>
concept EventType = std::derived_from<Event, T>;

class Events final {
  friend class App;
private:
  explicit Events(Resources& res, World& world) 
    : res_{res}, 
      world_{world}
  { }

public:
  ~Events() = default;

public:
  template<typename EventType>
  auto add_event(u32 reserved_amount = {}) -> Events& {
    constexpr auto sid    = core::type_sid<EventType>();
    std::string_view name = core::nameof<EventType>();
    
    auto [primary, secondary] = register_event(EventData {
      .handlers = {},
      .clear = [](UntypedEvents* events) {
        Vec<EventType>& vec = *(Vec<EventType>*)events;
        vec.clear();
      },
      .destroy = [](UntypedEvents* events){ 
        Vec<EventType>& vec = *(Vec<EventType>*)events;
        vec.clear();
      },
      .name = std::string(name.begin(), name.end()),
      .sid = sid,
      .size_of = sizeof(EventType),
    });

    Vec<EventType>* primary_vec = ::new((void*)primary.buff) Vec<EventType>();
    primary_vec->reserve(reserved_amount);

    Vec<EventType>* secondary_vec = ::new((void*)secondary.buff) Vec<EventType>();
    secondary_vec->reserve(reserved_amount);
    
    return *this;
  }
  
  //template<typename EventType, typename... Args>
  //auto listen(Invocable<void(const EventType&, Args...)> auto&& handler) -> Events& {
  //
  //}

  template<typename EventType, typename Instance, typename... Args>
  auto listen(auto&& handler) -> Events& {
    constexpr auto sid = core::type_sid<EventType>();
    get_event_data(sid).handlers.emplace_back(std::move(handler));
    return *this;
  }

  //template<typename EventType, typename Instance, typename... Args>
  //auto listen(Invocable<void(const Instance*, const EventType&, Args...)> auto&& handler) -> Events& {

  //}

  template<typename EventType, typename... Res>
  auto listen(void(*handler)(Span<const EventType>, Res...)) -> Events& {
    constexpr auto sid = core::type_sid<EventType>();
    get_event_data(sid).handlers.emplace_back(FreeFunctionHandlerSpan{handler});
    
    return *this;
  }

  template<typename Instance, typename EventType, typename... Args>
  auto listen(void(*handler)(Instance*, Span<const EventType>, Args...)) -> Events& {

    /*auto wrapper = [handler](const void* events) {
      Vec<EventType>& e = ((EventStorage<EventType>*)events)->read_buff_;
      handler(Span<const EventType>(e));
    };*/

    return *this;
  }

  template<typename Instance, typename EventType, typename... Res>
  auto listen(void(Instance::*handler)(Span<const EventType>, Res...), Instance* instance) -> Events& {
    
    constexpr auto sid = core::type_sid<EventType>();
    get_event_data(sid).handlers.emplace_back(MemberFnHandlerSpan{handler, instance});
    
    return *this;
  }

  template<typename Instance, typename EventType, typename... Res>
  auto listen(void(Instance::*handler)(EventType, Res...), Instance* instance) -> Events& {
    using event_type = std::remove_cvref_t<EventType>;

    constexpr auto sid = core::type_sid<event_type>();
    get_event_data(sid).handlers.emplace_back(MemberFnHandler{ handler, instance });

    return *this;
  }
  
  template<typename EventType, typename Instance, typename... Res>
  auto unlisten(void(Instance::*handler)(Viewer<EventType>, Res...), Instance* instance) -> Events& {
    
    EventData& data = get_event_data(core::type_sid<EventType>());
    
    MemberFnHandlerSpan delegate{ handler, instance };

    for (u32 i{}; i < data.handlers.size(); i++)
    {
      auto* ptr = data.handlers[i].target<decltype(delegate)>();
      if (ptr && ptr->fn == delegate.fn && ptr->instance == delegate.instance)
      {
        //std::swap(data.handlers[i], data.handlers.back());
        data.handlers[i].swap(data.handlers.back());
        data.handlers.resize(data.handlers.size() - 1);
        break;
      }
    }

    return *this;
  }

  template<typename EventType, typename Instance, typename... Res>
  auto unlisten(void(Instance::* handler)(EventType, Res...), Instance* instance) -> Events& {
    using event_type = std::remove_const_t<std::remove_reference_t<EventType>>;

    EventData& data = get_event_data(core::type_sid<event_type>());
    
    MemberFnHandler delegate{ handler, instance };
    
    for (u32 i{}; i != data.handlers.size(); i++)
    {
      auto* ptr = data.handlers[i].target<decltype(delegate)>();
      if (ptr && ptr->fn == delegate.fn && ptr->instance == delegate.instance)
      {
        std::swap(data.handlers[i], data.handlers.back());
        data.handlers.resize(data.handlers.size() - 1);
        break;
      }
    }
    return *this;
  }

  template<typename EventType>
  void trigger(const EventType& event) {
    Span events = Span{ &event, 1ull };
    trigger(events);
  }

  template<typename EventType>
  void trigger(Viewer<EventType> events) {
    constexpr auto sid = core::type_sid<EventType>();
    
    EventData& data = get_event_data(sid);

    trigger_impl(data.handlers, events.data(), u32(events.size()));
  }

  template<typename EventType>
  auto view() /* -> Viewer<EventType> */ {
    /// TODO:
    /// UPD: do I really have to?
  }

  template<typename EventType>
  auto emit() -> Emitter<EventType> {
    constexpr auto sid = core::type_sid<EventType>();
    
    u32 index = evt_table_[sid];
    
    Vec<EventType>& v = *(Vec<EventType>*)write_[index].buff;
    
    return Emitter<EventType>{v};
  }

private:
  void run();

private:
  using EventHandlerType = Fn<void(Events&, const void*, u32)>;
  
  struct EventData;
  
  auto get_event_data(StringID sid) -> EventData&;
  
  void trigger_impl(Span<EventHandlerType>, const void*, u32);
  
  struct UntypedEvents;
  
  auto register_event(EventData&&) -> Pair<UntypedEvents&, UntypedEvents&>;

  template<typename EventType>
  auto secure() {

  }

  template<typename Res>
  auto get_resource() -> decltype(auto) {
    using namespace traits::_;
    
    //using ResourceType = std::remove_reference_t<Res>;
    if constexpr(is_emitter<Res>::value)
    {
      return emit<typename Res::element_type>();
    }
    else
    {
      return static_cast<ResourceStorage<Res>*>(res_.at(core::type_sid<Res>()).get())->resource;
    }
  }

private:
  template<typename EventType, typename Instance, typename... Res>
  struct MemberFnHandlerSpan {
    void operator()(Events& events, const void* evt_view, u32 size) const {
      Span<const EventType> span((const EventType*)evt_view, size);
      (instance->*fn)(Span((const EventType*)evt_view, size), events.get_resource<Res>()...);
    }

    void(Instance::* fn)(Viewer<EventType>, Res...);
    Instance* instance;
  };

  template<typename EventType, typename Instance, typename... Res>
  struct MemberFnHandler {
    void operator()(Events& events, const void* evt_view, u32 size) const {
      
      using event_type = std::remove_cvref_t<EventType>;
      
      Span<const event_type> span((const event_type*)evt_view, size);
      
      for (auto& event : span)
      {
        (instance->*fn)(event, events.get_resource<Res>()...);
      }
    }

    void(Instance::* fn)(EventType, Res...);
    Instance* instance;
  };

  template<typename EventType, typename... Res>
  struct FreeFunctionHandlerSpan {
    void operator()(Events& events, const void* evt_view, u32 size) const {
      Span span((const EventType*)evt_view, size);
      fn(span, events.get_resource<Res>()...);
    }

    void(*fn)(Viewer<EventType>, Res...);
  };

  using EventType = StringID;
  
  struct UntypedEvents {
    char buff[sizeof(Vec<char>)]{};
  };

  struct EventData {
    Vec<EventHandlerType> handlers;
    void(*clear)(UntypedEvents*);
    void(*destroy)(UntypedEvents*);
    std::string name;
    StringID sid;
    u32 size_of;
  };
  
  Resources& res_;
  World& world_;

  HashMap<EventType, u32> evt_table_;
  Vec<EventData> evt_data_;
  Vec<UntypedEvents> write_;
  Vec<UntypedEvents> read_;
};