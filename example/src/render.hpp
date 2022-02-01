#pragma once

#include "vector.hpp"
#include <SDL2/SDL.h>
#include <string>

// Externs defined in main.cpp
extern const int SCREEN_WIDTH;
extern const int SCREEN_HEIGHT;
extern SDL_Renderer* renderer;

// Externs defined in render.cpp
extern const int RENDER_POSITION_CENTERED;
extern const SDL_Color COLOR_WHITE;
extern const SDL_Color COLOR_BLACK;
extern const SDL_Color COLOR_YELLOW;

typedef enum Font {
    FONT_HACK,
    FONT_COUNT
} Font;

typedef struct Image {
    SDL_Texture* texture;
    vec2 size;
    vec2 frame_size;
} Image;

// Resource initialization
bool render_load_resources();
void render_free_resources();
void render_load_font(Font font, std::string path, int size);
int render_load_image(std::string path);
int render_load_spritesheet(std::string path, vec2 frame_size);
std::string render_get_path(int image_index);
vec2 render_get_frame_size(int image_index);

// Render functions
void render_clear();
void render_present();
Image* render_create_text_image(const char* text, Font font, SDL_Color color);
void render_text(const char* text, Font font, SDL_Color color, vec2 position);
void render_text_centered(const char* text, Font font, SDL_Color color, SDL_Rect rect);
void render_image(int image_index, vec2 position);
void render_image_frame(int image_index, vec2 frame, vec2 position, bool flipped);
