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

#define PARSE_STATE_LINE_START 0
#define PARSE_STATE_PARSING_TYPE 1
#define PARSE_STATE_TYPE_PARSED 2
#define PARSE_STATE_PARSING_NAME 3
#define PARSE_STATE_LINE_FINISHED 4

int isNameChar(char ch) {
    return ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9' || ch == '_';
}

int isSpaceChar(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\r';
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

    char typeBuffer[TYPE_BUFFER_SIZE];
    int typeBufferIndex = 0;

    char nameBuffer[NAME_BUFFER_SIZE];
    int nameBufferIndex = 0;

    char buffer[BUFFER_SIZE];
    int bufferEnd = 0;
    int parseState = PARSE_STATE_LINE_START;
    int line = 0;
    int column = 0;
    int state = PARSE_STATE_LINE_START;

    while (1) {
        bufferEnd = fread(buffer, 1, BUFFER_SIZE, template);
        if (bufferEnd == 0) {
            break;
        }

        for (int index = 0; index < bufferEnd; index++) {
            const char ch = buffer[index];
            if (isNameChar(ch)) {
                if (state == PARSE_STATE_LINE_START || state == PARSE_STATE_PARSING_TYPE) {
                    typeBuffer[typeBufferIndex++] = ch;
                    state = PARSE_STATE_PARSING_TYPE;
                }
                else if (state == PARSE_STATE_TYPE_PARSED || state == PARSE_STATE_PARSING_NAME) {
                    nameBuffer[nameBufferIndex++] = ch;
                    state = PARSE_STATE_PARSING_NAME;
                }

                ++column;
            }
            else if (isSpaceChar(ch)) {
                if (state == PARSE_STATE_PARSING_TYPE) {
                    state = PARSE_STATE_TYPE_PARSED;
                }
                else if (state == PARSE_STATE_PARSING_NAME) {
                    state = PARSE_STATE_LINE_FINISHED;
                }

                ++column;
            }
            else if (ch == '\n') {
                if (state == PARSE_STATE_LINE_START) {
                    // Nothing to do
                }
                else if (state == PARSE_STATE_PARSING_TYPE || state == PARSE_STATE_TYPE_PARSED) {
                    typeBuffer[typeBufferIndex] = '\0';
                    // TODO: Format type
                    printf("%s _\n", typeBuffer);
                }
                else if (state == PARSE_STATE_PARSING_NAME || state == PARSE_STATE_LINE_FINISHED) {
                    typeBuffer[typeBufferIndex] = '\0';
                    nameBuffer[nameBufferIndex] = '\0';
                    // TODO: Format type
                    printf("%s %s\n", typeBuffer, nameBuffer);
                }

                typeBufferIndex = 0;
                nameBufferIndex = 0;

                ++line;
                column = 0;
                state = PARSE_STATE_LINE_START;
            }
            else if (state != PARSE_STATE_LINE_FINISHED) {
                fprintf(stderr, "Parse error at %d:%d", line, column);
                fclose(template);
                return 1;
            }
        }
    }

    fclose(template);
    return 0;
}