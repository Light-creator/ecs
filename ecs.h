#pragma once

#include <array>
#include <bitset>
#include <cstddef>
#include <cstring>
#include <stdarg.h>
#include <vector>

using c_type = size_t;

#define MAX_COMPONENTS  512
#define MAX_ENTITIES    1024

struct c_pool_t {
  char* pool_;
  size_t c_sz_;

  size_t cap_ = MAX_COMPONENTS;

  c_pool_t(size_t c_sz): c_sz_(c_sz) {
    // TODO: Making like vector constructor 
    pool_ = (char*)malloc(c_sz * MAX_ENTITIES);
  }

  void* get_component(size_t e_id) {
    return pool_+e_id*c_sz_;
  }

  void add_component(size_t e_id, void* data) {
    std::memcpy((pool_+e_id*c_sz_), data, c_sz_);
  }

  ~c_pool_t() {
    free(pool_);
  }
};

class ecs_t {
private:
  size_t entity_count_ = 0;
  std::array<c_pool_t*, MAX_COMPONENTS> components_; 

  std::vector<std::bitset<MAX_COMPONENTS>> entity_mask_;
  std::vector<std::bitset<MAX_COMPONENTS>> system_mask_;

public:
  size_t get_entities_count() { return entity_count_; }

  void register_component(c_type type, size_t c_sz) {
    components_[type] = new c_pool_t(c_sz);
  }
  
  template<typename... Types>
  void register_system(size_t sys_id, Types... types) {
    system_mask_.push_back({types...});
  }

  void add_component(size_t e_id, void* data, c_type type) {
    components_[type]->add_component(e_id, data);
    entity_mask_[e_id][type] = true;
  }

  bool match_mask(size_t e_id, size_t sys_id) {
    if((entity_mask_[e_id] & system_mask_[sys_id]) == system_mask_[sys_id])
      return true;

    return false;
  }

  void* get_component(size_t e_id, c_type type) {
    return components_[type]->get_component(e_id);
  }

  size_t create_entity() {
    entity_mask_.push_back({});
    return entity_count_++;
  }

};


