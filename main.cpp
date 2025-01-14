#include <iostream>

#include "ecs.h"

struct c_vel {
  float dx, dy;
};

struct c_transform {
  float x, y;
};

enum c_type_t {
  C_TRANSFORM,
  C_VELOCITY,

  C_COUNT
};

enum sys_maks {
  MOVEMENT_SYSTEM_MASK,

  SYSTEM_MASK_COUNT
};

void movement_system(ecs_t& ecs) {
  for(int e=0; e<ecs.get_entities_count(); e++) {
    if(ecs.match_mask(e, MOVEMENT_SYSTEM_MASK)) {
      c_vel* vel = reinterpret_cast<c_vel*>(ecs.get_component(e, C_VELOCITY));
      c_transform* transform = reinterpret_cast<c_transform*>(ecs.get_component(e, C_TRANSFORM));
    
      transform->x += vel->dx;
      transform->y += vel->dy;
    }
  }
}

int main() {
  ecs_t ecs;

  ecs.register_system(MOVEMENT_SYSTEM_MASK);

  ecs.register_component(C_VELOCITY, sizeof(c_vel));
  ecs.register_component(C_TRANSFORM, sizeof(c_transform));
  
  size_t player = ecs.create_entity();
  c_vel vel = {
    .dx = 1,
    .dy = 1
  };

  c_transform transform = {
    .x = 0,
    .y = 0
  };

  ecs.add_component(player, &vel, C_VELOCITY);
  ecs.add_component(player, &transform, C_TRANSFORM);

  movement_system(ecs);
  
  c_transform* tr = reinterpret_cast<c_transform*>(ecs.get_component(player, C_TRANSFORM));
  std::cout <<  tr->x << " " << tr->y << "\n";

  return 0;
}
