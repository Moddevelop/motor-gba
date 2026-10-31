/* engine.c - Engine setup. */
#include "engine.h"
#include "log.h"
#include "objects.h"

void engine_init(Engine *engine) {
    log_init(&engine->log);
    objects_init(engine);
}
