
# Entity-Component System
An Entity-Component System (ECS) is a software architecture used most commonly in game development. This library implements a simple ECS as a header-only library for use in your own projects.

## Getting Started
To use the library, add `ecs.hpp` to your project and declare an instance of `ecs::ECS`. 

```
#include "ecs.hpp"

typedef struct Position {
    int x;
    int y;
};

typedef struct Velocity {
    int x;
    int y;
};

int main() {
    ecs::ECS ecs;

    ecs.register_component<Position>();
    ecs.register_component<Velocity>();

    ecs::Entity friend1 = ecs.create_entity();

    ecs.add_component<Position>(friend1, (Position){ .x = 0, .y = 0 });
    ecs.add_component<Velocity>(friend1, (Position){ .x = 2, .y = 0 });

    ecs::Entity friend2 = ecs.create_entity();
    ecs.add_component<Position>(friend2, (Position){ .x = 100, .y = 100 });
    ecs.add_component<Velocity>(friend2, (Position){ .x = 0, .y = 2 });

    ecs::Entity friend3 = ecs.create_entity();
    ecs.add_component<Position>(friend3, (Position){ .x = 100, .y = 10 });

    bool game_is_running = true;
    while(game_is_running) {
        // friend3 will not be a part of the view, because it doesn't have a Velocity
        ecs::View physics_system = ecs.view<Position, Velocity>();
        for(ecs::Entity entity : physics_system) {
            // get_component() returns a mutable reference
            ecs.get_component<Position>(entity) += ecs.get_component<Velocity>(entity);
        }
    }
}
```

## What is an ECS? 

Entity-Component Systems have become more common in game development because they solve two problems present in Object-Oriented Programming (OOP). The first of these problems has to do with the inflexibility of inheritance. As an example of this, consider a game which has both penguins and pirates in it. The object-oriented approach would be to create a `Penguin` class and a `Pirate` class to organize the code related to these objects, however during development it turns out that these classes use the same code for their `move()` and `render()` function, so to avoid code repeating, a base `GameObject` is created which has both the `move()` and `render()` function. Then the `Penguin` and `Pirate` class extend `GameObject`, allowing them to gain the same functionality without repeating code. 