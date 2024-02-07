#ifndef DATA_BASE_TAKE_3_TREE_H
#define DATA_BASE_TAKE_3_TREE_H

#include <stdio.h>
#include <inttypes.h>

struct tree_header {
    struct tree_subheader *subheader;
    struct key **pattern;
    uint64_t *id_sequence;
};

struct tree_subheader {
    uint64_t ASCII_signature;
    uint64_t cur_id;
    uint64_t pattern_size;
};

#pragma pack(push, 8)
struct key {
    struct key_header *header;
    char *key_value;
};
struct key_header {
    uint32_t size;
    uint32_t type;
};
#pragma pack(pop)

#endif //DATA_BASE_TAKE_3_TREE_H
