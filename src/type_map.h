#ifndef _TYPE_MAP_H_
#define _TYPE_MAP_H_

struct TypeMapEntry {
    const char *inName;
    const char *outName;
    struct TypeMapEntry *next;
};

const struct TypeMapEntry *findInNameInTypeMap(const struct TypeMapEntry *map, const char *name);
void freeTypeMap(struct TypeMapEntry *map);

#endif // _TYPE_MAP_H_