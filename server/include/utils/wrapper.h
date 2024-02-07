#ifndef DATA_BASE_TAKE_3_WRAP_H
#define DATA_BASE_TAKE_3_WRAP_H
#include <stdio.h>
#include <inttypes.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>
#include "time.h"
#include "crud_interface.h"

void add_tuple_test(FILE *file, uint64_t *fields, uint64_t parent_id);

void find_tuple_by_id_test(FILE *file, struct tuple *tuple, uint64_t id);

void find_tuple_by_field_test(FILE *file, uint64_t field_number, uint64_t condition, struct result_list_tuple *res);

void update_tuple_field_by_id_test(FILE *file, uint64_t field_number, uint64_t *new_value, uint64_t id);

void remove_tuple_by_id_test(FILE *file, uint64_t id);

void find_tuple_by_parent_test(FILE *file, uint64_t parent_id, struct result_list_tuple *res);

#endif //DATA_BASE_TAKE_3_WRAP_H
