#ifndef _FORMATTER_H_
#define _FORMATTER_H_

struct StructEntry {
    char *type;
    char *name;
    unsigned int count;
};

int format(const struct StructEntry *entry);

#endif // _FORMATTER_H_