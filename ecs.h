#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <cstring>
#include <iostream>
#include <stdarg.h>
#include <vector>

using c_type = size_t;

#define MAX_COMPONENTS  512
#define MAX_ENTITIES    1024

/*
  TODO:
  1. Make sparse sets
  2. Make systems via lambda functions
  3. Make components pool via polymorphism      DONE
  4. Add groups for masks 
*/ 

class ic_pool_t {
public:
  virtual ~ic_pool_t() = default;
};

template<typename CType> 
class c_pool_t: public ic_pool_t {
private:
  std::vector<CType> pool_;

public:
  c_pool_t(): pool_(MAX_ENTITIES) {}

  CType* set(size_t e_id, CType data) {
    pool_[e_id] = data;
    return &pool_[e_id];
  }

  CType* get(size_t e_id) {
    return &pool_[e_id];
  }
};

class ecs_t {
private:
  size_t entity_count_ = 0;
  std::array<ic_pool_t*, MAX_COMPONENTS> components_; 

  std::vector<std::bitset<MAX_COMPONENTS>> entity_mask_;
  std::vector<std::bitset<MAX_COMPONENTS>> system_mask_;

public:
  size_t get_entities_count() { return entity_count_; }
    
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
    components_[get_c_id<CType>()] = new c_pool_t<CType>;
  }
  
  template<typename... Types>
  void register_system(size_t sys_id, Types... types) {
    system_mask_.push_back({types...});
  }
  
  template<typename CType>
  void add_component(size_t e_id, CType& data) {
    c_pool_t<CType>* c_pool = static_cast<c_pool_t<CType>*>(components_[get_c_id<CType>()]);
    c_pool->set(e_id, data);
    entity_mask_[e_id][get_c_id<CType>()] = true;
  }

  bool match_mask(size_t e_id, size_t sys_id) {
    if((entity_mask_[e_id] & system_mask_[sys_id]) == system_mask_[sys_id])
      return true;

    return false;
  }
  
  template<typename CType>
  CType* get_component(size_t e_id) {
    c_pool_t<CType>* c_pool = static_cast<c_pool_t<CType>*>(components_[get_c_id<CType>()]);
    return c_pool->get(e_id);
  }

  size_t create_entity() {
    entity_mask_.push_back({});
    return entity_count_++;
  }

};


