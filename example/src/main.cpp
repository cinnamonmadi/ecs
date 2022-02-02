#include "render.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <string>
#include <vector>
#include "breakout.hpp"

// Game constants
const char* GAME_TITLE = "ECS Demo";
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 360;

// Engine variables
SDL_Window* window;
SDL_Renderer* renderer;
int resolution_width = 1280;
int resolution_height = 720;

bool engine_is_fullscreen = false;
bool engine_is_running = true;
bool engine_render_fps = false;

// Timing variables
const float FRAME_DURATION = 1.0f / 60.0f;
float last_frame_time = 0.0f;
float last_update_time = 0.0f;
float last_second_time = 0.0f;
int frames_this_second = 0;
int fps = 0;
float delta = 0.0f;
float deltas_this_second = 0.0f;
float dps = 0;

// Game loop functions
void input();
void update();
void render();

// Engine functions
bool engine_init(int argc, char** argv);
void engine_quit();
void engine_set_resolution(int width, int height);
void engine_toggle_fullscreen();
void engine_clock_tick();

Breakout breakout;

int main(int argc, char** argv) {
    if(!engine_init(argc, argv)) {
        return 0;
    }

    while(engine_is_running) {
        input();
        update();
        render();
        engine_clock_tick();
    }

    return 0;
}

// Game loop functions

void input() {
    SDL_Event e;
    while(SDL_PollEvent(&e) != 0) {
        if(e.type == SDL_QUIT) {
            engine_is_running = false;
        } else if(e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_F2) {
            engine_render_fps = !engine_render_fps;
        } else {
            breakout.handle_input(e);
        }
    }
}

void update() {
    breakout.update();
}

void render() {
    render_clear();

    breakout.render();

    if(engine_render_fps) {
        render_text(("FPS: " + std::to_string(fps)).c_str(), FONT_HACK, COLOR_YELLOW, (vec2) { .x = 0, .y =  0});
        render_text(("DPS: " + std::to_string(dps)).c_str(), FONT_HACK, COLOR_YELLOW, (vec2) { .x = 0, .y = 10});
    }
    render_present();
}

// Engine functions

bool engine_init(int argc, char** argv) {
    bool init_fullscreened = false;

    if(SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "Unable to initialize SDL! SDL Error: " << SDL_GetError() << std::endl;
        return false;
    }

    window = SDL_CreateWindow(GAME_TITLE, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    int img_flags = IMG_INIT_PNG;

    if(!(IMG_Init(img_flags) & img_flags)){
        std::cout << "Unable to initialize SDL_image! SDL Error: " << IMG_GetError() << std::endl;
        return false;
    }

    if(TTF_Init() == -1){
        std::cout << "Unable to initialize SDL_ttf! SDL Error: " << TTF_GetError() << std::endl;
        return false;
    }

    if(!window || !renderer){
        std::cout << "Unable to initialize engine!" << std::endl;
        return false;
    }

    if(!render_load_resources()) {
        return false;
    }

    engine_set_resolution(resolution_width, resolution_height);
    if(init_fullscreened) {
        engine_toggle_fullscreen();
    }

    return true;
}

void engine_quit() {
    render_free_resources();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

    IMG_Quit();
    TTF_Quit();
    SDL_Quit();
}

void engine_set_resolution(int width, int height) {
    resolution_width = width;
    resolution_height = height;
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
    SDL_SetWindowSize(window, width, height);
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}

void engine_toggle_fullscreen() {
    if (engine_is_fullscreen){
        SDL_SetWindowFullscreen(window, 0);
    } else {
        SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
    }
    engine_is_fullscreen = !engine_is_fullscreen;
}

void engine_clock_tick() {
    frames_this_second++;
    float current_time = SDL_GetTicks() / 1000.0f;

    // Record delta time
    delta = current_time - last_update_time;
    deltas_this_second += delta;
    last_update_time = current_time;

    // If one second has passed, record what the fps and dps was during that second
    if(current_time - last_second_time >= 1.0f) {
        fps = frames_this_second;
        frames_this_second = 0;
        dps = deltas_this_second;
        deltas_this_second = 0;
        last_second_time += 1.0;
    }

    // Delay if there's extra time between frames
    if(current_time - last_frame_time < FRAME_DURATION) {
        unsigned long delay_time = (unsigned long)(1000.0f * (FRAME_DURATION - (current_time - last_frame_time)));
        SDL_Delay(delay_time);
    }

    last_frame_time = SDL_GetTicks() / 1000.0f;
}

