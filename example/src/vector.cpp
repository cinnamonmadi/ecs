#include "vector.hpp"

bool rects_intersect(const SDL_Rect& a, const SDL_Rect& b) {
    return !(a.x + a.w <= b.x ||
             b.x + b.w <= a.x ||
             a.y + a.h <= b.y ||
             b.y + b.h <= a.y);
}

bool vec2_in_rect(const vec2& v, const SDL_Rect& r) {
    return v.x >= r.x && v.x <= r.x + r.w && v.y >= r.y && v.y <= r.y + r.h;
}
