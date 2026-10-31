/* render.c - Software drawing. No C library calls. */
#include "render.h"

/* Cuts the span [*start, *start + *length) to [0, limit).
 * Returns 0 if nothing is left to draw. Cannot overflow. */
static int32_t clip_span(int32_t *start, int32_t *length, int32_t limit) {
    if (*length <= 0 || *start >= limit) {
        return 0;
    }
    if (*start < 0) {
        *length += *start; /* start is negative, length is positive: safe */
        *start = 0;
        if (*length <= 0) {
            return 0;
        }
    }
    if (*length > limit - *start) {
        *length = limit - *start;
    }
    return 1;
}

void render_clear(Color *fb, Color color) {
    for (int32_t i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        fb[i] = color;
    }
}

void render_pixel(Color *fb, int32_t x, int32_t y, Color color) {
    if (x < 0 || y < 0 || x >= SCREEN_WIDTH || y >= SCREEN_HEIGHT) {
        return;
    }
    fb[y * SCREEN_WIDTH + x] = color;
}

void render_hline(Color *fb, int32_t x, int32_t y, int32_t length, Color color) {
    if (y < 0 || y >= SCREEN_HEIGHT || !clip_span(&x, &length, SCREEN_WIDTH)) {
        return;
    }
    Color *row = fb + y * SCREEN_WIDTH + x;
    for (int32_t i = 0; i < length; i++) {
        row[i] = color;
    }
}

void render_vline(Color *fb, int32_t x, int32_t y, int32_t length, Color color) {
    if (x < 0 || x >= SCREEN_WIDTH || !clip_span(&y, &length, SCREEN_HEIGHT)) {
        return;
    }
    Color *p = fb + y * SCREEN_WIDTH + x;
    for (int32_t i = 0; i < length; i++) {
        *p = color;
        p += SCREEN_WIDTH;
    }
}

void render_fill_rect(Color *fb, int32_t x, int32_t y, int32_t w, int32_t h, Color color) {
    if (!clip_span(&x, &w, SCREEN_WIDTH) || !clip_span(&y, &h, SCREEN_HEIGHT)) {
        return;
    }
    for (int32_t row = 0; row < h; row++) {
        Color *p = fb + (y + row) * SCREEN_WIDTH + x;
        for (int32_t i = 0; i < w; i++) {
            p[i] = color;
        }
    }
}

void render_scene(const Engine *engine, Color *fb) {
    render_clear(fb, COLOR_BACKGROUND);
    for (int32_t layer = 0; layer < LAYER_COUNT; layer++) {
        for (int32_t i = 0; i < MAX_OBJECTS; i++) {
            const Object *o = &engine->objects[i];
            if (!o->active || o->layer != layer) {
                continue;
            }
            if (o->type == OBJ_TYPE_EMPTY) {
                render_hline(fb, o->x - 8, o->y, 17, COLOR_WHITE);
                render_vline(fb, o->x, o->y - 8, 17, COLOR_WHITE);
            } else {
                render_fill_rect(fb, o->x - 8, o->y - 8, 16, 16, COLOR_SPRITE);
            }
        }
    }
}
