#include <iostream>

#include "ecs2.h"

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
  // for(int e=0; e<ecs.get_entities_count(); e++) {
  //   if(ecs.match_mask(e, MOVEMENT_SYSTEM_MASK)) {
  //     c_vel* vel = ecs.get_component<c_vel>(e);
  //     c_transform* transform = ecs.get_component<c_transform>(e);

  //     transform->x += vel->dx;
  //     transform->y += vel->dy;
  //   }
  // }
  auto& target_mask = ecs.get_sys_mask(MOVEMENT_SYSTEM_MASK);
  for(auto& [mask, group]: ecs.get_mask_groups()) {
    if((mask & target_mask) == target_mask) {
      for(size_t e: group) {
        c_vel* vel = ecs.get_component<c_vel>(e);
        c_transform* transform = ecs.get_component<c_transform>(e);

        transform->x += vel->dx;
        transform->y += vel->dy;     
      }
    }
  }

}

auto movement_sys = ([](c_vel* vel, c_transform* transform) {
  transform->x += vel->dx;
  transform->y += vel->dy;
});

int main() {
  ecs_t ecs;

  ecs.register_system(MOVEMENT_SYSTEM_MASK);

  ecs.register_component<c_vel>();
  ecs.register_component<c_transform>();
  
  size_t player = ecs.create_entity();
  c_vel vel = {
    .dx = 1,
    .dy = 1
  };

  c_transform transform = {
    .x = 0,
    .y = 0
  };

  ecs.add_component<c_vel>(player, vel);
  ecs.add_component<c_transform>(player, transform);

  // movement_system(ecs);
  
  auto v = ecs.get_view<c_vel, c_transform>();
  v.for_each([](c_vel& vel, c_transform& tr) {
    tr.x += vel.dx;
    tr.y += vel.dy;
  });

  c_transform* tr = ecs.get_component<c_transform>(player);
  std::cout <<  tr->x << " " << tr->y << "\n";

  return 0;
}
