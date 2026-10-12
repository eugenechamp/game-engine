#pragma once
#include "fwd.hpp"
#include <type_traits>
#include <concepts>

#include "def.hpp" // remove
#include "meta.hpp" // remove

/// @brief The most basic subsystem class that the engine provides.
/// It's usually used for creating standalone subsystems that are not a part of any subsystem family
class Subsystem {
  friend App;
  friend class SubsystemSet;
public:
  using InSet = class SubsystemSet;

public: // it was protected initially
  Subsystem() = delete;
  explicit Subsystem(App& app);
  Subsystem(const Subsystem&) = delete;
  Subsystem(Subsystem&&) = delete;
  
  auto operator=(const Subsystem&) = delete;
  auto operator=(Subsystem&&) = delete;

public:
  virtual ~Subsystem() = default;
  
protected:
  virtual void init() {}
  virtual void shutdown() {}
  
  virtual void fixed_update(float dt) = 0;
  virtual void update(float dt)       = 0;

  virtual auto fixed_timestep() -> Duration;

  virtual void start() {}
  virtual void finish() {}

  virtual bool run_condition() { 
    return true; // always runs by default
  }
  virtual bool shutdown_condition() { 
    return false; // never shutdowns by default 
  }

  // don't use it
  virtual void serialize()              {}
  virtual void deserialize()            {}

protected:
  auto world() -> World& {
    return world_;
  }

  auto app() -> App& {
    return app_;
  }

  auto events() -> Events& {
    return events_;
  }

  [[noreturn]] auto cmd() -> Commands& {
  }
  
  bool active() const { 
    return is_active_; 
  }
  
  bool active(bool value) { 
    is_active_ = value;
  }

private:
  App& app_;
  Events& events_;
  World& world_;
  bool is_active_;
};

#include <queue>

template<typename T>
class DirectedGraph {
private:
  HashMap<T, Vec<T>> adj_list_;
public:
  // Initialize the graph with a set number of nodes
  
  void add_node(const T& node) {
    adj_list_.emplace(std::pair{ node, Vec<T>{} });
  }

  // Add a directed edge from source node 'u' to destination node 'v'
  void add_edge(const T& u, const T& v) {
    auto& list = adj_list_[u];

    if (!adj_list_.contains(v))
    {
      adj_list_.emplace(std::pair{ v, Vec<T>{} });
    }

    list.push_back(v);
  }

  // Kahn's Algorithm to process a Topological Sort and verify it is a DAG
  bool validate_and_sort(Vec<T>& order) {
    HashMap<T, int> in_degree;

    for (auto& [key, value] : adj_list_)
    {
      in_degree[key] = 0;
    }

    // Step 1: Calculate the incoming edge degrees for every vertex

    for (auto& [key, value] : adj_list_)
    {
      for (const T& neighbor : value)
      {
        in_degree[neighbor]++;
      }
    }

    // for (int i = 0; i < adjList.size(); ++i) {
    //     for (T& neighbor : adjList[i]) {
    //         inDegree[neighbor]++;
    //     }
    // }

    // Step 2: Queue all vertices with no dependencies (in-degree 0)
    // std::queue<int> zero_in_degree;
    // for (int i = 0; i < numVertices; ++i) {
    //     if (inDegree[i] == 0) {
    //         zero_in_degree.push(i);
    //     }
    // }

    std::queue<T> zero_in_degree;
    for (auto& [key, value] : in_degree) {
      if (value == 0) {
        zero_in_degree.push(key);
      }
    }

    // Step 3: Process the queue and reduce neighboring degrees
    while (!zero_in_degree.empty()) {
      T current = zero_in_degree.front();
      zero_in_degree.pop();
      order.push_back(current);

      for (auto& neighbor : adj_list_[current]) {
        in_degree[neighbor]--;
        if (in_degree[neighbor] == 0) {
          zero_in_degree.push(neighbor);
        }
      }
    }
    std::cout << "order: " << order.size() << " " << adj_list_.size() << "\n";

    // If the processed order does not equal total vertices, a cycle exists
    return order.size() == adj_list_.size();
  }
};

template<typename T>
concept SubsystemConcept = std::derived_from<T, Subsystem>;

#include "time.hpp"

struct SubsystemBuildResult {
  Vec<StringID> before;
  Vec<StringID> after;
};

class SubsystemSet {
  friend App;
private:
  struct SubsystemInfo {
    Unique<Subsystem> subsystem;
    Vec<StringID> before;
    Vec<StringID> after;
    float time_elapsed = 0.f;
    bool was_running_last_frame;
  };

public:
  SubsystemSet(App& app)
    : app{ app }
  { }
  
  virtual ~SubsystemSet() = default;

  virtual bool run_condition() {
    return true;
  }

  virtual bool shutdown_condition() {
    return false;
  }

  void update(float dt) {
    if (needs_rebuild_)
    {
      build();
      needs_rebuild_ = false;
    }

    for (SubsystemInfo* info : subsystems_in_order_)
    {
      Subsystem& subsystem = *info->subsystem;
      float& time_elapsed = info->time_elapsed;

      if (subsystem.run_condition())
      {
        if (!info->was_running_last_frame)
        {
          subsystem.start();
          info->was_running_last_frame = !info->was_running_last_frame;
        }

        float fixed_timestep = subsystem.fixed_timestep().sec();

        i32 fixed_step_updates = 0;
        time_elapsed += dt;
        while ((time_elapsed - fixed_timestep) >= fixed_timestep)
        {
          fixed_step_updates++;
          time_elapsed -= fixed_timestep;
        }

        for (i32 i = 0; i < fixed_step_updates; i++)
        {
          subsystem.fixed_update(fixed_timestep);
        }
        subsystem.update(dt);
      }
      else
      {
        if (info->was_running_last_frame)
        {
          subsystem.finish();
          info->was_running_last_frame = !info->was_running_last_frame;
        }
      }

      if (subsystem.shutdown_condition())
      {
        subsystem.shutdown();
      }
    }
  }

  void build() {
    subsystems_in_order_.clear();
    graph_.validate_and_sort(subsystems_in_order_);
  }

  template<typename T, typename... Args>
  void add_subsystem(SubsystemBuildResult result, Args&&... args) {
    auto* subsystem = (Subsystem*)std::malloc(sizeof(T));
    auto sid = core::type_sid<T>();
    Unique<SubsystemInfo>& info = subsystems_[sid];
      
    info = Unique<SubsystemInfo>(new SubsystemInfo(
      Unique<Subsystem>(subsystem), 
      std::move(result.before), 
      std::move(result.after), 
      false 
    ));
    
    graph_.add_node(subsystems_[sid].get());

    for (auto& before : info->before)
    {
      graph_.add_edge(subsystems_[sid].get(), subsystems_[before].get());
    }

    for (auto& after : info->after)
    {
      graph_.add_edge(subsystems_[after].get(), subsystems_[sid].get());
    }

    ::new((void*)subsystem) T(app, std::forward<Args>(args)...);
    subsystem->init();

    needs_rebuild_ = true;

    //add_subsystem_impl(std::move(make_subsystem));
  }

  void add_subsystem_impl(Fn<void()>);

private:
  // idk if subsystem sets should manage subsystems. it's probably better to delegate that 
  // to SubsystemManager in the future, no?
  App& app;
  HashMap<StringID, Unique<SubsystemInfo>> subsystems_;
  Vec<SubsystemInfo*> subsystems_in_order_;
  DirectedGraph<SubsystemInfo*> graph_;
  bool needs_rebuild_{};
};

template<typename T>
concept SubsystemSetConcept = std::derived_from<T, SubsystemSet>;

#define IN_SET(SubsystemSetType) static_assert(SubsystemSetConcept<SubsystemSetType>, #SubsystemSetType " is not of SubsystemSet type"); using InSet = SubsystemSetType

// move that shit elsewhere
class SubsystemInstanceRegistry {
  friend App;
public:
  static bool contains(Subsystem* subsystem) {
    return address_to_sid_.contains(subsystem);
  }

private:
  static void register_subsystem(StringID sid, Subsystem* subsystem) {
    sid_to_address_.emplace(std::pair{ sid, subsystem });
    address_to_sid_.emplace(std::pair{ subsystem, sid });
  }

  static void unregister_subsystem(Subsystem* subsystem) {
    // sid_to_address_.erase(std::pair{ sid, subsystem });
    // address_to_sid_.erase(std::pair{ subsystem, sid });
  }

private:
  static inline HashMap<StringID, Subsystem*> sid_to_address_;
  static inline HashMap<Subsystem*, StringID> address_to_sid_;
};

class OnStartupSubsystemSet : public SubsystemSet {};
class OnStartup : public Subsystem {
public:
  // using InSet = OnStartupSubsystemSet;
  IN_SET(OnStartupSubsystemSet);
  using Subsystem::Subsystem;
};

class BeginFrameSet : public SubsystemSet {
  using SubsystemSet::SubsystemSet;
};

class BeginFrame : public Subsystem {
public:
  using InSet = BeginFrameSet;
  using Subsystem::Subsystem;
};

class PreUpdateSet : public SubsystemSet {
  using SubsystemSet::SubsystemSet;
};

class PreUpdate : public Subsystem {
public:
  using InSet = PreUpdateSet;
  using Subsystem::Subsystem;
};

class OnUpdateSet : public SubsystemSet {
  using SubsystemSet::SubsystemSet;
};

class OnUpdate : public Subsystem {
public:
  using InSet = OnUpdateSet;
  using Subsystem::Subsystem;
};

class PostUpdateSet : public SubsystemSet {
  using SubsystemSet::SubsystemSet;
};

class PostUpdate : public Subsystem {
public:
  using InSet = PostUpdateSet;
  using Subsystem::Subsystem;
};

class EndFrameSet : public SubsystemSet {
  using SubsystemSet::SubsystemSet;
};
class EndFrame : public Subsystem {
public:
  using InSet = EndFrameSet;
  using Subsystem::Subsystem;
};

//class PrePhysicsUpdate : public Subsystem {
//public:
//  using InSet = PrePhysicsUpdate;
//  using Subsystem::Subsystem;
//};
//
//class PhysicsUpdate : public Subsystem {
//public:
//  using InSet = PhysicsUpdate;
//  using Subsystem::Subsystem;
//};
//
//class PostPhysicsUpdate : public Subsystem {
//public: 
//  using Family = PostPhysicsUpdate;
//  using Subsystem::Subsystem;
//};


//template<typename SubsystemType>
//concept IsSubsystemFamilyMember = requires {
//  {
//    std::is_void_v<std::void_t<typename SubsystemType::Family>> &&
//    std::is_base_of_v<SubsystemFamily, typename SubsystemType::Family> &&
//    std::is_base_of_v<typename SubsystemType::Family, SubsystemType> 
//  };
//};

#if 0
struct SubsystemBuilder {
private:
  friend App;
  SubsystemBuilder() = delete;
public:
  template<std::derived_from<Subsystem> SubsystemType>
  auto before() && -> SubsystemBuilder& {

  }

  ~SubsystemBuilder() {
    build();
  }

private:
  void build() {

  }
};

struct SubsystemFamilyBuilder {
  friend App;
private:
  SubsystemFamilyBuilder() {

  }
public:
  template<IsSubsystemFamily Family>
  auto before() && -> SubsystemFamilyBuilder& {

  }

  template<IsSubsystemFamily Family>
  auto after() && -> SubsystemFamilyBuilder& {

  }

  template<IsStandaloneSubsystem Family>
  auto after() && -> SubsystemFamilyBuilder& {

  }
};

template<typename Family>
struct SubsystemFamilyMemberBuilder {
  friend App;
private:
  SubsystemFamilyMemberBuilder(SubsystemSchedule& schedule) {
  }

public:
  template<IsSubsystemFamilyMember<Family> S>
  auto before() && -> SubsystemFamilyBuilder& {
    static_assert(std::is_base_of_v<Family, S> && std::is_same_v<Family, S::Family>,
      "order dependency can only be set for subsystems that are members of the same family group");
  }

  template<IsSubsystemFamilyMember<Family> S>
  auto after() && -> SubsystemFamilyBuilder& {
    static_assert(std::is_base_of_v<Family, S> && std::is_same_v<Family, S::Family>,
      "order dependency can only be set for subsystems that are members of the same family group");
  }

  // rm that
  void args(auto&&... ctor_args) {

  }

  ~SubsystemFamilyBuilder() {

  }
};
#endif // 0