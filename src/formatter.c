#include <stdio.h>
#include "formatter.h"

int format(const struct StructEntry *entry) {
    if (entry->name != NULL) {
        if (entry->count >= 2) {
            printf("%s[%u] %s\n", entry->type, entry->count, entry->name);
        }
        else {
            printf("%s %s\n", entry->type, entry->name);
        }
    }

    return 0;
}