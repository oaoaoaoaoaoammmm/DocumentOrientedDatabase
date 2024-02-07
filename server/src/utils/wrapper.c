#include "../../include/utils/wrapper.h"

void add_tuple_test(FILE *file, uint64_t *fields, uint64_t parent_id) {
    clock_t cl = clock();
    add_tuple(file, fields, parent_id);
    printf("%f\n", (double) (clock() - cl) / CLOCKS_PER_SEC);
}

void find_tuple_by_id_test(FILE *file, struct tuple *tuple, uint64_t id) {
    clock_t cl = clock();
    find_tuple_by_id(file, &tuple, id);
    printf("%f\n", (double) (clock() - cl) / CLOCKS_PER_SEC);
}

void find_tuple_by_field_test(FILE *file, uint64_t field_number, uint64_t condition, struct result_list_tuple *res) {
    clock_t cl = clock();
    find_tuple_by_field(file, field_number, &condition, &res);
    printf("%f\n", (double) (clock() - cl) / CLOCKS_PER_SEC);
}

void update_tuple_field_by_id_test(FILE *file, uint64_t field_number, uint64_t *new_value, uint64_t id) {
    clock_t cl = clock();
    update_tuple_field_by_id(file, field_number, new_value, id);
    printf("%f\n", (double) (clock() - cl) / CLOCKS_PER_SEC);
}

void remove_tuple_by_id_test(FILE *file, uint64_t id) {
    clock_t cl = clock();
    remove_tuple_by_id(file, id, 0);
    printf("%f\n", (double) (clock() - cl) / CLOCKS_PER_SEC);
}

void find_tuple_by_parent_test(FILE *file, uint64_t parent_id, struct result_list_tuple *res) {
    clock_t cl = clock();
    find_tuple_by_parent(file, parent_id, &res);
    printf("%f\n", (double) (clock() - cl) / CLOCKS_PER_SEC);
}