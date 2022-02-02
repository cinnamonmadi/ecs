#include "breakout.hpp"

#include "render.hpp"

typedef vec2 Velocity;
typedef struct Face {
    SDL_Rect rect;
    SDL_Color color;
} Face;

const int PLAYER_SPEED = 3;
const int BALL_SPEED = 3;

Breakout::Breakout() {
    ecs.register_component<Velocity>();
    ecs.register_component<Face>();

    player_create();
    ball_create();

    set_state(READY);
}

void Breakout::handle_input(SDL_Event e) {
    if(state != PLAYING) {
        bool pressed_space = e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_SPACE;
        if(pressed_space) {
            switch(state) {
                case READY:
                    set_state(PLAYING);
                    break;
                case FAIL:
                    set_state(READY);
                    break;
                case SUCCESS:
                    set_state(READY);
                    break;
                default:
                    break;
            }
        }
    } else {
        if(e.type == SDL_KEYDOWN) {
            SDL_Keycode keycode = e.key.keysym.sym;
            switch(keycode) {
                case SDLK_LEFT:
                    player_input_held[INPUT_LEFT] = true;
                    ecs.get_component<Velocity>(player).x = -PLAYER_SPEED;
                    break;
                case SDLK_RIGHT:
                    player_input_held[INPUT_RIGHT] = true;
                    ecs.get_component<Velocity>(player).x = PLAYER_SPEED;
                    break;
            }
        } else if(e.type == SDL_KEYUP) {
            SDL_Keycode keycode = e.key.keysym.sym;
            switch(keycode) {
                case SDLK_LEFT: {
                    player_input_held[INPUT_LEFT] = false;
                    if(player_input_held[INPUT_RIGHT]) {
                        ecs.get_component<Velocity>(player).x = PLAYER_SPEED;
                    } else {
                        ecs.get_component<Velocity>(player).x = 0;
                    }
                    break;
                }
                case SDLK_RIGHT:
                    player_input_held[INPUT_RIGHT] = false;
                    if(player_input_held[INPUT_LEFT]) {
                        ecs.get_component<Velocity>(player).x = -PLAYER_SPEED;
                    } else {
                        ecs.get_component<Velocity>(player).x = 0;
                    }
                    break;
            }
        }
    }
}

void Breakout::update() {
    if(state != PLAYING) {
        return;
    }

    ecs::View movement_view = ecs.view<Face, Velocity>();
    for(ecs::Entity e : movement_view) {
        Face& face = ecs.get_component<Face>(e);
        Velocity& velocity = ecs.get_component<Velocity>(e);

        // Increment the entity's position
        face.rect.x += velocity.x;
        face.rect.y += velocity.y;

        // Check to ensure entity stays in the screen
        bool reached_x_bounds = face.rect.x < 0 || face.rect.x + face.rect.w > SCREEN_WIDTH;
        if(reached_x_bounds && e == player) {
            face.rect.x -= velocity.x;
        } else if(reached_x_bounds && e == ball) {
            velocity.x *= -1;
        }

        if(e == ball && face.rect.y < 0) {
            velocity.y *= -1;
        } else if(e == ball && face.rect.y + face.rect.h > SCREEN_HEIGHT) {
            set_state(READY);
        }
    }

    ecs::View collision_view = ecs.view<Face>();
    SDL_Rect ball_rect = ecs.get_component<Face>(ball).rect;
    for(ecs::Entity e : collision_view) {
        if(e == ball) {
            continue;
        }

        SDL_Rect entity_rect = ecs.get_component<Face>(e).rect;
        if(rects_intersect(ball_rect, entity_rect)) {
            Velocity& ball_velocity = ecs.get_component<Velocity>(ball);
            ball_velocity.y *= -1;
            if(e == player) {
                bool ball_on_player_left_side = ball_rect.x + ball_rect.w < entity_rect.x + (entity_rect.w / 2);
                if( (ball_on_player_left_side && ball_velocity.x < 0) ||
                    (!ball_on_player_left_side && ball_velocity.x > 0)) {
                    ball_velocity.x *= -1;
                }
            } else {
                ecs.remove_entity(e);
            }
        }
    }
}

void Breakout::render() {
    ecs::View render_view = ecs.view<Face>();
    for(ecs::Entity e : render_view) {
        Face entity_face = ecs.get_component<Face>(e);

        SDL_SetRenderDrawColor(renderer, entity_face.color.r, entity_face.color.g, entity_face.color.b, 255);
        SDL_RenderFillRect(renderer, &entity_face.rect);
    }

    if(state == READY) {
        render_text("Press space to start!", FONT_HACK, COLOR_WHITE, (vec2) { .x = RENDER_POSITION_CENTERED, .y = 150 });
    } else if(state == FAIL) {
        render_text("You lost! Press space to continue.", FONT_HACK, COLOR_WHITE, (vec2) { .x = RENDER_POSITION_CENTERED, .y = 150 });
    } else if(state == SUCCESS) {
        render_text("You won! Press space to continue.", FONT_HACK, COLOR_WHITE, (vec2) { .x = RENDER_POSITION_CENTERED, .y = 150 });
    }
}

void Breakout::set_state(State new_state) {
    state = new_state;
    if(state == READY) {
        // Remoe any existing bricks
        ecs::View brick_view = ecs.view<Face>();
        for(ecs::Entity e : brick_view) {
            if(e != player && e != ball) {
                ecs.remove_entity(e);
            }
        }

        // Reset player and ball position
        player_reset_position();
        ball_reset_position();

        // Recreate the bricks
        create_bricks();
    } else if(state == PLAYING) {
        ecs.get_component<Velocity>(ball) = (vec2) { .x = BALL_SPEED, .y = BALL_SPEED };
    } else if(state == FAIL || state == SUCCESS) {
        ecs.get_component<Velocity>(ball) = (vec2) { .x = 0, .y = 0 };
    }
}

void Breakout::player_create() {
    player = ecs.create_entity();
    ecs.add_component<Face>(player, (Face) {
        .rect = (SDL_Rect) { .x = 0, .y = 0, .w = 100, .h = 10 },
        .color = (SDL_Color) { .r = 255, .g = 255, .b = 255 }
    });
    ecs.add_component<Velocity>(player, (Velocity) { .x = 0, .y = 0 });
}

void Breakout::player_reset_position() {
    Face& player_face = ecs.get_component<Face>(player);
    player_face.rect.x = (SCREEN_WIDTH / 2) - (player_face.rect.w / 2);
    player_face.rect.y = SCREEN_HEIGHT - player_face.rect.h - 5;
}

void Breakout::ball_create() {
    ball = ecs.create_entity();
    ecs.add_component<Face>(ball, (Face) {
        .rect = (SDL_Rect) { .x = 0, .y = 0, .w = 10, .h = 10 },
        .color = (SDL_Color) { .r = 255, .g = 255, .b = 255 }
    });
    ecs.add_component<Velocity>(ball, (Velocity) { .x = 0, .y = 0 });
}

void Breakout::ball_reset_position() {
    Face& ball_face = ecs.get_component<Face>(ball);
    ball_face.rect.x = (SCREEN_WIDTH / 2) - (ball_face.rect.w / 2);
    ball_face.rect.y = (SCREEN_HEIGHT / 2) - (ball_face.rect.h / 2);
}

void Breakout::create_bricks() {
    const vec2 BRICK_SIZE = (vec2) { .x = 50, .y = 10 };
    const vec2 BRICK_PADDING = (vec2) { .x = 2, .y = 2 };
    const int OFFSET = 25;
    const int NUMBER_OF_ROWS = 5;

    for(int row = 0; row < NUMBER_OF_ROWS; row++) {
        vec2 brick_position = BRICK_PADDING + (vec2) { .x = 0, .y = ((BRICK_SIZE.y + BRICK_PADDING.y) * row) };
        int row_max = SCREEN_WIDTH;
        if(row % 2 == 1) {
            brick_position.x += OFFSET;
            row_max -= OFFSET;
        }

        while(brick_position.x + BRICK_SIZE.x < row_max) {
            ecs::Entity new_brick = ecs.create_entity();
            ecs.add_component<Face>(new_brick, (Face) {
                .rect = (SDL_Rect) {
                    .x = brick_position.x,
                    .y = brick_position.y,
                    .w = BRICK_SIZE.x,
                    .h = BRICK_SIZE.y
                },
                .color = (SDL_Color) { .r = 0, .g = 255, .b = 0 }
            });

            brick_position.x += BRICK_SIZE.x + BRICK_PADDING.x;
        }
    }
}
