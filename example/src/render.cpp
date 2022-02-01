#include "render.hpp"
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <vector>

const int RENDER_POSITION_CENTERED = -1;
const SDL_Color COLOR_WHITE = (SDL_Color) { .r = 255, .g = 255, .b = 255, .a = 255 };
const SDL_Color COLOR_BLACK = (SDL_Color) { .r = 0, .g = 0, .b = 0, .a = 0 };
const SDL_Color COLOR_YELLOW = (SDL_Color) { .r = 255, .g = 255, .b = 0, .a = 255 };

// Resources
TTF_Font** fonts;
std::vector<Image> images;
std::vector<std::string> image_paths;

// Resource management functions

bool render_load_resources() {
    fonts = new TTF_Font*[FONT_COUNT];
    render_load_font(FONT_HACK, "./res/hack.ttf", 10);

    return true;
}

void render_free_resources() {
    for(int i = 0; i < FONT_COUNT; i++) {
        TTF_CloseFont(fonts[i]);
    }
    delete [] fonts;

    for(int i = 0; i < images.size(); i++) {
        SDL_DestroyTexture(images[i].texture);
    }
}

void render_load_font(Font font, std::string path, int size) {
    fonts[font] = TTF_OpenFont(path.c_str(), size);
    if(!fonts[font]) {
        std::cout << "Unable to open font! SDL Error: " << TTF_GetError() << std::endl;
        return;
    }
}

int render_load_image(std::string path) {
    for(int i = 0; i < images.size(); i++) {
        bool image_already_loaded = path == image_paths[i];
        if(image_already_loaded) {
            return i;
        }
    }

    SDL_Surface* loaded_surface = IMG_Load(path.c_str());
    if(loaded_surface == nullptr) {
        std::cout << "Unable to load image " << path << "! SDL Error: " << IMG_GetError() << std::endl;
        return -1;
    }

    Image new_image;
    new_image.texture = SDL_CreateTextureFromSurface(renderer, loaded_surface);
    if(new_image.texture == nullptr) {
        std::cout << "Unable to create image texture! SDL Error: " << SDL_GetError() << std::endl;
        return -1;
    }
    new_image.size = (vec2) {  .x = loaded_surface->w, .y = loaded_surface->h };
    new_image.frame_size = (vec2) { .x = new_image.size.x, .y = new_image.size.y };

    images.push_back(new_image);
    image_paths.push_back(path);

    SDL_FreeSurface(loaded_surface);

    return images.size() - 1;
}

int render_load_spritesheet(std::string path, vec2 frame_size) {
    int image_index = render_load_image(path);
    images[image_index].frame_size = frame_size;

    return image_index;
}

std::string render_get_path(int image_index) {
    return image_paths[image_index];
}

vec2 render_get_frame_size(int image_index) {
    return images[image_index].frame_size;
}

// Rendering functions

void render_clear() {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);
}

void render_present() {
    SDL_RenderPresent(renderer);
}

Image* render_create_text_image(const char* text, Font font, SDL_Color color) {
    SDL_Surface* text_surface = TTF_RenderText_Solid(fonts[font], text, color);
    if(text_surface == nullptr) {
        std::cout << "Unable to render text to surface! SDL Error: " << TTF_GetError() << std::endl;
        return nullptr;
    }

    SDL_Texture* text_texture = SDL_CreateTextureFromSurface(renderer, text_surface);
    if(text_texture == nullptr) {
        std::cout << "Unable to create text texture! SDL Error: " << SDL_GetError() << std::endl;
        return nullptr;
    }

    Image* text_image = new Image();
    text_image->texture = text_texture;
    text_image->size = (vec2){ .x = text_surface->w, .y = text_surface->h };
    text_image->frame_size = (vec2) { .x = 0, .y = 0 };

    SDL_FreeSurface(text_surface);

    return text_image;
}

void render_text(const char* text, Font font, SDL_Color color, vec2 position) {
    Image* text_image = render_create_text_image(text, font, color);

    SDL_Rect source_rect = (SDL_Rect){ .x = 0, .y = 0, .w = text_image->size.x, .h = text_image->size.y };
    SDL_Rect dest_rect = (SDL_Rect){ .x = position.x, .y = position.y, .w = text_image->size.x, .h = text_image->size.y };
    if(dest_rect.x == RENDER_POSITION_CENTERED) {
        dest_rect.x = (SCREEN_WIDTH / 2) - (source_rect.w / 2);
    }
    if(dest_rect.y == RENDER_POSITION_CENTERED) {
        dest_rect.y = (SCREEN_HEIGHT / 2) - (source_rect.h / 2);
    }

    SDL_RenderCopy(renderer, text_image->texture, &source_rect, &dest_rect);
    SDL_DestroyTexture(text_image->texture);
    delete text_image;
}

void render_text_centered(const char* text, Font font, SDL_Color color, SDL_Rect rect) {
    Image* text_image = render_create_text_image(text, font, color);

    SDL_Rect source_rect = (SDL_Rect) { .x = 0, .y = 0, .w = text_image->size.x, .h = text_image->size.y };
    SDL_Rect dst_rect = (SDL_Rect) {
        .x = rect.x + (rect.w / 2) - (text_image->size.x / 2),
        .y = rect.y + (rect.h / 2) - (text_image->size.y / 2),
        .w = text_image->size.x,
        .h = text_image->size.y };

    SDL_RenderCopy(renderer, text_image->texture, &source_rect, &dst_rect);
    SDL_DestroyTexture(text_image->texture);
    delete text_image;
}

void render_image(int image_index, vec2 position) {
    SDL_Rect dst_rect = (SDL_Rect) {
        .x = position.x,
        .y = position.y,
        .w = images[image_index].size.x,
        .h = images[image_index].size.y,
    };

    SDL_Rect screen_rect = (SDL_Rect) { .x = 0, .y = 0, .w = SCREEN_WIDTH, .h = SCREEN_HEIGHT };
    if(!rects_intersect(dst_rect, screen_rect)) {
        return;
    }

    SDL_RenderCopy(renderer, images[image_index].texture, NULL, &dst_rect);
}

void render_image_frame(int image_index, vec2 frame, vec2 position, bool flipped) {
    Image& image = images[image_index];

    SDL_Rect src_rect = (SDL_Rect) {
        .x = frame.x * image.frame_size.x,
        .y = frame.y * image.frame_size.y,
        .w = image.frame_size.x,
        .h = image.frame_size.y,
    };
    SDL_Rect dst_rect = (SDL_Rect) {
        .x = position.x,
        .y = position.y,
        .w = image.frame_size.x,
        .h = image.frame_size.y
    };

    if(src_rect.x < 0 || src_rect.x > image.size.x - image.frame_size.x
        || src_rect.y < 0 || src_rect.y > image.size.y - image.frame_size.y) {
        std::cout << "Index (" << frame.x << ", " << frame.y << ") out of bounds for image with path " << image_paths[image_index] << std::endl;
        return;
    }

    SDL_Rect screen_rect = (SDL_Rect) { .x = 0, .y = 0, .w = SCREEN_WIDTH, .h = SCREEN_HEIGHT };
    if(!rects_intersect(dst_rect, screen_rect)) {
        return;
    }

    SDL_RendererFlip render_flip = SDL_FLIP_NONE;
    if(flipped) {
        render_flip = SDL_FLIP_HORIZONTAL;
    }

    SDL_RenderCopyEx(renderer, image.texture, &src_rect, &dst_rect, 0, NULL, render_flip);
}
