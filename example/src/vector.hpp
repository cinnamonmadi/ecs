#pragma once

#include <SDL2/SDL.h>
#include <iostream>

typedef struct vec2 {
    int x;
    int y;
    inline vec2 operator+(const vec2& other) const {
        return (vec2) { .x = this->x + other.x, .y = this->y + other.y };
    }
    inline vec2& operator+=(const vec2& other) {
        this->x += other.x;
        this->y += other.y;
        return *this;
    }
    inline vec2 operator-(const vec2& other) const {
        return *this + other.inverse();
    }
    inline vec2& operator-=(const vec2& other) {
        this->x -= other.x;
        this->y -= other.y;
        return *this;
    }
    inline vec2 operator*(const float f) const {
        return (vec2) { .x = (int)(this->x * f), .y = (int)(this->y * f) };
    }
    inline vec2 inverse() const {
        return (vec2) { .x = this->x * -1, .y = this->y * -1 };
    }
    inline float length() const {
        return sqrt((x * x) + (y * y));
    }
    inline vec2 normalized() const {
        float length = this->length();
        if(length == 0) {
            return (vec2) { .x = 0, .y = 0 };
        } else {
            return (vec2) { .x = (int)(x / length), .y = (int)(y / length) };
        }
    }
    inline bool operator==(const vec2& other) const {
        return x == other.x && y == other.y;
    }
    inline bool operator!=(const vec2& other) const {
        return !(*this == other);
    }
} vec2;

inline std::ostream& operator<<(std::ostream& os, const vec2& v) {
    os << "(" << v.x << ", " << v.y << ")";
    return os;
}

bool rects_intersect(const SDL_Rect& a, const SDL_Rect& b);
bool vec2_in_rect(const vec2& v, const SDL_Rect& r);
