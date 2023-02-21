#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "type_map.h"
#include "parser.h"

#define ARG_IN_TEMPLATE "--in-template"
#define ARG_IN_TYPES    "--in-types"

#define BUFFER_SIZE 4096

struct Arguments {
    char *structName;
    char *fileName;
    struct TypeMapEntry *types;
};

#define EXTRACT_TYPES_STATE_READING_KEY 0

static int extractTypes(const char *typesArg, struct TypeMapEntry **mapPointer) {
    *mapPointer = NULL;
    char ch;
    int keyStart = 0;
    int keyEnd;
    int valueStart = 0;
    int state = EXTRACT_TYPES_STATE_READING_KEY;
    int index;
    for (index = 0; (ch = typesArg[index]) != '\0'; index++) {
        if (keyStart == valueStart && ch == '=') {
            if (keyStart == index) {
                fprintf(stderr, "Error parsing argument " ARG_IN_TYPES " at position %u. Key cannot be empty\n", index);
                return 1;
            }

            keyEnd = index;
            valueStart = index + 1;
        }
        else if (keyStart != valueStart && ch == ';') {
            if (valueStart == index) {
                fprintf(stderr, "Error parsing argument " ARG_IN_TYPES " at position %u. Value cannot be empty\n", index);
                return 1;
            }

            void *newBlock = malloc(sizeof(struct TypeMapEntry) + (index - keyStart) + 1);
            struct TypeMapEntry *newEntry = newBlock;
            char *strBlock = newBlock + sizeof(struct TypeMapEntry);
            strncpy(strBlock, typesArg + keyStart, index - keyStart);
            strBlock[keyEnd - keyStart] = '\0';
            strBlock[index - keyStart] = '\0';
            newEntry->inName = strBlock;
            newEntry->outName = strBlock + (valueStart - keyStart);
            newEntry->next = *mapPointer;
            *mapPointer = newEntry;

            keyStart = index + 1;
            valueStart = index + 1;
        }
        else if (!isNameChar(ch)) {
            fprintf(stderr, "Error parsing argument " ARG_IN_TYPES " at position %u. Invalid character found\n", index);
            return 1;
        }
    }

    if (keyStart != valueStart && index > valueStart) {
        void *newBlock = malloc(sizeof(struct TypeMapEntry) + (index - keyStart) + 1);
        struct TypeMapEntry *newEntry = newBlock;
        char *strBlock = newBlock + sizeof(struct TypeMapEntry);
        strncpy(strBlock, typesArg + keyStart, index - keyStart);
        strBlock[keyEnd - keyStart] = '\0';
        strBlock[index - keyStart] = '\0';
        newEntry->inName = strBlock;
        newEntry->outName = strBlock + (valueStart - keyStart);
        newEntry->next = *mapPointer;
        *mapPointer = newEntry;
    }
    else if (index > keyStart) {
        fprintf(stderr, "Error parsing argument " ARG_IN_TYPES ". Unexpected end of argument\n");
        return 1;
    }

    return 0;
}

int fillArguments(int argc, char *argv[], struct Arguments *args) {
    args->fileName = NULL;
    args->types = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ARG_IN_TEMPLATE) == 0) {
            if (++i == argc) {
                fprintf(stderr, "File name expected after " ARG_IN_TEMPLATE "\n");
                return 1;
            }

            args->fileName = argv[i];
        }
        else if (strcmp(argv[i], ARG_IN_TYPES) == 0) {
            if (++i == argc) {
                fprintf(stderr, "Type definition expected after " ARG_IN_TYPES "\n");
                return 1;
            }

            if (extractTypes(argv[i], &args->types)) {
                return 1;
            }
        }
        else {
            fprintf(stderr, "Invalid argument %s\n", argv[i]);
            return 1;
        }
    }

    if (args->fileName == NULL) {
        fprintf(stderr, "Missing argument " ARG_IN_TEMPLATE "\n");
        return 1;
    }

    char *structName = strrchr(args->fileName, '/');
    if (structName) {
        args->structName = structName + 1;
    }
    else {
        args->structName = args->fileName;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    struct Arguments args;
    const int result = fillArguments(argc, argv, &args);
    if (result != 0) {
        return result;
    }

    FILE *template = fopen(args.fileName, "r");
    if (template == NULL) {
        fprintf(stderr, "Unable to open file %s\n", args.fileName);
        return 1;
    }

    startParse(args.structName);

    char buffer[BUFFER_SIZE];
    struct ParserState parserState;
    parserState.typeBufferIndex = 0;
    parserState.nameBufferIndex = 0;
    parserState.count = 0;

    int bufferEnd = 0;
    parserState.line = 0;
    parserState.column = 0;
    parserState.state = PARSE_STATE_PARSING_TYPE;

    while (1) {
        bufferEnd = fread(buffer, 1, BUFFER_SIZE, template);
        if (bufferEnd == 0) {
            break;
        }

        for (int index = 0; index < bufferEnd; index++) {
            if (parseChar(buffer[index], &parserState)) {
                fprintf(stderr, "Parse error at %d:%d\n", parserState.line, parserState.column);
                fclose(template);
                return 1;
            }
        }
    }

    finishParse();
    fclose(template);
    return 0;
}