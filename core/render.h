/* render.h - Software drawing into a SCREEN_WIDTH x SCREEN_HEIGHT framebuffer.
 *
 * Pixels are 16-bit BGR555 (red in the low bits), the same layout as the GBA
 * bitmap modes. The caller owns the framebuffer:
 *   - on the GBA pass (Color *)0x06000000 (video memory, mode 3)
 *   - in the editor pass an ordinary array
 * Every drawing function clips to the screen, so out-of-range coordinates
 * are safe. Keep coordinates within roughly +/- 1,000,000.
 */
#ifndef RENDER_H
#define RENDER_H

#include "engine.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint16_t Color;

#define RGB15(r, g, b) ((Color)(((r) & 31) | (((g) & 31) << 5) | (((b) & 31) << 10)))

#define COLOR_BACKGROUND RGB15(3, 3, 6)
#define COLOR_WHITE      RGB15(31, 31, 31)
#define COLOR_SPRITE     RGB15(10, 25, 15)

void render_clear(Color *fb, Color color);
void render_pixel(Color *fb, int32_t x, int32_t y, Color color);
void render_hline(Color *fb, int32_t x, int32_t y, int32_t length, Color color);
void render_vline(Color *fb, int32_t x, int32_t y, int32_t length, Color color);
void render_fill_rect(Color *fb, int32_t x, int32_t y, int32_t w, int32_t h, Color color);

/* Clears the screen and draws every active object, layer 0 first.
 * Empty -> a white cross. Sprite -> a 16x16 square centered on the object. */
void render_scene(const Engine *engine, Color *fb);

#ifdef __cplusplus
}
#endif

#endif /* RENDER_H */
