#ifndef _FORMATTER_C_STRUCT_H_
#define _FORMATTER_C_STRUCT_H_

#include "../formatter.h"

int startStruct(const char *name);
int finishStruct();
int formatStruct(const struct StructEntry *entry);

#endif // _FORMATTER_C_STRUCT_H_