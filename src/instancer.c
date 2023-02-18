#include <stdio.h>
#include <string.h>
#include "parser.h"

#define ARG_IN_TEMPLATE "--in-template"

#define BUFFER_SIZE 4096

struct Arguments {
    char *fileName;
};

int fillArguments(int argc, char *argv[], struct Arguments *args) {
    args->fileName = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], ARG_IN_TEMPLATE) == 0) {
            if (++i == argc) {
                fprintf(stderr, "File name expected after " ARG_IN_TEMPLATE "\n");
                return 1;
            }

            args->fileName = argv[i];
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
        fprintf(stderr, "Unable to open file %s", args.fileName);
        return 1;
    }

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

    fclose(template);
    return 0;
}