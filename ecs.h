#pragma once

#include <algorithm>
#include <array>
#include <bitset>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <stdarg.h>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

using c_type = size_t;

#define MAX_COMPONENTS  512
#define MAX_ENTITIES    1024
#define NULLVAL         MAX_ENTITIES+1

/*
  TODO:
  1. Make sparse sets                           DONE
  2. Make systems via lambda functions          DONE
  3. Make components pool via polymorphism      DONE
  4. Add groups for masks                       DONE
  5. Add pages to sparse set
  6. Add smart_pointers
*/ 

class i_sparse_set_t {
public:
  virtual ~i_sparse_set_t() = default;
};

template<typename CType> 
class sparse_set_t: public i_sparse_set_t {
private:
  std::array<size_t, MAX_ENTITIES> sparse_;
  std::vector<CType> dense_;
  std::vector<size_t> dense_to_id_;

public:
  sparse_set_t() {
    sparse_.fill(NULLVAL);
  };

  CType* set(size_t e_id, CType data) {
    if(sparse_[e_id] != NULLVAL) {
      size_t idx = sparse_[e_id];
      dense_[idx] = data;
      return &dense_[idx];
    }

    sparse_[e_id] = dense_.size();
    dense_.push_back(data);
    dense_to_id_.push_back(e_id);
    return &dense_[sparse_[e_id]];
  }
  
  void del(size_t e_id) {
    size_t idx = sparse_[e_id];
    if(idx == NULLVAL) return;

    size_t last_idx = dense_.size()-1;
    size_t last_e_id = dense_to_id_[last_idx];
    std::swap(dense_[last_idx], dense_[idx]);
    std::swap(dense_to_id_[last_idx], dense_to_id_[idx]);
    dense_.pop_back();
    dense_to_id_.pop_back();
      
    sparse_[last_e_id] = idx;
  }

  CType* get(size_t e_id) {
    size_t idx = sparse_[e_id];
    if(idx == NULLVAL) return nullptr;
  
    return &dense_[idx];
  }

  CType& get_ref(size_t e_id) {
    size_t idx = sparse_[e_id];
    // if(idx == NULLVAL) return nullptr;
  
    return dense_[idx];
  }
};

template<typename... Ts>
struct type_list_t {
  using type_tuple = std::tuple<Ts...>;
  
  template<size_t Idx>
  using selected_type = std::tuple_element_t<Idx, type_tuple>;
};

template<typename... Cs>
class view_t {
private:
  using type_lst = type_list_t<Cs...>;

  std::array<i_sparse_set_t*, sizeof...(Cs)> components_;
  std::unordered_map<std::bitset<MAX_COMPONENTS>, std::vector<size_t>>* mask_groups_;
  std::bitset<MAX_COMPONENTS> target_mask_;
  
  template<size_t Idx>
  auto get_pool_by_idx() {
    using c_type = typename type_lst::template selected_type<Idx>;
    return static_cast<sparse_set_t<c_type>*>(components_[Idx]);
  }

  template<size_t... Is>
  auto make_c_tuple(size_t e_id, std::index_sequence<Is...>) {
    return std::make_tuple((std::ref(get_pool_by_idx<Is>()->get_ref(e_id)))...);
  }

public:

  view_t(
    std::array<i_sparse_set_t*, sizeof...(Cs)>&& cs,
    std::unordered_map<std::bitset<MAX_COMPONENTS>, std::vector<size_t>>* mg,
    std::bitset<MAX_COMPONENTS> tm
  ): components_(cs), mask_groups_(mg), target_mask_(tm) {}
  
  template<typename Fn>
  void for_each(Fn func) {
    auto idxs = std::make_index_sequence<sizeof...(Cs)>();

    for(auto& [mask, group]: *mask_groups_) {
      if((mask & target_mask_) == target_mask_) {
        for(size_t e_id: group) {
          if constexpr (std::is_invocable_v<Fn, Cs&...>) {
            std::apply(func, make_c_tuple(e_id, idxs));
          }
        }
      }
    }
  }

};

class ecs_t {
private:
  size_t entity_count_ = 0;
  std::array<i_sparse_set_t*, MAX_COMPONENTS> components_; 

  std::vector<std::bitset<MAX_COMPONENTS>> entity_mask_;
  std::vector<std::bitset<MAX_COMPONENTS>> system_mask_;

  std::unordered_map<std::bitset<MAX_COMPONENTS>, std::vector<size_t>> mask_groups_;

  template<typename CType>
  i_sparse_set_t* get_component_pool() {
    size_t idx = get_c_id<CType>();
    return components_[idx];
  }

  template<typename CType>
  void set_bit(std::bitset<MAX_COMPONENTS>& mask) {
    mask[get_c_id<CType>()] = true;
  }

  template<typename... Cs>
  std::bitset<MAX_COMPONENTS> get_c_mask() {
    std::bitset<MAX_COMPONENTS> mask;
    (set_bit<Cs>(mask), ...);
    return mask;
  }

public:
  ecs_t() {
    mask_groups_[{}] = {};
  }

  size_t get_entities_count() { return entity_count_; }
  std::bitset<MAX_COMPONENTS>& 
    get_sys_mask(size_t sys_id) { return system_mask_[sys_id]; }

  std::unordered_map<std::bitset<MAX_COMPONENTS>, std::vector<size_t>>&
    get_mask_groups() { return mask_groups_; }
    
  inline size_t get_c_id() {
    static size_t id = 0;
    return id++;
  }

  template<typename CType>
  size_t get_c_id() {
    static size_t id = get_c_id();
    return id;
  }

  template<typename CType>
  void register_component() {
    components_[get_c_id<CType>()] = new sparse_set_t<CType>;
  }
  
  template<typename... Types>
  void register_system(size_t sys_id, Types... types) {
    system_mask_.push_back({types...});
  }
  
  template<typename CType>
  void add_component(size_t e_id, CType& data) {
    sparse_set_t<CType>* c_pool = static_cast<sparse_set_t<CType>*>(components_[get_c_id<CType>()]);
    c_pool->set(e_id, data);
    
    std::bitset<MAX_COMPONENTS>& e_mask = entity_mask_[e_id];
    
    std::vector<size_t>& vec = mask_groups_[e_mask];
    vec.erase(std::remove(vec.begin(), vec.end(), e_id), vec.end());
    e_mask[get_c_id<CType>()] = true;
    
    if(mask_groups_.find(e_mask) == mask_groups_.end()) {
      mask_groups_[e_mask] = {};
    }
    mask_groups_[e_mask].push_back(e_id);
  }

  bool match_mask(size_t e_id, size_t sys_id) {
    if((entity_mask_[e_id] & system_mask_[sys_id]) == system_mask_[sys_id])
      return true;

    return false;
  }
  
  template<typename CType>
  CType* get_component(size_t e_id) {
    sparse_set_t<CType>* c_pool = static_cast<sparse_set_t<CType>*>(components_[get_c_id<CType>()]);
    return c_pool->get(e_id);
  }

  size_t create_entity() {
    entity_mask_.push_back({});
    mask_groups_[{}].push_back(entity_count_);
    return entity_count_++;
  }

  template<typename... Cs>
  view_t<Cs...> get_view() {
    std::bitset<MAX_COMPONENTS> mask = get_c_mask<Cs...>();
    return view_t<Cs...>{ {get_component_pool<Cs>()...}, &mask_groups_, mask };
  }

};
