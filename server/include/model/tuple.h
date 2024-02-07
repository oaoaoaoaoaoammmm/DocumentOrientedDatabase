#ifndef DATA_BASE_TAKE_3_TUPLE_H
#define DATA_BASE_TAKE_3_TUPLE_H

#include <stdio.h>
#include <inttypes.h>

union tuple_header {
    struct {
        uint64_t parent;
        uint64_t alloc;
    };
    struct {
        uint64_t prev;
        uint64_t next;
    };
};

struct tuple {
    union tuple_header header;
    uint64_t *data;
};

#endif //DATA_BASE_TAKE_3_TUPLE_H
