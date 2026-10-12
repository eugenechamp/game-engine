#pragma once
#ifndef _ENGINE_STATES_HPP_
#define _ENGINE_STATES_HPP_

#include "def.hpp"
#include "events.hpp"

///TODO: remove
#include "meta.hpp"

template<typename State>
struct OnStateChanged {
  State changed_from;
  State changed_to;
};

template<typename State>
struct OnStateAdded {};

template<typename State>
struct OnStateRemoved {};

struct StateBase {};

template<typename T>
struct State : StateBase {
  explicit State(T state) 
    : state{ state }
  { }

  std::atomic<T> state;
};

class States {
  friend class App;
public:
  ~States() = default;

private:
  States(Events& events)
   : events_{events}
  { }

  template<typename StateType>
  void init_state(StateType state) {
    constexpr auto sid = core::type_sid<StateType>();
    auto it = lookup_.find(sid);
    
    assert(it == lookup_.end());

    lookup_.emplace(std::pair{ sid, Unique<StateBase>(new State<StateType>(state))});
      
    events_.add_event<OnStateChanged<StateType>>(0u);
  }

  template<typename StateType>
  void set_state(StateType new_state) {
    auto old_state = get_unchecked<StateType>().state.exchange(new_state);
    
    events_.emit<OnStateChanged<StateType>>().emplace_back(OnStateChanged<StateType>{ 
      .changed_from = old_state,
      .changed_to   = new_state
    });
  }

  template<typename StateType>
  bool in_state(StateType state) {
    return (get_unchecked<StateType>().state.load() == state);
  }

  template<typename StateType>
  auto get_unchecked() -> State<StateType>& {
    constexpr auto sid = core::type_sid<StateType>();
    auto it = lookup_.find(sid);
    assert(it != lookup_.end());

    return *static_cast<State<StateType>*>(it->second.get());
  }

private:

private:
  HashMap<StringID, Unique<StateBase>> lookup_;
  Events& events_;
};

#endif