#pragma once

#include <SDL2/SDL.h>
#include "ecs.hpp"
#include <vector>
#include "vector.hpp"

class State {
    public:
        State();
        void handle_input(SDL_Event e);
        void update();
        void render();
    private:
        ecs::ECS ecs;
        ecs::Entity player;

        ecs::Entity create_cube(SDL_Rect position, SDL_Color color, vec2 velocity);
};
