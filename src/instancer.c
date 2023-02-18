#include <stdio.h>
#include <string.h>

#define ARG_IN_TEMPLATE "--in-template"

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

#define BUFFER_SIZE 4096
#define TYPE_BUFFER_SIZE 32
#define NAME_BUFFER_SIZE 32

#define PARSE_STATE_PARSING_TYPE 0
#define PARSE_STATE_PARSING_NAME 1
#define PARSE_STATE_LINE_FINISHED 2

int isNameChar(char ch) {
    return ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9' || ch == '_';
}

int isSpaceChar(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\r';
}

struct ParserState {
    char typeBuffer[TYPE_BUFFER_SIZE];
    char nameBuffer[NAME_BUFFER_SIZE];
    int typeBufferIndex;
    int nameBufferIndex;

    int line;
    int column;
    int state;
};

int parseChar(char ch, struct ParserState *state) {
    if (isNameChar(ch)) {
        if (state->state == PARSE_STATE_PARSING_TYPE) {
            state->typeBuffer[state->typeBufferIndex++] = ch;
        }
        else if (state->state == PARSE_STATE_PARSING_NAME) {
            state->nameBuffer[state->nameBufferIndex++] = ch;
        }

        ++state->column;
    }
    else if (isSpaceChar(ch)) {
        if (state->state == PARSE_STATE_PARSING_TYPE && state->typeBufferIndex > 0) {
            state->state = PARSE_STATE_PARSING_NAME;
        }
        else if (state->state == PARSE_STATE_PARSING_NAME && state->nameBufferIndex > 0) {
            state->state = PARSE_STATE_LINE_FINISHED;
        }

        ++state->column;
    }
    else if (ch == '\n') {
        if (state->state == PARSE_STATE_PARSING_TYPE && state->typeBufferIndex > 0 || state->state == PARSE_STATE_PARSING_NAME && state->nameBufferIndex == 0) {
            state->typeBuffer[state->typeBufferIndex] = '\0';
            // TODO: Format type
            printf("%s _\n", state->typeBuffer);
        }
        else if (state->state == PARSE_STATE_PARSING_NAME || state->state == PARSE_STATE_LINE_FINISHED) {
            state->typeBuffer[state->typeBufferIndex] = '\0';
            state->nameBuffer[state->nameBufferIndex] = '\0';
            // TODO: Format type
            printf("%s %s\n", state->typeBuffer, state->nameBuffer);
        }

        state->typeBufferIndex = 0;
        state->nameBufferIndex = 0;

        ++state->line;
        state->column = 0;
        state->state = PARSE_STATE_PARSING_TYPE;
    }
    else if (state->state != PARSE_STATE_LINE_FINISHED) {
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