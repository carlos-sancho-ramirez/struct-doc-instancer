#include <string.h>
#include <stdlib.h>
#include "type_map.h"

const struct TypeMapEntry *findInNameInTypeMap(const struct TypeMapEntry *map, const char *name) {
    while (map) {
        if (!strcmp(map->inName, name)) {
            return map;
        }

        map = map->next;
    }

    return NULL;
}

void freeTypeMap(struct TypeMapEntry *map) {
    while (map) {
        struct TypeMapEntry *entry = map;
        map = map->next;
        free(entry);
    }
}