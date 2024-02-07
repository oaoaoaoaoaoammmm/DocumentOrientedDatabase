#ifndef DATA_BASE_TAKE_3_FILE_MANAGER_H
#define DATA_BASE_TAKE_3_FILE_MANAGER_H

#include <stdio.h>

enum file_read_status read_from_file(FILE *file, void *buffer, size_t size);

enum file_write_status write_to_file(FILE *file, void *buffer, size_t size);

enum file_open_status open_exist_file(char *filename, FILE **file);

enum file_open_status open_new_file(char *filename, FILE **file);

enum file_open_status open_file_anyway(FILE **file, char *filename);

enum file_open_status open_file(char *filename, FILE **file, char *open_descriptor);

void close_file(FILE *file);

enum file_read_status {
    READ_OK = 0,
    READ_END_OF_FILE,
    READ_INVALID
};

enum file_write_status {
    WRITE_OK = 0,
    WRITE_WRONG_INTEGRITY,
    WRITE_INVALID
};

enum file_open_status {
    OPEN_OK = 0,
    OPEN_FAILED
};

#endif //DATA_BASE_TAKE_3_FILE_MANAGER_H
