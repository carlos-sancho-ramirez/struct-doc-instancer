#ifndef _PARSER_H_
#define _PARSER_H_
#include "type_map.h"

#define TYPE_BUFFER_SIZE 128
#define NAME_BUFFER_SIZE 128

#define PARSE_STATE_PARSING_TYPE 0
#define PARSE_STATE_PARSING_NAME 1
#define PARSE_STATE_PARSING_COUNT 2
#define PARSE_STATE_COUNT_FINISHED 3
#define PARSE_STATE_LINE_FINISHED 4
#define PARSE_STATE_PARSING_TYPE_DEFINITION 5
#define PARSE_STATE_CONSTRAINTS_REACHED 6

struct ParserState {
    char typeBuffer[TYPE_BUFFER_SIZE];
    char nameBuffer[NAME_BUFFER_SIZE];
    unsigned int typeBufferIndex;
    unsigned int nameBufferIndex;
    unsigned int count;

    unsigned int line;
    unsigned int column;
    unsigned int state;
};

int isNameChar(char ch);

int startParse(const char *structName);
int finishParse();
int parseChar(char ch, struct ParserState *state, const struct TypeMapEntry *types);

#endif // _PARSER_H_