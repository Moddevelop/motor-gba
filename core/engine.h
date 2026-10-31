/* engine.h - Limits and shared data types for the engine core.
 *
 * The core is plain C99 and never calls the C library (no malloc, printf,
 * memcpy...), so the same code builds freestanding for the GBA and normally
 * for the desktop/Android editor.
 *
 * The core also owns no global state: the platform layer allocates an Engine
 * and a framebuffer and passes them in. That avoids needing .data/.bss
 * support in the GBA startup code.
 */
#ifndef ENGINE_H
#define ENGINE_H

#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Limits (small on purpose) ---- */
#define SCREEN_WIDTH   240
#define SCREEN_HEIGHT  160
#define MAX_OBJECTS    256
#define MAX_NAME_LEN   24   /* including the terminating '\0' */
#define LAYER_COUNT    4    /* layer 0 is drawn first (back) */
#define LOG_LINES      64   /* the log keeps only the last LOG_LINES lines */
#define LOG_LINE_LEN   80   /* including the terminating '\0' */

typedef enum {
    OBJ_TYPE_EMPTY  = 0,    /* a position only; draws just an editor gizmo */
    OBJ_TYPE_SPRITE = 1
} ObjectType;

typedef struct {
    int32_t id;                 /* unique, starts at 1 */
    char name[MAX_NAME_LEN];
    ObjectType type;
    int32_t x, y;               /* screen pixels; integers only (no floats on the GBA) */
    int32_t layer;              /* 0 = back ... LAYER_COUNT - 1 = front */
    uint8_t active;             /* 0 = free slot */
} Object;

typedef struct {
    char lines[LOG_LINES][LOG_LINE_LEN];
    uint32_t total;             /* lines ever written (the ring keeps the last ones) */
} Log;

typedef struct {
    Object objects[MAX_OBJECTS];
    int32_t next_id;
    Log log;
} Engine;                       /* about 17 KB: fits in GBA IWRAM or EWRAM */

/* Puts every field in a known state. Call once before using an Engine. */
void engine_init(Engine *engine);

#ifdef __cplusplus
}
#endif

#endif /* ENGINE_H */
