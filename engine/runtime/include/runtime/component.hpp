#pragma once
#include "events.hpp"
#include "core/tuple.hpp"

enum struct ComponentID: u64 {
  Invalid = u64(-1)
};

//TODO: remove that 
struct Component {
  struct Defaults {
    using Replicated = False;
  };

protected:
  struct NoDefaults {};
  //using Replicated = Defaults::Replicated;
  struct Replicated {}; // = std::conditional_t<value, True, False>;

  //using NetRelevancy = None;
};

template<typename T>
concept ComponentType = std::derived_from<T, Component>;

template<ComponentType C>
struct ComponentStatics {
  //static inline void* info = register_component();
};

class ComponentSetBase {};

template <typename Target, typename... Pack>
constexpr bool contains_v = std::disjunction_v<std::is_same<Target, Pack>...>;

// 1. A helper to count how many times a specific type (T) appears in the pack
template <typename T, typename... Ts>
inline constexpr std::size_t type_count_v = (std::is_same_v<T, Ts> +...);

// 2. A concept that checks if a parameter pack has ANY duplicate types
template <typename... Ts>
concept PackHasDuplicates = ((type_count_v<Ts, Ts...> > 1) || ...);

template<typename T>
struct OneOrPack {
  using type = T;
};

template<typename... Comps>
class ComponentSet : public ComponentSetBase {
  //static_assert(!PackHasDuplicates<Comps...>, "A component type occurs more than once");
public:
  friend class World;

  using Components = Tuple<Comps...>;

  ComponentSet() = default;
  ~ComponentSet() = default;

  explicit ComponentSet(Comps&&... arg)
    : components_{ CORE_FWD(arg)... }
  {
  }

  template<typename... Args> requires(!PackHasDuplicates<Args...>)
  explicit ComponentSet(Args&&... args)
    : components_{}
  {
    //static_assert((contains_v<Args, Comps...> && ...), "Component type in set was unexpected");
    ((core::get<Args>(components_) = CORE_FWD(args)), ...);
  }

private:
  using _BaseClass = ComponentSet;

private:
  Components components_{};
};
