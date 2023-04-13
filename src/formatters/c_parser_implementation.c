#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "c_parser_implementation.h"
#include "../basic_types.h"

struct StructEntryLinkedEntry {
    int typeId;
    char name[128];
    unsigned int count;
    struct StructEntryLinkedEntry *next;
};

static struct StructEntryLinkedEntry *firstEntry = NULL;
static struct StructEntryLinkedEntry *lastEntry = NULL;

int startParserImplementation(const char *name) {
    printf("int parse%s(FILE *file, struct %s *data) {\n", name, name);
}

int numberOfBytes(int type) {
    if (type == BASIC_TYPE_ID_BYTE) {
        return 1;
    }
    else if (type == BASIC_TYPE_ID_WORD16_LE || type == BASIC_TYPE_ID_WORD16_BE) {
        return 2;
    }
    else if (type == BASIC_TYPE_ID_WORD32_LE || type == BASIC_TYPE_ID_WORD32_BE) {
        return 4;
    }
    else if (type == BASIC_TYPE_ID_WORD64_LE || type == BASIC_TYPE_ID_WORD64_BE) {
        return 8;
    }

    return 0;
}

static const char *typeNamePascalCase(int type) {
    if (type == BASIC_TYPE_ID_WORD16_LE) {
        return "Word16LittleEndian";
    }
    else if (type == BASIC_TYPE_ID_WORD16_BE) {
        return "Word16BigEndian";
    }
    else if (type == BASIC_TYPE_ID_WORD32_LE) {
        return "Word32LittleEndian";
    }
    else if (type == BASIC_TYPE_ID_WORD32_BE) {
        return "Word32BigEndian";
    }
    else if (type == BASIC_TYPE_ID_WORD64_LE) {
        return "Word64LittleEndian";
    }
    else if (type == BASIC_TYPE_ID_WORD64_BE) {
        return "Word64BigEndian";
    }

    return "";
}

int formatParserImplementation(const struct StructEntry *entry) {
    if (entry->typeId == BASIC_TYPE_ID_VOID) {
        return 0;
    }

    struct StructEntryLinkedEntry *newEntry = malloc(sizeof(struct StructEntryLinkedEntry));
    if (!newEntry) {
        fprintf(stderr, "Unable to allocate memory\n");
        return 1;
    }

    if (!firstEntry) {
        firstEntry = newEntry;
    }
    else {
        lastEntry->next = newEntry;
    }

    lastEntry = newEntry;
    newEntry->typeId = entry->typeId;
    if (entry->name) {
        strcpy(newEntry->name, entry->name);
    }
    else {
        newEntry->name[0] = '\0';
    }
    
    newEntry->count = entry->count;
    newEntry->next = NULL;

    return 0;
}

int finishParserImplementation() {
    if (firstEntry) {
        int structSize = 0;
        for (const struct StructEntryLinkedEntry *entry = firstEntry; entry; entry = entry->next) {
            structSize += numberOfBytes(entry->typeId);
        }

        printf("    char buffer[%d];\n", structSize);
        printf("    if (fread(buffer, 1, %d, file) < %d) {\n", structSize, structSize);
        printf("        fprintf(stderr, \"Unexpected end of file\\n\");\n");
        printf("        return 1;\n");
        printf("    }\n\n");

        structSize = 0;
        for (const struct StructEntryLinkedEntry *entry = firstEntry; entry; entry = entry->next) {
            if (entry->name[0]) {
                if (entry->typeId == BASIC_TYPE_ID_BYTE) {
                    printf("    data->%s = buffer[%d];\n", entry->name, structSize);
                }
                else {
                    printf("    data->%s = read%s(buffer + %d);\n", entry->name, typeNamePascalCase(entry->typeId), structSize);
                }
            }
            structSize += numberOfBytes(entry->typeId);
        }

        while (firstEntry) {
            void *entry = firstEntry;
            firstEntry = firstEntry->next;
            free(entry);
        }
        lastEntry = NULL;

        printf("\n");
    }

    printf("    return 0;\n}\n");
}
