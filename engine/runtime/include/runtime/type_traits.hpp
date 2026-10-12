#pragma once
#include<type_traits>

using True  = std::true_type;
using False = std::false_type;

template<typename T>
struct Emitter;

namespace traits {
  namespace _ {
    template<typename T>
    struct decay_functor {
      using type = decay_functor<decltype(&T::operator())>::type;
    };

    template<typename Instance, typename... Args>
    struct decay_functor<void(Instance::*)(Args...)> {
      using type = void(Args...);
    };

    template<typename Instance, typename... Args>
    struct decay_functor<void(Instance::*)(Args...) const> {
      using type = void(Args...);
    };

    template<typename... Args>
    struct decay_functor<void(*)(Args...)> {
      using type = void(Args...);
    };
  
    template<typename T>
    struct is_emitter {
      static constexpr bool value = false;
    };

    template<typename T>
    struct is_emitter<Emitter<T>> {
      static constexpr bool value = true;
    };

    template<typename T>
    struct is_qeury {
      static constexpr bool value = false;
    };
  }
}