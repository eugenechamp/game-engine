#pragma once

#ifndef FLECS_CPP
  #define FLECS_CPP
#endif
#include "log.hpp"

#include <flecs.h>
#include <core/tuple.hpp>

#include "def.hpp"

#include "events.hpp"
#include "app.hpp"
#include "component.hpp"

struct Entity;
enum struct EntityID: u64;

//enum struct entity_t : native_entity_t {};
class World;

template<typename P1, typename P2>
struct ExcludeArgs;

template<typename... Ts>
struct ParamPack {};

template<typename... A, typename... B>
struct ExcludeArgs<ParamPack<A...>, ParamPack<B...>>
{
private:
  static constexpr size_t N =
    (sizeof...(A) < sizeof...(B)) ? sizeof...(A) : sizeof...(B);

  template<size_t, typename...>
  struct drop;

  template<size_t N0, typename T, typename... Ts>
  struct drop<N0, T, Ts...> : drop<N0 - 1, Ts...> {};

  template<typename T, typename... Ts>
  struct drop<0, T, Ts...> {
    using type = ParamPack<T, Ts...>;
  };

  template<size_t N0>
  struct drop<N0> {
    using type = ParamPack<>;
  };

public:
  using Type = typename drop<N, A...>::type;
};

template<size_t NUM, typename... A>
struct ExcludeArgsN
{
private:
  static constexpr size_t N =
    (sizeof...(A) < NUM) ? sizeof...(A) : NUM;

  template<size_t, typename...>
  struct drop;

  template<size_t N0, typename T, typename... Ts>
  struct drop<N0, T, Ts...> : drop<N0 - 1, Ts...> {};

  template<typename T, typename... Ts>
  struct drop<0, T, Ts...> {
    using type = ParamPack<T, Ts...>;
  };

  template<size_t N0>
  struct drop<N0> {
    using type = ParamPack<>;
  };

public:
  using Type = typename drop<N, A...>::type;
};

template<typename T>
struct is_term_operator {
  static constexpr bool value = false;
};

//template<typename... Ts>
//struct is_term_operator <With<Ts...>> {
//  static constexpr bool value = true;
//};
//template<typename... Ts>
//struct is_term_operator <Without<Ts...>> {
//  static constexpr bool value = true;
//};
//template<typename... Ts>
//struct is_term_operator <Or<Ts...>> {
//  static constexpr bool value = true;
//};
//template<typename... Ts>
//struct is_term_operator <Opt<Ts...>> {
//  static constexpr bool value = true;
//};
//template<typename... Ts>
//struct is_term_operator<Term<Ts...>> {
//  static constexpr bool value = true;
//};
//
//template<typename T>
//concept IsTermOperator = is_term_operator<T>::value;

//template<typename... Args>
//struct GetTermsOperatorPack {
//private:
//  static constexpr size_t N = sizeof...(Args);
//
//  template<typename...>
//  struct drop;
//
//  template<IsTermOperator T, typename... Ts>
//  struct drop<T, Ts...> : drop<Ts...> {};
//
//  template<typename T, typename... Ts>
//  struct drop<T, Ts...> {
//    using type = ParamPack<T, Ts...>;
//  };
//
//  template<>
//  struct drop {
//    using type = ParamPack<>;
//  };
//
//public:
//  using Type = typename drop<N, Args...>::type;
//};

template<typename...Comps>
struct Query;

template<typename T>
struct is_query {
  static constexpr bool value = false;
};

template<typename... Comps>
struct is_query<Query<Comps...>> {
  static constexpr bool value = true;
};

template<typename T>
struct WorldUpdateParamPack { using Type = WorldUpdateParamPack<decltype(&T::operator())>::Type; };

template<typename...Args>
struct WorldUpdateParamPack<void(*)(Args...)> {
  using Type = ParamPack<Args...>; 
};

//template<typename...Args>
//struct WorldUpdateParamPack<void(*)(float, Args...)> {
//  using Type = ParamPack<Args...>;
//};

template<typename Instance, typename...Args>
struct WorldUpdateParamPack<void(Instance::*)(Args...)> {
  using Type = ParamPack<Instance*, Args...>;
};

//template<typename Instance, typename...Args>
//struct WorldUpdateParamPack<void(Instance::*)(float, Args...)> {
//  using Type = ParamPack<Args...>;
//};
//
//template<typename Instance, typename...Args>
//struct WorldUpdateParamPack<void(Instance::*)(float, Args...) const> {
//  using Type = ParamPack<Args...>;
//};

template<typename Instance, typename...Args>
struct WorldUpdateParamPack<void(Instance::*)(Args...) const> {
  using Type = ParamPack<const Instance*, Args...>;
};

template<typename T>
struct InvokeWithParam {
  void call(auto&&...) = delete;
};

template<typename F>
struct WorldUpdateRunner {
  friend class World;
  using Runnable = F;
public:
  auto run() -> std::invoke_result_t<F> {
    assert(guard && "WorldUpdateRunner::run was called two or more times for one instance");

    guard = false;
    if constexpr (std::is_void_v<std::invoke_result_t<F>>)
    {
      fn_();
      fn_.~F();
    }
    else
    {
      auto result = fn_();
      fn_.~F();
      return result;
    }
  }
  
  // can be synced with World::join()
  auto async() {
    static_assert(false, "Unimplemented");
  }
  // put on another thread, sync at the end of the frame
  void detach() {
    static_assert(false, "Unimplemented");
  }

private:
  WorldUpdateRunner(F&& _fn)
    : fn_{ CORE_MOVE(_fn) }
  { }
public:
  WorldUpdateRunner(const WorldUpdateRunner&) = delete;
  WorldUpdateRunner(WorldUpdateRunner&&)      = delete;
  auto operator=(const WorldUpdateRunner&)    = delete;
  auto operator=(WorldUpdateRunner&&)         = delete;
  
private:
  Runnable  fn_;
  bool guard = true;
};

class World {
  template<typename...Ts>
  friend struct Query;
  
public:
  World(App& app)
    : ecs_{}, 
      local_player_{},
      app_{app}
  {

  }
  
  template<typename Q>
  auto make_query() {
    return Q(*this);
  }

  template<typename... Comps>
  auto create(Comps&&... components) -> Entity;
  
  // remove that probably
  template<typename... Comps>
  auto create_named(std::string_view name, Comps&&... components) -> Entity;

  template<typename... Comps>
  auto create() -> Entity;

  template<typename... Comps>
  auto create_named(std::string_view name) -> Entity;

  auto make_entity_impl(EntityID entity) -> Entity;

  template<typename... Comps>
  auto prefab() -> Entity;

  template<typename... Comps>
  auto prefab(Comps&&... components) -> Entity;

  template<typename T>
  auto get_res();

  template<ComponentType C>
  void add_component(EntityID entity);

  template<typename T>
  void register_ecs_events();

  template<ComponentType C>
  void add_component(EntityID entity, C&& component);

  template<ComponentType C>
  auto get_component(EntityID entity) -> const C*;

  template<ComponentType C>
  auto get_mut_component(EntityID entity) -> C*;

  template<ComponentType C>
  void remove_component(EntityID entity);

  void destroy(EntityID entity);
  
  template<ComponentType C>
  bool has_component(EntityID ent);

  auto find_entity(std::string_view name) -> Entity;
  
  /*template<typename T>
  void observer(Fn<void(OnAdd<T>)> callback) {
    ecs_.observer<T>()
      .event(flecs::OnAdd)
      .each([call = std::move(callback)](flecs::entity e, T& comp) {
        call(OnAdd({}, Entity(e), comp));
      });
  }*/

  /*template<typename T>
  void observer(Fn<void(OnRemove<T>)> callback) {
    ecs_.observer<T>()
      .event(flecs::OnRemove)
      .each([call = std::move(callback)](flecs::entity e, T& comp) {
        call(OnRemove({}, Entity(e), comp));
      });
  }
*/

  //template<typename Fx>
  //void update(Fx&& callable) {
  //  InvokeWithParam<typename WorldUpdateParamPack<std::remove_cvref_t<std::decay_t<Fx>>>::Type>().call(*this, callable);
  //}

  //template<typename Fx>
  //void update(Fx&& callable, float dt) {
  //  InvokeWithParam<typename WorldUpdateParamPack<std::remove_cvref_t<std::decay_t<Fx>>>::Type>().call(*this, callable, dt);
  //}

  //template<typename Fx, typename Instance>
  //void update(Fx&& callable, Instance* instance) {
  //  InvokeWithParam<typename WorldUpdateParamPack<std::remove_cvref_t<std::decay_t<Fx>>>::Type>().call(*this, callable, instance);
  //}

  //template<typename Fx, typename Instance>
  //void update(Fx&& callable, Instance* instance, float dt) {
  //  InvokeWithParam<typename WorldUpdateParamPack<std::remove_cvref_t<std::decay_t<Fx>>>::Type>().call(*this, callable, instance, dt);
  //}

  template<typename Fx, typename... Args>
  auto update(Fx&& callable, Args&&... args) -> decltype(auto) {
    //using Res = typename ExcludeArgs<typename WorldUpdateParamPack<std::remove_cvref_t<std::decay_t<Fx>>>::Type, ParamPack<Args...>>::Type;
    using FnParams = typename WorldUpdateParamPack<std::remove_cvref_t<std::decay_t<Fx>>>::Type;
    using Res = typename ExcludeArgs<FnParams, ParamPack<Args...>>::Type;
    
    /*log_info("CallerArgs: {},", typeid(ParamPack<Args...>).name());
    log_info("Fn params: {}", typeid(FnParams).name());
    log_info("Res: {}", typeid(Res).name());*/
    return WorldUpdateRunner {
      [...params = CORE_FWD(args), callable, &world = *this]() mutable {
        InvokeWithParam<Res>().call(world, callable, CORE_FWD(params)...);
      }
    };

    //InvokeWithParam<Res>().call(*this, callable, std::forward<Args>(args)...);
  }

  //template<typename Fx, typename Arg, typename... Args>
  //void update(Fx&& callable, Arg arg, Args... args) {
  //  //using Res = typename ExcludeArgs<typename WorldUpdateParamPack<std::remove_cvref_t<std::decay_t<Fx>>>::Type, ParamPack<Args...>>::Type;
  //  using FnParams = typename WorldUpdateParamPack<std::remove_cvref_t<std::decay_t<Fx>>>::Type;
  //  using Res = typename ExcludeArgs<FnParams, ParamPack<Arg, Args...>>::Type;
  //  log_info("CallerArgs: {}", typeid(ParamPack<Args...>).name());
  //  log_info("Fn params: {}", typeid(FnParams).name());
  //  log_info("Res: {}", typeid(Res).name());

  //  InvokeWithParam<Res>().call(*this, callable, std::forward<Arg>(arg), std::forward<Args>(args)...);
  //}

  //template<typename Fx>
  //void update(Fx&& callable) {
  //  using Res = typename WorldUpdateParamPack<std::remove_cvref_t<std::decay_t<Fx>>>::Type;
  //  log_info("Res: {}", typeid(Res).name());

  //  InvokeWithParam<Res>().call(*this, callable);
  //}

  auto local_player() {
    return local_player_;
  }

public:
  /// TODO: get rid of it and abstract away
  auto get() -> flecs::world& {
    return ecs_;
  }

private:
  template<typename T>
  auto set_component(flecs::entity entity, T&& value) {
    if constexpr (ComponentType<T>)
    {
      if constexpr (std::is_empty_v<T>)
      {
        add_component<T>(EntityID(entity.id()));
      }
      else
      {
         add_component<T>(EntityID(entity.id()), CORE_FWD(value));
      }
    }
    else
    {
      core::apply([this, entity](auto&&... args) {
        (set_component(entity, CORE_FWD(args)), ...);
        }, CORE_MOVE(value.components_));
    }
  }

private:
  flecs::world ecs_;
  flecs::entity local_player_;
  App& app_;
};

template<typename... Ts>
struct InvokeWithParam<ParamPack<Ts...>> {
  //template<typename Fx, typename Instance>
  //void call(World& world, Fx&& callable, Instance* instance) {
  //  std::invoke(callable, instance, world.get_res<Ts>()...);
  //}

  //template<typename Fx, typename Instance>
  //void call(World& world, Fx&& callable, Instance* instance, float dt) {
  //  std::invoke(callable, instance, dt, world.get_res<Ts>()...);
  //}

  //template<typename Fx>
  //void call(World& world, Fx&& callable) {
  //  std::invoke(callable, world.get_res<Ts>()...);
  //}

  //template<typename Fx>
  //void call(World& world, Fx&& callable, float dt) {
  //  //callable(dt, world.get_res<Ts>()...);
  //  std::invoke(callable, dt, world.get_res<Ts>()...);
  //}

  ///--------------------

  template<typename Fx, typename... Args>
  void call(World& world, Fx&& callable, Args&&... args) {
    std::invoke(callable, CORE_FWD(args)..., world.get_res<Ts>()...);
  }

  /*template<typename Fx>
  void call(World& world, Fx&& callable) {
    std::invoke(callable, world.get_res<Ts>()...);
  }

  template<typename Fx, typename Arg, typename... Args>
  void call(World& world, Fx&& callable, Arg&& arg, Args&&... args) {
      std::invoke(callable, std::forward<Arg>(arg), std::forward<Args>(args)..., world.get_res<Ts>()...);
  }*/

  //template<typename Fx>
  //void call(World& world, Fx&& callable) {
  //  std::invoke(callable, world.get_res<Ts>()...);
  //}
};

template<typename... Comps>
struct With {};
template<typename... Comps>
struct Without {};
template<typename... Comps>
struct Or {};
template<typename... Comps>
struct Opt {};

template<typename... Terms>
struct Term {};

template<typename... Comps>
struct QueryBase {
public:
  
  /*template<typename... Terms>
  auto build(Term<Terms...>, World& world)
  {
    auto query_builder = world().get().query_builder<Comps...>();
    
    (add_term_operator<Terms>(query_builder), ...);
    
    return query_builder.build();
  }

  template<typename T>
  void add_term_operator(T, auto& query_builder) { }
  
  template<typename... Ts>
  void add_term_operator(With<Ts...>, auto& query_builder) {
    (query_builder.with<Ts>(), ...);
  }

  template<typename... Ts>
  void add_term_operator(Without<Ts...>, auto& query_builder) {
    (query_builder.without<Ts>(), ...);
  }

  template<typename... Ts>
  void add_term_operator(Or<Ts...>, auto& query_builder) {
    using Others = void;
    using Last = void;
    (query_builder.with<Ts>().or_(), ...);
    query_builder.with<Last>();
  }
  
  template<typename... Ts>
  void add_term_operator(Opt<Ts...>, auto& query_builder) {
    (query_builder.with<Ts>().optional(), ...);
  }*/
};

template<typename...Comps>
struct Query {
  friend World;
private:
  Query(World& world) {
    query_ = world.get()
      .query_builder<Comps...>()
      .build();
  }

public:
  void for_each(auto&& callable) {
    query_.each(callable);
  }
private:
  flecs::query<Comps...> query_;
};

template<typename... Operators, typename...Comps>
struct Query<Term<Operators...>, Comps...> {
  friend World;
private:
  Query(World& world) {
    query_ = world.get()
      .query_builder<Comps...>()
      .build();
  }

public:
  void for_each(auto&& callable) {
    query_.each(callable);
  }
private:
  flecs::query<Comps...> query_;
};

template<typename... Includes, typename...Comps>
struct Query<With<Includes...>, Comps...> {
  friend World;
private:
  Query(World& world) 
    : query_(build(world))
  { }

public:
  void for_each(auto&& callable) {
    query_.each(callable);
  }
  
private:
  auto build(World& world) {
    auto query_builder = world.get().query_builder<Comps...>();
    (query_builder.with<Includes>(), ...);
    return query_builder.build();
  }

  flecs::query<Comps..., Includes...> query_;
};

template<typename... Excludes, typename...Comps>
struct Query<Without<Excludes...>, Comps...> {
  friend World;
private:
  Query(World& world) 
    : query_ (world.get()
      .query_builder<Comps...>()
      .without<Excludes...>()
      .build())
  {}

public:
  void for_each(auto&& callable) {
    query_.each(callable);
  }
private:
  flecs::query<Comps..., Excludes...> query_;
};

template<typename... Includes, typename... Excludes, typename...Comps>
struct Query<With<Includes...>, Without<Excludes...>, Comps...> {
  friend World;
private:
  Query(World& world) 
    : query_(build(world))
  { }
  
  auto build(World& world) {
    auto query_builder = world.get().query_builder<Comps...>();
    (query_builder.with<Includes>(), ...);
    (query_builder.without<Excludes>(), ...);
    return query_builder.build();
  }

public:
  void for_each(auto&& callable) {
    query_.each(callable);
  }
private:
  flecs::query<Comps..., Includes..., Excludes...> query_;
};

template<typename... Ts, typename...Comps>
struct Query<Or<Ts...>, Comps...> {
  friend World;
private:
  Query(World& world) 
    : query_(build(world))
  {}

  auto build(World& world){
    auto query_builder = world.get().query_builder<Comps...>();
    (query_builder.with<Ts>().or_(), ...);
    return query_builder.build();
  }

public:
  void for_each(auto&& callable) {
    query_.each(callable);
  }

private:
  flecs::query<Comps...> query_;
};

#include "world.inl"