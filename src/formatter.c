#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "formatter.h"
#include "basic_types.h"

int startStruct(const char *name) {
    printf("struct %s {\n", name);
}

int finishStruct() {
    printf("};\n");
}

int format(const struct StructEntry *entry) {
    const int typeId = entry->typeId;
    if (entry->name != NULL && typeId != BASIC_TYPE_ID_VOID) {
        const char *outType = NULL;
        if (typeId == BASIC_TYPE_ID_BYTE) {
            outType = "char";
        }
        else if (typeId == BASIC_TYPE_ID_WORD16_LE || typeId == BASIC_TYPE_ID_WORD16_BE) {
            outType = "short";
        }
        else if (typeId == BASIC_TYPE_ID_WORD32_LE || typeId == BASIC_TYPE_ID_WORD32_BE) {
            outType = "int";
        }
        else if (typeId == BASIC_TYPE_ID_WORD64_LE || typeId == BASIC_TYPE_ID_WORD64_BE) {
            outType = "long";
        }

        if (outType == NULL) {
            fprintf(stderr, "Undefined type identifier %u\n", entry->typeId);
            return 1;
        }

        if (entry->count >= 2) {
            printf("    %s[%u] %s;\n", outType, entry->count, entry->name);
        }
        else {
            printf("    %s %s;\n", outType, entry->name);
        }
    }

    return 0;
}