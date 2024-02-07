#ifndef DATA_BASE_TAKE_3_CRUD_H
#define DATA_BASE_TAKE_3_CRUD_H

#include "low_data_manager.h"

struct result_list_tuple {
    struct result_list_tuple *prev;
    struct tuple *value;
    uint64_t id;
};

void print_result_list_tuple_id(struct result_list_tuple *result);

enum crud_operation_status add_tuple(FILE *file, uint64_t *fields, uint64_t parent_id);

enum crud_operation_status remove_tuple_by_id(FILE *file, uint64_t id, uint8_t str_flag);

enum crud_operation_status find_tuple_by_id(FILE *file, struct tuple **tuple, uint64_t id);

enum crud_operation_status find_tuple_by_field(FILE *file, uint64_t field_number, uint64_t *condition, struct result_list_tuple **result);

enum crud_operation_status find_tuple_by_parent(FILE *file, uint64_t parent_id, struct result_list_tuple **result);

enum crud_operation_status update_tuple_field_by_id(FILE *file, uint64_t field_number, uint64_t *new_value, uint64_t id);

enum crud_operation_status find_all(FILE *file, struct result_list_tuple **result);

enum crud_operation_status remove_all(FILE *file);

void free_result_list(struct result_list_tuple *result);

#endif //DATA_BASE_TAKE_3_CRUD_H
