#pragma once

#include <SDL2/SDL.h>
#include "ecs.hpp"
#include "vector.hpp"

class Breakout {
    typedef enum State {
        READY,
        PLAYING,
        FAIL,
        SUCCESS
    } State;
    typedef enum PlayerInput {
        INPUT_LEFT,
        INPUT_RIGHT
    } PlayerInput;
    public:
        Breakout();
        void handle_input(SDL_Event e);
        void update();
        void render();
    private:
        ecs::ECS ecs;

        State state;
        bool player_input_held[2];

        ecs::Entity player;
        ecs::Entity ball;

        void set_state(State new_state);

        void player_create();
        void player_reset_position();

        void ball_create();
        void ball_reset_position();

        void create_bricks();
};
