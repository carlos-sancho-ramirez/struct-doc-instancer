#ifndef _TYPE_MAP_H_
#define _TYPE_MAP_H_

struct TypeMapEntry {
    const char *inName;
    const char *outName;
    struct TypeMapEntry *next;
};

#endif // _TYPE_MAP_H_