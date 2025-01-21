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

auto movement_sys = ([](c_vel& vel, c_transform& transform) {
  transform.x += vel.dx;
  transform.y += vel.dy;
});

int main() {
  ecs_t ecs;

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
  
  // auto v = ecs.get_view<c_vel, c_transform>();
  // v.for_each([](c_vel& vel, c_transform& tr) {
  //   tr.x += vel.dx;
  //   tr.y += vel.dy;
  // });

  ecs.for_each<c_vel, c_transform>(movement_sys);

  c_transform* tr = ecs.get_component<c_transform>(player);
  std::cout <<  tr->x << " " << tr->y << "\n";

  return 0;
}
