/* core_test.c - Host-side tests for the engine core.
 *
 * Build and run from the repository root (compile every .c file in core/):
 *   gcc -std=c99 -Wall -Wextra -I. tests/core_test.c core/engine.c core/log.c \
 *       core/objects.c core/render.c -o core_test && ./core_test
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

#include "core/engine.h"
#include "core/log.h"
#include "core/objects.h"
#include "core/render.h"

static int checks = 0;
static int failures = 0;

#define CHECK(cond)                                                      \
    do {                                                                 \
        checks++;                                                        \
        if (!(cond)) {                                                   \
            failures++;                                                  \
            printf("FAIL line %d: %s\n", __LINE__, #cond);               \
        }                                                                \
    } while (0)

#define CHECK_STR(actual, expected)                                      \
    do {                                                                 \
        const char *a_ = (actual);                                       \
        checks++;                                                        \
        if (a_ == NULL || strcmp(a_, (expected)) != 0) {                 \
            failures++;                                                  \
            printf("FAIL line %d: got \"%s\", expected \"%s\"\n",        \
                   __LINE__, a_ ? a_ : "(NULL)", (expected));            \
        }                                                                \
    } while (0)

/* The framebuffer sits between two guard zones so we can detect any write
 * outside the 240x160 area. */
#define GUARD 512
#define FB_PIXELS (SCREEN_WIDTH * SCREEN_HEIGHT)
static Color fb_mem[GUARD + FB_PIXELS + GUARD];
static Color *fb = fb_mem + GUARD;

static void guards_reset(void) {
    for (int i = 0; i < GUARD; i++) {
        fb_mem[i] = 0xBEEF;
        fb_mem[GUARD + FB_PIXELS + i] = 0xBEEF;
    }
    for (int i = 0; i < FB_PIXELS; i++) fb[i] = 0;
}

static int guards_ok(void) {
    for (int i = 0; i < GUARD; i++) {
        if (fb_mem[i] != 0xBEEF || fb_mem[GUARD + FB_PIXELS + i] != 0xBEEF) return 0;
    }
    return 1;
}

static Color px(int x, int y) { return fb[y * SCREEN_WIDTH + x]; }

static void test_objects(void) {
    static Engine e;
    engine_init(&e);
    CHECK(objects_count(&e) == 0);

    Object *a = object_create(&e, "Empty_1", OBJ_TYPE_EMPTY, 120, 80, 1);
    Object *b = object_create(&e, "Player", OBJ_TYPE_SPRITE, 10, 20, 2);
    CHECK(a != NULL && b != NULL);
    CHECK(a->id == 1 && b->id == 2);
    CHECK(objects_count(&e) == 2);
    CHECK_STR(log_get(&e.log, 0), "Object created: id=1 name=Empty_1 pos=(120,80)");
    CHECK_STR(log_get(&e.log, 1), "Object created: id=2 name=Player pos=(10,20)");

    CHECK(object_find(&e, 2) == b);
    CHECK(object_find(&e, 99) == NULL);

    /* layer clamping and NULL / long names */
    Object *c = object_create(&e, "this-name-is-way-too-long-for-the-slot", OBJ_TYPE_EMPTY, 0, 0, 99);
    CHECK(c != NULL && c->layer == LAYER_COUNT - 1);
    CHECK(strlen(c->name) == MAX_NAME_LEN - 1);
    Object *d = object_create(&e, NULL, OBJ_TYPE_EMPTY, 0, 0, -5);
    CHECK(d != NULL && d->layer == 0 && d->name[0] == '\0');

    /* destroy and slot reuse */
    CHECK(object_destroy(&e, 1) == 1);
    CHECK(object_find(&e, 1) == NULL);
    CHECK(object_destroy(&e, 1) == 0);
    Object *f = object_create(&e, "Reused", OBJ_TYPE_EMPTY, 5, 5, 0);
    CHECK(f == a);          /* first free slot is reused */
    CHECK(f->id == 5);      /* ids are never reused */

    /* fill the table, then overflow it */
    engine_init(&e);
    for (int i = 0; i < MAX_OBJECTS; i++) {
        CHECK(object_create(&e, "x", OBJ_TYPE_EMPTY, i, 0, 0) != NULL);
    }
    CHECK(objects_count(&e) == MAX_OBJECTS);
    CHECK(object_create(&e, "overflow", OBJ_TYPE_EMPTY, 0, 0, 0) == NULL);
    CHECK_STR(log_get(&e.log, log_count(&e.log) - 1), "ERROR: object table full (max 256)");
}

static void test_log(void) {
    static Log l;
    char big[300];

    log_init(&l);
    CHECK(log_count(&l) == 0);
    CHECK(log_get(&l, 0) == NULL);

    log_addf(&l, "plain");
    CHECK_STR(log_get(&l, 0), "plain");
    log_addf(&l, "n=%d s=%s pct=%%", -42, "ok");
    CHECK_STR(log_get(&l, 1), "n=-42 s=ok pct=%");
    log_addf(&l, "%d %d %d", 0, INT_MAX, INT_MIN);
    CHECK_STR(log_get(&l, 2), "0 2147483647 -2147483648");
    log_addf(&l, "null=%s", (const char *)NULL);
    CHECK_STR(log_get(&l, 3), "null=(null)");
    log_addf(&l, "unknown %x and trailing %");
    CHECK_STR(log_get(&l, 4), "unknown %x and trailing %");

    /* truncation at LOG_LINE_LEN - 1 characters */
    memset(big, 'A', sizeof big - 1);
    big[sizeof big - 1] = '\0';
    log_addf(&l, "%s", big);
    CHECK(strlen(log_get(&l, 5)) == LOG_LINE_LEN - 1);

    /* ring buffer keeps only the last LOG_LINES lines, oldest first */
    log_init(&l);
    for (int i = 0; i < 200; i++) log_addf(&l, "line %d", i);
    CHECK(log_count(&l) == LOG_LINES);
    CHECK_STR(log_get(&l, 0), "line 136");
    CHECK_STR(log_get(&l, LOG_LINES - 1), "line 199");
    CHECK(log_get(&l, LOG_LINES) == NULL);
    CHECK(log_get(&l, -1) == NULL);
}

static void test_render_primitives(void) {
    guards_reset();
    render_clear(fb, 0x1234);
    CHECK(px(0, 0) == 0x1234 && px(239, 159) == 0x1234);

    guards_reset();
    render_pixel(fb, 3, 4, 7);
    CHECK(px(3, 4) == 7 && px(4, 3) == 0);
    render_pixel(fb, -1, 0, 9);
    render_pixel(fb, 0, -1, 9);
    render_pixel(fb, SCREEN_WIDTH, 0, 9);
    render_pixel(fb, 0, SCREEN_HEIGHT, 9);
    render_pixel(fb, INT_MAX, INT_MAX, 9);
    render_pixel(fb, INT_MIN, INT_MIN, 9);
    CHECK(guards_ok());

    guards_reset();
    render_hline(fb, 10, 5, 4, 1);
    CHECK(px(9, 5) == 0 && px(10, 5) == 1 && px(13, 5) == 1 && px(14, 5) == 0);
    render_hline(fb, -3, 6, 5, 2);               /* cut on the left */
    CHECK(px(0, 6) == 2 && px(1, 6) == 2 && px(2, 6) == 0);
    render_hline(fb, 238, 7, 10, 3);             /* cut on the right */
    CHECK(px(238, 7) == 3 && px(239, 7) == 3);
    render_hline(fb, 0, -1, 10, 4);
    render_hline(fb, 0, SCREEN_HEIGHT, 10, 4);
    render_hline(fb, 0, 0, 0, 4);
    render_hline(fb, 0, 0, -5, 4);
    render_hline(fb, INT_MIN, 0, INT_MAX, 4);
    render_hline(fb, INT_MAX, 0, INT_MAX, 4);
    CHECK(guards_ok());

    guards_reset();
    render_vline(fb, 5, 10, 4, 1);
    CHECK(px(5, 9) == 0 && px(5, 10) == 1 && px(5, 13) == 1 && px(5, 14) == 0);
    render_vline(fb, 6, -2, 4, 2);
    CHECK(px(6, 0) == 2 && px(6, 1) == 2 && px(6, 2) == 0);
    render_vline(fb, 7, 158, 10, 3);
    CHECK(px(7, 158) == 3 && px(7, 159) == 3);
    render_vline(fb, -1, 0, 10, 4);
    render_vline(fb, SCREEN_WIDTH, 0, 10, 4);
    render_vline(fb, 0, INT_MIN, INT_MAX, 4);
    CHECK(guards_ok());

    guards_reset();
    render_fill_rect(fb, 20, 30, 3, 2, 5);
    CHECK(px(20, 30) == 5 && px(22, 31) == 5 && px(23, 30) == 0 && px(20, 32) == 0);
    render_fill_rect(fb, -5, -5, 10, 10, 6);     /* corner */
    CHECK(px(0, 0) == 6 && px(4, 4) == 6 && px(5, 5) == 0);
    render_fill_rect(fb, 235, 155, 20, 20, 7);   /* opposite corner */
    CHECK(px(239, 159) == 7 && px(235, 155) == 7 && px(234, 155) == 0);
    render_fill_rect(fb, -1000, -1000, 5000, 5000, 8);
    CHECK(px(0, 0) == 8 && px(239, 159) == 8);
    render_fill_rect(fb, 0, 0, 0, 10, 9);
    render_fill_rect(fb, 0, 0, 10, -1, 9);
    render_fill_rect(fb, INT_MIN, INT_MIN, INT_MAX, INT_MAX, 9);
    CHECK(guards_ok());
}

static void test_render_scene(void) {
    static Engine e;
    engine_init(&e);
    guards_reset();

    object_create(&e, "Empty", OBJ_TYPE_EMPTY, 100, 80, 1);
    object_create(&e, "Back", OBJ_TYPE_SPRITE, 50, 50, 0);
    object_create(&e, "Front", OBJ_TYPE_SPRITE, 55, 50, 2);
    object_create(&e, "Offscreen", OBJ_TYPE_SPRITE, -500, 900, 1);
    object_create(&e, "Edge", OBJ_TYPE_EMPTY, 239, 159, 3);
    render_scene(&e, fb);

    CHECK(px(0, 0) == COLOR_BACKGROUND);
    CHECK(px(100, 80) == COLOR_WHITE);                  /* cross center */
    CHECK(px(92, 80) == COLOR_WHITE && px(108, 80) == COLOR_WHITE);
    CHECK(px(100, 72) == COLOR_WHITE && px(100, 88) == COLOR_WHITE);
    CHECK(px(91, 80) == COLOR_BACKGROUND && px(100, 71) == COLOR_BACKGROUND);
    CHECK(px(50, 50) == COLOR_SPRITE);                  /* sprites */
    CHECK(px(42, 42) == COLOR_SPRITE && px(41, 42) == COLOR_BACKGROUND);
    CHECK(px(239, 159) == COLOR_WHITE);                 /* clipped cross */
    CHECK(guards_ok());

    /* layer order: the sprite on layer 2 must cover the one on layer 0 */
    guards_reset();
    engine_init(&e);
    object_create(&e, "Front", OBJ_TYPE_EMPTY, 10, 10, 3);   /* created first... */
    object_create(&e, "Sprite", OBJ_TYPE_SPRITE, 10, 10, 0); /* ...but drawn first */
    render_scene(&e, fb);
    CHECK(px(10, 10) == COLOR_WHITE);                   /* cross on top of the square */
    CHECK(px(5, 5) == COLOR_SPRITE);
}

int main(void) {
    test_objects();
    test_log();
    test_render_primitives();
    test_render_scene();
    printf("%d checks, %d failures\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
