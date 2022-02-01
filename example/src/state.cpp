#include "state.hpp"

#include "render.hpp"

typedef vec2 Velocity;
typedef struct Face {
    SDL_Rect rect;
    SDL_Color color;
} Face;

State::State() {
    ecs.register_component<Velocity>();
    ecs.register_component<Face>();

    player = create_cube((SDL_Rect) { .x = 0, .y = 0, .w = 10, .h = 10
    }, (SDL_Color) { .r = 255, .g = 0, .b = 0 }, (vec2) { .x = 0, .y = 0 });
    create_cube((SDL_Rect) { .x = 300, .y = 100, .w = 15, .h = 10
    }, (SDL_Color) { .r = 0, .g = 255, .b = 0 }, (vec2) { .x = -2, .y = 0 });

    // This entity does not have a velocity, so it will not be called in the movement code
    // But it does have a face, so it will be called in the render code
    ecs::Entity stationary_rect = ecs.create_entity();
    ecs.add_component<Face>(stationary_rect, (Face) {
        .rect = (SDL_Rect) { .x = 200, .y = 200, .w = 50, .h = 10 },
        .color = (SDL_Color) { .r = 255, .g = 255, .b = 255 }
    });
}

void State::handle_input(SDL_Event e) {
    if(e.type == SDL_KEYDOWN) {
        SDL_Keycode keycode = e.key.keysym.sym;
        switch(keycode) {
            case SDLK_UP:
                ecs.get_component<Velocity>(player).y = -2;
                break;
            case SDLK_RIGHT:
                ecs.get_component<Velocity>(player).x = 2;
                break;
            case SDLK_DOWN:
                ecs.get_component<Velocity>(player).y = 2;
                break;
            case SDLK_LEFT:
                ecs.get_component<Velocity>(player).x = -2;
                break;
        }
    } else if(e.type == SDL_KEYUP) {
        SDL_Keycode keycode = e.key.keysym.sym;
        switch(keycode) {
            case SDLK_UP:
                ecs.get_component<Velocity>(player).y = 0;
                break;
            case SDLK_RIGHT:
                ecs.get_component<Velocity>(player).x = 0;
                break;
            case SDLK_DOWN:
                ecs.get_component<Velocity>(player).y = 0;
                break;
            case SDLK_LEFT:
                ecs.get_component<Velocity>(player).x = 0;
                break;
        }
    }
}

void State::update() {
    ecs::View movement_view = ecs.view<Face, Velocity>();
    for(ecs::Entity e : movement_view) {
        Face& face = ecs.get_component<Face>(e);
        vec2 position = (vec2) { .x = face.rect.x, .y = face.rect.y };
        position += ecs.get_component<Velocity>(e);
        face.rect.x = position.x;
        face.rect.y = position.y;
    }
}

void State::render() {
    ecs::View render_view = ecs.view<Face>();
    for(ecs::Entity e : render_view) {
        Face entity_face = ecs.get_component<Face>(e);

        SDL_SetRenderDrawColor(renderer, entity_face.color.r, entity_face.color.g, entity_face.color.b, 255);
        SDL_RenderFillRect(renderer, &entity_face.rect);
    }
}

ecs::Entity State::create_cube(SDL_Rect position, SDL_Color color, vec2 velocity) {
    ecs::Entity new_entity = ecs.create_entity();
    ecs.add_component<Velocity>(new_entity, (Velocity) { .x = velocity.x, .y = velocity.y });
    ecs.add_component<Face>(new_entity, (Face) {
        .rect = position,
        .color = color
    });

    return new_entity;
}
