
# Entity-Component System
An Entity-Component System is a software architecture which organizes objects into IDs (called **entities**) and their related data (called **components**). This library implements a simple ECS as a header-only library for use in your own projects.


## Getting Started
To use the library, add `ecs.hpp` to your project and declare an instance of `ecs::ECS`.

``` c++
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
            Position& entity_position = ecs.get_component<Position>();
            Velocity& entity_velocity = ecs.get_component<Velocity>();

            entity_position.x += entity_velocity.x;
            entity_position.y += entity_velocity.y;
        }
    }
}
```
See the `example` folder for an example of using the ECS in a full project. The example project uses SDL2 and requires SDL2 to compile and run.

## Why use an ECS?

ECSs are used commonly in game development because they solve the following two problems present in Object-Oriented Programming (OOP).

The first of these problems has to do with the **inflexibility of inheritance**. As an example, consider the scenario where you have two base classes, `NPC` and `Enemy`. During development, you realize that you need to create a special `Pirate` NPC that will behave like an `NPC` until he is angered, at which point he will attack the player and behave like an `Enemy`. This creates a situation where you would like `Pirate` extend from two different base classes, but there's no easy way to solve this without confusing the inheritence tree that all the objects in the game are relying on.

An ECS solves this issue by using **composition**, rather than inheritence, to handle shared behavior between objects. When an entity is created in an ECS, it has no built-in properties or behavior like it would in OOP. Instead, data is added to the entity by attaching components (You create a `Pirate`, and then you give him a **position** component and a **velocity** component). Behavior is handled by way of **systems**, which are pieces of code that act on all entities which have certain components.

For example, suppose you have a physics system that updates the position of all entities based on their velocity. The physics system would run for all entities which have both a **position** and a **velocity**, and it would skip any entity which didn't have those components. This way you don't need to worry about inheritence, you just need to be sure that any entity which should use the physics code has both a **position** and **velocity** attached to it.

The second concern with OOP has to do with **performance**. In game programming it is common to iterate over a list of objects and perform some code on each one. Such iteration pulls every object that it iterates over entirely from memory, however most of the time the code only needs a select few fields from each object. For example, when handling the movement of all the `Enemy` objects, you only need the **position** and **velocity** variable attached to each `Enemy`, but to access those variables you need to pull the entire `Enemy` into memory. If the number of enemies is large enough and the `Enemy` has a large number of member variables, this can lead to performance issues where the required `Enemy` is not stored in the cache and must be retrieved from memory.

ECSs address this issue by storing components not as class member variables but instead in tightly-packed arrays by component type. The ECS keeps track of which components are assigned to which entities. So in our example about `Enemy` movement, the physics system would just need to loop through the **position** and **velocity** arrays, which is much more efficient than having to access all the other components attached to each `Enemy`.

## API Documentation

- **ECS()**

    Default constructor. Creates a new instance of the Entity-Component System.

- **ecs::Entity create_entity()**

    Creates a new entity and returns the ID. `ecs::Entity` is an alias for `std::uint32_t`.

- **void remove_entity(ecs::Entity entity_to_remove)**

    Removes the entity with the given ID along with all associated components.

- **void register_component\<T>()**

    Creates a new component array of the type T. A data type can only be registered as a component once (including any of its aliases).
    ``` c++
    using num = int;

    ecs::ECS my_ecs;
    my_ecs.register_component<char>(); // success!
    my_ecs.register_component<int>(); // success!
    my_ecs.register_component<num>(); // error! You cannot add num because you have
                                      // already added int, which is the same type!
    ```

- **void add_component\<T>(ecs::Entity entity, T component)**

    Adds a component of type T to the entity.

- **void remove_component\<T>(ecs::Entity entity)**

    Removes a component of type T from the entity.

- **T& get_component\<T>(ecs::Entity entity)**

    Returns a reference to the component of type T that is attached to the entity. Since it is a reference, the returned component data is mutable.
    ``` c++
    // Register a component of type int in the ECS
    my_ecs.register_component<int>();

    // Create a new entity and assign it an int component with the value 5
    ecs::Entity my_entity = my_ecs.create_entity();
    my_ecs.add_component<int>(my_entity, 5);

    // Access the entity's int component and increment it by 1
    my_ecs.get_component<int>(my_entity) += 1;

    // Output: value is 6
    std::cout << "value is: " << my_ecs.get_component<int>() << std::endl;
    ```

- **ecs::View view<T, ...typenames>()**

    Returns an `ecs::View` containing all entities which have the components listed in template types list. `ecs::View` is an alias for `std::set<ecs::Entity>`.
    
    Views are how this ECS implementation handles systems. To create a system, just create a view and run your system's code over each of its members.
    ``` c++
    // Create a view containing all entities which have both a position and a velocity
    ecs::View my_view = my_ecs.view<Position, Velocity>();

    // Iterate over all the entities in the view
    for(ecs::Entity entity : my_view) {
        Position& entity_position = my_ecs.get_component<Position>(entity);
        Position& entity_velocity = my_ecs.get_component<Velocity>(entity);

        entity_position += entity_velocity;
    }
    ```
