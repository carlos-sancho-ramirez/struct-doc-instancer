#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include "parser.h"
#include "formatter.h"
#include "basic_types.h"

static int isCypherChar(char ch) {
    return ch >= '0' && ch <= '9';
}

int isNameChar(char ch) {
    return ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9' || ch == '_';
}

static int isSpaceChar(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\r';
}

int startParse(const char *structName) {
    startStruct(structName);
}

int finishParse() {
    finishStruct();
}

int parseChar(char ch, struct ParserState *state, const struct TypeMapEntry *types) {
    if (ch == '0' && state->state == PARSE_STATE_PARSING_COUNT && state->count == 0) {
        // For now, numbers starting with 0 are forbidden to allow future extensions for octal, hex, or any other kind
        return 1;
    }
    else if (isCypherChar(ch) && state->state == PARSE_STATE_PARSING_COUNT) {
        state->count = state->count * 10 + ch - '0';
        ++state->column;
    }
    else if (isNameChar(ch)) {
        if (state->state == PARSE_STATE_PARSING_TYPE || state->state == PARSE_STATE_PARSING_TYPE_DEFINITION) {
            state->typeBuffer[state->typeBufferIndex++] = ch;
        }
        else if (state->state == PARSE_STATE_PARSING_NAME) {
            state->nameBuffer[state->nameBufferIndex++] = ch;
        }
        else if (state->state == PARSE_STATE_PARSING_COUNT) {
            return 1;
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
        else if (state->state == PARSE_STATE_PARSING_COUNT) {
            state->state = PARSE_STATE_COUNT_FINISHED;
        }
        else if (state->state == PARSE_STATE_PARSING_TYPE_DEFINITION && state->typeBufferIndex > 0) {
            state->typeBuffer[state->typeBufferIndex] = '\0';
            if (!findInNameInTypeMap(types, state->typeBuffer)) {
                fprintf(stderr, "Missing type definition for %s\n", state->typeBuffer);
                return 1;
            }

            state->typeBufferIndex = 0;
            state->nameBufferIndex = 0;
            state->state = PARSE_STATE_LINE_FINISHED;
        }

        ++state->column;
    }
    else if (ch == '\n') {
        if ((state->state == PARSE_STATE_PARSING_NAME || state->state == PARSE_STATE_LINE_FINISHED) && (state->nameBufferIndex >= 2 || state->nameBufferIndex == 1 && state->nameBuffer[0] != '_')) {
            state->typeBuffer[state->typeBufferIndex] = '\0';
            state->nameBuffer[state->nameBufferIndex] = '\0';

            const struct TypeMapEntry *typeEntry = findInNameInTypeMap(types, state->typeBuffer);
            const char *resolvedType;
            if (typeEntry) {
                resolvedType = typeEntry->outName;
            }
            else {
                resolvedType = state->typeBuffer;
            }

            struct StructEntry entry;
            if (!strcmp(resolvedType, basicTypeVoid)) {
                entry.typeId = BASIC_TYPE_ID_VOID;
            }
            else if (!strcmp(resolvedType, basicTypeByte)) {
                entry.typeId = BASIC_TYPE_ID_BYTE;
            }
            else if (!strcmp(resolvedType, basicTypeWord16LE)) {
                entry.typeId = BASIC_TYPE_ID_WORD16_LE;
            }
            else if (!strcmp(resolvedType, basicTypeWord16BE)) {
                entry.typeId = BASIC_TYPE_ID_WORD16_BE;
            }
            else if (!strcmp(resolvedType, basicTypeWord32LE)) {
                entry.typeId = BASIC_TYPE_ID_WORD32_LE;
            }
            else if (!strcmp(resolvedType, basicTypeWord32BE)) {
                entry.typeId = BASIC_TYPE_ID_WORD32_BE;
            }
            else if (!strcmp(resolvedType, basicTypeWord64LE)) {
                entry.typeId = BASIC_TYPE_ID_WORD64_LE;
            }
            else if (!strcmp(resolvedType, basicTypeWord64BE)) {
                entry.typeId = BASIC_TYPE_ID_WORD64_BE;
            }
            else {
                fprintf(stderr, "Wrong type %s\n", state->typeBuffer);
                return 1;
            }

            entry.name = state->nameBuffer;
            entry.count = state->count;
            if (format(&entry)) {
                return 1;
            }
        }
        else if (state->state == PARSE_STATE_PARSING_TYPE_DEFINITION) {
            if (state->typeBufferIndex == 0) {
                fprintf(stderr, "Empty type definition\n");
                return 1;
            }

            state->typeBuffer[state->typeBufferIndex] = '\0';
            if (!findInNameInTypeMap(types, state->typeBuffer)) {
                fprintf(stderr, "Missing type definition for %s\n", state->typeBuffer);
                return 1;
            }

            state->state = PARSE_STATE_LINE_FINISHED;
        }
        else if (state->state == PARSE_STATE_PARSING_COUNT || state->state == PARSE_STATE_COUNT_FINISHED) {
            // ']' is expected before the end of the line
            return 1;
        }

        state->typeBufferIndex = 0;
        state->nameBufferIndex = 0;
        state->count = 0;

        ++state->line;
        state->column = 0;
        state->state = PARSE_STATE_PARSING_TYPE;
    }
    else if (ch == '[' && state->state == PARSE_STATE_PARSING_TYPE && state->typeBufferIndex > 0) {
        state->state = PARSE_STATE_PARSING_COUNT;
    }
    else if (ch == ']' && (state->state == PARSE_STATE_PARSING_COUNT || state->state == PARSE_STATE_COUNT_FINISHED)) {
        if (state->count <= 1) {
            // Count must be 2 or more, if not, why defining a count?
            return 1;
        }

        state->state = PARSE_STATE_PARSING_NAME;
    }
    else if (ch == '>' && state->state == PARSE_STATE_PARSING_TYPE && state->typeBufferIndex == 0) {
        state->state = PARSE_STATE_PARSING_TYPE_DEFINITION;
    }
    else if (state->state != PARSE_STATE_LINE_FINISHED) {
        return 1;
    }

    return 0;
}
