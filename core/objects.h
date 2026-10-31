/* objects.h - The object table. Everything in a scene is an Object; an
 * "Empty" is just an object with a position and no picture. */
#ifndef OBJECTS_H
#define OBJECTS_H

#include "engine.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Called by engine_init(). Marks every slot as free. */
void objects_init(Engine *engine);

/* Creates an object in the first free slot and logs it.
 * layer is clamped to 0 .. LAYER_COUNT - 1; the name is cut to fit.
 * Returns NULL (and logs an error) if the table is full. */
Object *object_create(Engine *engine, const char *name, ObjectType type,
                      int32_t x, int32_t y, int32_t layer);

/* Frees the object with this id. Returns 1 if it existed, 0 otherwise. */
int32_t object_destroy(Engine *engine, int32_t id);

/* Returns the active object with this id, or NULL. */
Object *object_find(Engine *engine, int32_t id);

/* Number of active objects. */
int32_t objects_count(const Engine *engine);

#ifdef __cplusplus
}
#endif

#endif /* OBJECTS_H */
