#ifndef DATA_BASE_TAKE_3_OPERATION_H
#define DATA_BASE_TAKE_3_OPERATION_H

#include "data_manager.h"
#include <unistd.h>
#include <string.h>

enum crud_operation_status swap_tuple_to(FILE *file, uint64_t pos_from, uint64_t pos_to, size_t tuple_size);

enum crud_operation_status insert_new_tuple(FILE *file, struct tuple *tuple, size_t full_tuple_size, uint64_t *tuple_pos);

enum crud_operation_status insert_string_tuple(FILE *file, char *string, size_t tuple_size, uint64_t *str_pos);

enum crud_operation_status remove_from_id_array(FILE *file, uint64_t id, uint64_t* offset);

enum crud_operation_status id_to_offset(FILE *file, uint64_t id, uint64_t* offset);

enum crud_operation_status change_string_tuple(FILE *file, uint64_t offset, char *new_string, size_t size);

enum crud_operation_status link_strings_to_tuple(FILE *file, struct tuple *tpl, uint64_t tpl_offset);

size_t append_to_id_array(FILE *file, uint64_t offset);

void get_types(FILE *file, uint32_t **types, size_t *size);

enum crud_operation_status {
    CRUD_OK = 0,
    CRUD_END_OF_FILE,
    CRUD_INVALID
};

#endif //DATA_BASE_TAKE_3_OPERATION_H
