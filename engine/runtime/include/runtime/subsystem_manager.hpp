#pragma once
#include "subsystem.hpp"
#include "def.hpp"
#include <utility>

#include "commands.hpp"

struct SubsystemSetBuildResult {
  Vec<StringID> before;
  Vec<StringID> after;
};

struct SubsystemSetInfo {
  Unique<SubsystemSet> set;
  Vec<StringID>        before;
  Vec<StringID>        after;
  bool                 needs_rebuild;
};

class SubsystemManager {
  friend class App;
private:

public:
  SubsystemManager(App& app, Commands& commands)
    : app_{ app }, commands_{ commands }
  { }

  template<typename T, typename... Args> //requires(SubsystemSetConcept<typename T::InSet>)
  void add_subsystem(SubsystemBuildResult result, Args&&... args) {
    commands_.enqueue([this, result = CORE_MOVE(result), ...args = args]() mutable {
      constexpr auto sid = core::type_sid<typename T::InSet>();
    
      if (!sets_.contains(sid))
        return;
    
      SubsystemSetInfo& info = *sets_[sid];
      info.set->add_subsystem<T>(std::move(result), std::forward<Args>(args)...);
    });
  }

  template<SubsystemSetConcept T> 
  void add_set(SubsystemSetBuildResult result) {
    commands_.enqueue([this, result = CORE_MOVE(result)]() mutable {
      constexpr auto sid = core::type_sid<T>();
      auto& info = sets_[sid];

      if(!info)
      {
        info = Unique<SubsystemSetInfo>(new SubsystemSetInfo);
      }

      info->set = Unique<T>{ (T*)std::malloc(sizeof(T)) };
      
      info->before = CORE_MOVE(result.before);
      info->after  = CORE_MOVE(result.after);
      
      subsystem_set_graph_.add_node(sid);

      for(auto& before : info->before)
      {
        /*
        if (!sets_.contains(before))
        {
          sets_.emplace(std::pair{ before, {} });
        }*/
        
        // FIXME: the issue here is that we create a new entry in a hash_map with before key, copy a pointer (null) and then
        // when comes the actual creation, we assign a new value but the copy remains with null inside the graph.
        // we now have 3 nodes instead of 2
        subsystem_set_graph_.add_edge(sid, before);
      }

      for(auto& after : info->after) 
      {
        /*if (!sets_.contains(after))
        {
          sets_.emplace(std::pair{ after, {} });
        }*/

        subsystem_set_graph_.add_edge(after, sid);
      }
      
      ::new((void*)info->set.get()) T(app_);

      dirty(true);
    });
  }


  void update(float dt) {
    if(dirty_)
    {
      subsystem_set_graph_.validate_and_sort(order_);
      dirty_ = false;
    }

    for(StringID sid : order_) 
    {
      auto& set = sets_[sid]->set;

      if(set->run_condition())
      {
        set->update(dt);
      }
    }
  }

  void dirty(bool value) {
    dirty_ = value;
  }
  
private:
  App&                                             app_;
  Commands&                                   commands_;
  DirectedGraph<StringID>        subsystem_set_graph_{};
  HashMap<StringID, Unique<SubsystemSetInfo>> sets_{};
  Vec<StringID>                               order_{};
  bool                                        dirty_ = { true };
};

//template<typename T>
//class DirectedGraph {
//private:
//  HashMap<T, Vec<T>> adj_list_;
//public:
//  // Initialize the graph with a set number of nodes
//
//  // Add a directed edge from source node 'u' to destination node 'v'
//  void add_edge(const T& u, T&& v) {
//    auto& list = adj_list_[u];
//
//    if(!adj_list_.contains(v))
//    {
//      adj_list_.emplace(std::pair{v, std::vector<T>{}});
//    }
//
//    list.push_back(v);
//  }
//
//  // Kahn's Algorithm to process a Topological Sort and verify it is a DAG
//  bool topological_sort_kahn(std::vector<T>& order) {
//    HashMap<T, int> in_degree;
//
//    for(auto& [key, value] : adj_list_) 
//    {
//      in_degree[key] = 0;
//    }
//
//    // Step 1: Calculate the incoming edge degrees for every vertex
//    
//    for(auto& [key, value] : adj_list_) 
//    {
//      for(const T& neighbor : value) 
//      {
//        in_degree[neighbor]++;
//      }
//    }
//    
//    // for (int i = 0; i < adjList.size(); ++i) {
//    //     for (T& neighbor : adjList[i]) {
//    //         inDegree[neighbor]++;
//    //     }
//    // }
//
//    // Step 2: Queue all vertices with no dependencies (in-degree 0)
//    // std::queue<int> zero_in_degree;
//    // for (int i = 0; i < numVertices; ++i) {
//    //     if (inDegree[i] == 0) {
//    //         zero_in_degree.push(i);
//    //     }
//    // }
//
//    std::queue<T> zero_in_degree;
//    for (auto& [key, value] : in_degree)
//    {
//      if (value == 0) 
//      {
//        zero_in_degree.push(key);
//      }
//    }
//
//    // Step 3: Process the queue and reduce neighboring degrees
//    while (!zero_in_degree.empty()) 
//    {
//      T current = zero_in_degree.front();
//      zero_in_degree.pop();
//      order.push_back(current);
//
//      for (auto& neighbor : adj_list_[current]) 
//      {
//        in_degree[neighbor]--;
//        if (in_degree[neighbor] == 0) 
//        {
//          zero_in_degree.push(neighbor);
//        }
//      }
//    }
//    // std::cout << order.size() << " " << adj_list_.size();
//
//    // If the processed order does not equal total vertices, a cycle exists
//    return order.size() == adj_list_.size();
//  }
//};


class SubsystemManager;

template<SubsystemSetConcept T>
struct SubsystemSetBuilder {
private:
  friend class App;
  friend class SubsystemManager;
  
  SubsystemSetBuilder(SubsystemManager* manager)
    : manager_{ manager }, 
      result_{}
  {}

public:
  SubsystemSetBuilder(const SubsystemSetBuilder&) = delete;
  SubsystemSetBuilder(SubsystemSetBuilder&&) = delete;

  auto operator=(const SubsystemSetBuilder&) = delete;
  auto operator=(SubsystemSetBuilder&&) = delete;

public:
  using Self = SubsystemSetBuilder;
  
  template<SubsystemSetConcept S>
  auto before() -> Self& {
    result_.before.push_back(core::type_sid<S>());
    return *this;
  }
  
  template<SubsystemSetConcept S>
  auto after() -> Self& {
    result_.after.push_back(core::type_sid<S>());
    return *this;
  }
  
  ~SubsystemSetBuilder() {
    build();
  }

private:  
  void build() {
    #if _MSVC_VER
      manager_->template add_set<T>(CORE_MOVE(result_));
    #else
      manager_->add_set<T>(CORE_MOVE(result_));
    #endif
  }

private:
  SubsystemManager* manager_;
  SubsystemSetBuildResult result_;
};

#include <tuple>

template<typename T, typename... Args>
struct SubsystemBuilder {
private:
  friend class App;
  friend class SubsystemManager;

  SubsystemBuilder(SubsystemManager* manager, Args&&... args)
    : manager_{ manager }, result_{}, args_{ CORE_FWD(args)... }
  { }

public:
  using Self = SubsystemBuilder;

  template<SubsystemConcept S>
  auto before() -> Self& {
    result_.before.push_back(core::type_sid<S>());
    return *this;
  }

  template<SubsystemConcept S>
  auto after() -> Self& {
    result_.after.push_back(core::type_sid<S>());
    return *this;
  }

  ~SubsystemBuilder() {
    manager_->add_subsystem<T>(std::move(result_), std::forward<Args>(args_)...);
  }

private:
  SubsystemManager* manager_;
  SubsystemBuildResult result_;
  std::tuple<Args...> args_;
};
