#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "formatter.h"

static const char *find(struct TypeMapEntry *map, const char *inName) {
    while (map != NULL) {
        if (strcmp(inName, map->inName) == 0) {
            return map->outName;
        }

        map = map->next;
    }

    return NULL;
}

int startStruct(const char *name) {
    printf("struct %s {\n", name);
}

int finishStruct() {
    printf("};\n");
}

int format(const struct StructEntry *entry) {
    struct TypeMapEntry *typeMap = malloc(sizeof(struct TypeMapEntry));
    typeMap->inName = "byte";
    typeMap->outName = "char";
    typeMap->next = NULL;

    if (entry->name != NULL) {
        const char * const outType = find(typeMap, entry->type);
        if (outType == NULL) {
            fprintf(stderr, "Undefined type %s", entry->type);
            free(typeMap);
            return 1;
        }

        if (entry->count >= 2) {
            printf("    %s[%u] %s\n", outType, entry->count, entry->name);
        }
        else {
            printf("    %s %s\n", outType, entry->name);
        }
    }

    free(typeMap);
    return 0;
}