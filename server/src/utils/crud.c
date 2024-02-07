#include "../../include/utils/crud_interface.h"


enum crud_operation_status swap_last_tuple_to(FILE *file, uint64_t pos_to, size_t tuple_size) {
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    fseek(file, (long) -(get_real_tuple_size(size) + sizeof(union tuple_header)), SEEK_END);
    uint64_t pos_from = ftell(file);
    enum crud_operation_status status = swap_tuple_to(file, pos_to, pos_from, tuple_size);
    ftruncate(fileno(file), (long) pos_from);
    free(types);

    return status;
}

enum crud_operation_status add_tuple(FILE *file, uint64_t *fields, uint64_t parent_id) {
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    struct tree_header *header = malloc(sizeof(struct tree_header));
    enum file_read_status status_header = read_tree_header(header, file);
    if (status_header == READ_INVALID) {
        free(types);
        free(header);
        return CRUD_INVALID;
    }

    struct tuple *new_tuple = malloc(sizeof(struct tuple));
    union tuple_header new_tuple_header = {.parent = parent_id, .alloc = header->subheader->cur_id};
    new_tuple->header = new_tuple_header;
    new_tuple->data = malloc(get_real_tuple_size(size));
    uint64_t link;

    for (size_t iter = 0; iter < size; iter++) {
        if (types[iter] == STRING_TYPE) {
            insert_string_tuple(file, (char *) fields[iter], get_real_tuple_size(size), &link);
            new_tuple->data[iter] = link;
        } else {
            new_tuple->data[iter] = (uint64_t) fields[iter];
        }
    }

    size_t full_tuple_size = sizeof(union tuple_header) + get_real_tuple_size(size);
    enum crud_operation_status status = insert_new_tuple(file, new_tuple, full_tuple_size, &link);
    link_strings_to_tuple(file, new_tuple, link);
    append_to_id_array(file, link);

    free(new_tuple->data);
    free(new_tuple);
    free(types);
    free_test_tree_header(header);

    return status;
}

enum crud_operation_status find_tuple_by_id(FILE *file, struct tuple **tuple, uint64_t id) {
    uint64_t offset;
    id_to_offset(file, id, &offset);
    if (offset == NULL_VALUE) return CRUD_INVALID;
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    fseek(file, offset, SEEK_SET);
    read_basic_tuple(file, tuple, size);

    return CRUD_OK;
}

enum crud_operation_status remove_tuple_by_id(FILE *file, uint64_t id, uint8_t str_flag) {
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);

    if (!str_flag) {
        uint64_t offset;
        if (remove_from_id_array(file, id, &offset) == CRUD_INVALID) {
            free(types);
            return CRUD_INVALID;
        }

        for (size_t field_num = 0; field_num < size; field_num++) {
            if (types[field_num] == STRING_TYPE) {
                struct tuple *tpl;
                fseek(file, (long) offset, SEEK_SET);
                read_basic_tuple(file, &tpl, size);
                remove_tuple_by_id(file, tpl->data[field_num], 1);
                free(tpl->data);
                free(tpl);
            }
        }

        swap_last_tuple_to(file, offset, get_real_tuple_size(size));

        struct result_list_tuple *children = NULL;
        find_tuple_by_parent(file, id, &children);
        while (children != NULL) {
            remove_tuple_by_id(file, children->id, 0);
            children = children->prev;
        }

    } else {
        struct tuple *str_tpl;
        while (id != NULL_VALUE) {
            fseek(file, id, SEEK_SET);
            read_string_tuple(file, &str_tpl, size);
            swap_last_tuple_to(file, id, get_real_tuple_size(size) + sizeof(union tuple_header));
            id = str_tpl->header.next;
            free(str_tpl->data);
            free(str_tpl);
            struct tree_header *header = malloc(sizeof(struct tree_header));
            read_tree_header(header, file);
        }
    }

    free(types);
    return CRUD_OK;
}

void append_to_result_list(struct tuple **tuple_to_add, uint64_t id, struct result_list_tuple **result) {

    if ((*result) == NULL) {
        (*result) = malloc(sizeof(struct result_list_tuple));
        (*result)->prev = NULL;

    } else {
        struct result_list_tuple *new_result = malloc(sizeof(struct result_list_tuple));
        new_result->prev = *result;
        *result = new_result;
    }

    (*result)->value = *tuple_to_add;
    (*result)->id = id;
    (*tuple_to_add) = malloc(sizeof(struct tuple));
}

void print_result_list_tuple_id(struct result_list_tuple *result) {
    while (result != NULL) {
        printf("tuple id = %ld\n", result->id);
        result = result->prev;
    }
}

enum crud_operation_status
find_tuple_by_field(FILE *file, uint64_t field_number, uint64_t *condition, struct result_list_tuple **result) {
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    uint64_t type = types[field_number];

    struct tree_header *header = malloc(sizeof(struct tree_header));
    enum file_read_status status_header = read_tree_header(header, file);
    if (status_header == READ_INVALID) {
        free(header);
        return CRUD_INVALID;
    }

    struct tuple *cur_tuple;
    for (size_t i = 0; i < header->subheader->cur_id; i++) {
        if (header->id_sequence[i] == NULL_VALUE) continue;
        fseek(file, header->id_sequence[i], SEEK_SET);
        enum file_read_status status = read_basic_tuple(file, &cur_tuple, size);
        if (status == READ_INVALID) return CRUD_INVALID;

        if (type == STRING_TYPE) {
            char *s;
            enum file_read_status status_tuple = read_string_from_tuple(file, &s, size, cur_tuple->data[field_number]);
            if (status_tuple == READ_INVALID) return CRUD_INVALID;
            if (!strcmp(s, (char *) condition)) {
                append_to_result_list(&cur_tuple, i, result);
            }
        } else {
            if (cur_tuple->data[field_number] == condition) {
                append_to_result_list(&cur_tuple, i, result);
            }
        }
    }

    return CRUD_OK;
}

enum crud_operation_status find_tuple_by_parent(FILE *file, uint64_t parent_id, struct result_list_tuple **result) {
    if (parent_id == 0) return CRUD_OK;
    struct tree_header *header = malloc(sizeof(struct tree_header));
    enum file_read_status status_header = read_tree_header(header, file);
    if (status_header == READ_INVALID) {
        free(header);
        return CRUD_INVALID;
    }
    struct tuple *cur_tuple = malloc(sizeof(struct tuple));
    for (size_t i = 0; i < header->subheader->cur_id; i++) {
        if (header->id_sequence[i] == NULL_VALUE) continue;
        fseek(file, header->id_sequence[i], SEEK_SET);
        enum file_read_status status = read_basic_tuple(file, &cur_tuple, header->subheader->pattern_size);
        if (status == READ_INVALID) return CRUD_INVALID;
        if (cur_tuple->header.parent == parent_id) {
            append_to_result_list(&cur_tuple, i, result);
        }
    }
    return CRUD_OK;
}

enum crud_operation_status update_tuple_field_by_id(FILE *file, uint64_t field_number, uint64_t *new_value, uint64_t id) {
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    uint64_t type = types[field_number];
    uint64_t offset;
    id_to_offset(file, id, &offset);
    struct tuple *cur_tuple;
    fseek(file, offset, SEEK_SET);
    enum file_read_status status = read_basic_tuple(file, &cur_tuple, size);
    if (status == READ_INVALID) {
        free(types);
        return CRUD_INVALID;
    }
    if (type == STRING_TYPE) {
        change_string_tuple(file, cur_tuple->data[field_number], (char *) new_value, get_real_tuple_size(size));
    } else {
        memcpy(&(cur_tuple->data[field_number]), &new_value, sizeof(* new_value));
        fseek(file, offset, SEEK_SET);
        write_tuple(file, cur_tuple, get_real_tuple_size(size));
    }
    free(types);
    return 0;
}

enum crud_operation_status find_all(FILE *file, struct result_list_tuple **result) {
    struct tree_header *header = malloc(sizeof(struct tree_header));
    enum file_read_status status_header = read_tree_header(header, file);
    if (status_header == READ_INVALID) {
        free(header);
        return CRUD_INVALID;
    }

    for (size_t i = header->subheader->cur_id; i > 0; i--) {
        if (header->id_sequence[i] == NULL_VALUE) continue;
        struct tuple *cur_tuple = malloc(sizeof(struct tuple));

        fseek(file, header->id_sequence[i], SEEK_SET);
        enum file_read_status status = read_basic_tuple(file, &cur_tuple, header->subheader->pattern_size);
        if (status == READ_INVALID) return CRUD_INVALID;
        append_to_result_list(&cur_tuple, i, result);
    }
    return CRUD_OK;
}

enum crud_operation_status remove_all(FILE *file) {
    struct tree_header *header = malloc(sizeof(struct tree_header));
    enum file_read_status status_header = read_tree_header(header, file);
    if (status_header == READ_INVALID) {
        free(header);
        return CRUD_INVALID;
    }

    for (size_t i = header->subheader->cur_id; i > 0; i--) {
        if (header->id_sequence[i] == NULL_VALUE) continue;
        remove_tuple_by_id(file, i, 0);
    }
    return CRUD_OK;
}

void free_result_list(struct result_list_tuple *result) {
    if (result != NULL) {
        struct result_list_tuple *next;
        while (result != NULL) {
            next = result->prev;
            free(result->value->data);
            free(result->value);
            free(result);
            result = next;
        }
    }
}