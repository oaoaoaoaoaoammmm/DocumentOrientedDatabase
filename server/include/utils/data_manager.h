#ifndef DATA_BASE_TAKE_3_DATA_MANAGER_H
#define DATA_BASE_TAKE_3_DATA_MANAGER_H

#include <stdlib.h>
#include <string.h>

#include "../config/config.h"
#include "../model/tuple.h"
#include "../model/header.h"
#include "file_manager.h"

size_t get_real_id_array_size(uint64_t pattern_size, uint64_t cur_id);

size_t get_real_tuple_size(uint64_t pattern_size);

size_t get_id_array_size(uint64_t pattern_size, uint64_t cur_id);

enum file_read_status read_string_tuple(FILE *file, struct tuple **tuple, uint64_t pattern_size);

enum file_read_status read_string_from_tuple(FILE *file, char **string, uint64_t pattern_size, uint64_t offset);

enum file_write_status write_tree_header(FILE *file, struct tree_header *header);

enum file_write_status
init_empty_file(FILE *file, char **pattern, uint32_t *types, size_t pattern_size, size_t *key_sizes);

enum file_write_status write_tuple(FILE *file, struct tuple *tuple, size_t tuple_size);

enum file_read_status read_basic_tuple(FILE *file, struct tuple **tuple, uint64_t pattern_size);

enum file_write_status write_pattern(FILE *file, struct key **pattern, size_t pattern_size);

void print_tree_header_from_file(FILE *file);

void print_tuple_array_from_file(FILE *file);

void free_test_tree_header(struct tree_header *header);

enum file_read_status read_tree_header(struct tree_header *header, FILE *file);

#endif //DATA_BASE_TAKE_3_DATA_MANAGER_H
