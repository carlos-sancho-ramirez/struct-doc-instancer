#include <stddef.h>
#include "parser.h"
#include "formatter.h"

static int isCypherChar(char ch) {
    return ch >= '0' && ch <= '9';
}

static int isNameChar(char ch) {
    return ch >= 'A' && ch <= 'Z' || ch >= 'a' && ch <= 'z' || ch >= '0' && ch <= '9' || ch == '_';
}

static int isSpaceChar(char ch) {
    return ch == ' ' || ch == '\t' || ch == '\r';
}

int parseChar(char ch, struct ParserState *state) {
    if (ch == '0' && state->state == PARSE_STATE_PARSING_COUNT && state->count == 0) {
        // For now, numbers starting with 0 are forbidden to allow future extensions for octal, hex, or any other kind
        return 1;
    }
    else if (isCypherChar(ch) && state->state == PARSE_STATE_PARSING_COUNT) {
        state->count = state->count * 10 + ch - '0';
        ++state->column;
    }
    else if (isNameChar(ch)) {
        if (state->state == PARSE_STATE_PARSING_TYPE) {
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

        ++state->column;
    }
    else if (ch == '\n') {
        if ((state->state == PARSE_STATE_PARSING_NAME || state->state == PARSE_STATE_LINE_FINISHED) && (state->nameBufferIndex >= 2 || state->nameBuffer[0] != '_')) {
            state->typeBuffer[state->typeBufferIndex] = '\0';
            state->nameBuffer[state->nameBufferIndex] = '\0';

            struct StructEntry entry;
            entry.type = state->typeBuffer;
            entry.name = state->nameBuffer;
            entry.count = state->count;
            if (format(&entry)) {
                return 1;
            }
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
    else if (state->state != PARSE_STATE_LINE_FINISHED) {
        return 1;
    }

    return 0;
}
