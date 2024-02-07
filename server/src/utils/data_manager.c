#include "../../include/utils/data_manager.h"
#include "../../include/utils/low_data_manager.h"

size_t get_real_tuple_size(uint64_t pattern_size) {
    return pattern_size * SINGLE_TUPLE_VALUE_SIZE < MINIMAL_TUPLE_SIZE
           ? MINIMAL_TUPLE_SIZE
           : pattern_size * SINGLE_TUPLE_VALUE_SIZE;
}

uint64_t max(uint64_t n1, uint64_t n2) {
    if (n1 > n2) return n1;
    return n2;
}

size_t get_id_array_size(uint64_t pattern_size, uint64_t cur_id) {
    size_t real_tuple_size = get_real_tuple_size(pattern_size) + sizeof(union tuple_header);
    if (cur_id == 0) cur_id++;
    size_t whole = (cur_id * OFFSET_VALUE_SIZE / real_tuple_size);
    size_t frac = (cur_id * OFFSET_VALUE_SIZE % real_tuple_size ? 1 : 0);
    size_t value = max((frac + whole) * real_tuple_size / OFFSET_VALUE_SIZE,
                       MIN_ID_ARRAY_SIZE * real_tuple_size / OFFSET_VALUE_SIZE);
    return value;
}

enum file_read_status read_tree_subheader(struct tree_subheader *header, FILE *file) {
    enum file_read_status code = read_from_file(file, header, sizeof(struct tree_subheader));
    return code;
}


enum file_read_status read_key(struct key *key, FILE *file) {
    struct key_header *header = malloc(sizeof(struct key_header));
    enum file_read_status code = read_from_file(file, header, sizeof(struct key_header));
    key->header = header;

    char *key_value = (char *) malloc(
            header->size / FILE_GRANULARITY + (header->size % FILE_GRANULARITY ? FILE_GRANULARITY : 0));
    code |= read_from_file(file, key_value, header->size);
    key->key_value = key_value;

    return code;
}

enum file_read_status read_tree_header(struct tree_header *header, FILE *file) {
    fseek(file, 0, SEEK_SET);
    struct tree_subheader *subheader = malloc(sizeof(struct tree_subheader));
    enum file_read_status code = read_tree_subheader(subheader, file);
    header->subheader = subheader;
    struct key **array_of_key = malloc(sizeof(struct key *) * subheader->pattern_size);
    header->pattern = array_of_key;
    for (size_t iter = subheader->pattern_size; iter-- > 0; array_of_key++) {
        struct key *element_pattern = malloc(sizeof(struct key));
        code |= read_key(element_pattern, file);
        *array_of_key = element_pattern;
    }

    size_t real_id_array_size = get_id_array_size(header->subheader->pattern_size, header->subheader->cur_id);
    uint64_t *id_array = (uint64_t *) malloc(real_id_array_size * sizeof(uint64_t));
    header->id_sequence = id_array;
    code |= read_from_file(file, id_array, real_id_array_size * sizeof(uint64_t));
    return code;
}

enum file_read_status read_basic_tuple(FILE *file, struct tuple **tuple, uint64_t pattern_size) {
    union tuple_header *header = malloc(sizeof(union tuple_header));
    enum file_read_status code = read_from_file(file, header, sizeof(union tuple_header));
    struct tuple *temp_tuple = malloc(sizeof(struct tuple));
    temp_tuple->header = *header;
    free(header);

    uint64_t *data = malloc(get_real_tuple_size(pattern_size));
    code |= read_from_file(file, data, get_real_tuple_size(pattern_size));
    temp_tuple->data = data;
    *tuple = temp_tuple;

    return code;
}

enum file_read_status read_string_tuple(FILE *file, struct tuple **tuple, uint64_t pattern_size) {
    union tuple_header *header = malloc(sizeof(union tuple_header));
    enum file_read_status code = read_from_file(file, header, sizeof(union tuple_header));
    struct tuple *temp_tuple = malloc(sizeof(struct tuple));
    temp_tuple->header = *header;
    free(header);

    uint64_t *data = (uint64_t *) malloc(get_real_tuple_size(pattern_size));
    code |= read_from_file(file, data, get_real_tuple_size(pattern_size));
    temp_tuple->data = data;
    *tuple = temp_tuple;

    return code;
}

size_t how_long_string_is(FILE *file, uint64_t offset) {
    fseek(file, offset, SEEK_SET);
    size_t len = 1;
    union tuple_header *temp_header = malloc(sizeof(union tuple_header));
    read_from_file(file, temp_header, sizeof(union tuple_header));

    while (temp_header->next) {
        fseek(file, temp_header->next, SEEK_SET);
        read_from_file(file, temp_header, sizeof(union tuple_header));
        len++;
    }
    free(temp_header);
    return len;
}

enum file_read_status read_string_from_tuple(FILE *file, char **string, uint64_t pattern_size, uint64_t offset) {
    size_t str_len = how_long_string_is(file, offset);
    size_t rts = get_real_tuple_size(pattern_size);
    *string = malloc(str_len * rts);
    struct tuple *temp_tuple;
    for (size_t iter = 0; iter < str_len; iter++) {
        fseek(file, offset, SEEK_SET);
        read_string_tuple(file, &temp_tuple, pattern_size);
        offset = temp_tuple->header.next;
        strncpy((*string) + rts * iter, (char *) temp_tuple->data, rts);
        free(temp_tuple->data);
        free(temp_tuple);
    }
    return 0;
}


enum file_write_status write_tree_subheader(FILE *file, struct tree_subheader *subheader) {
    enum file_write_status code = write_to_file(file, subheader, sizeof(struct tree_subheader));
    return code;
}

enum file_write_status write_pattern(FILE *file, struct key **pattern, size_t pattern_size) {
    enum file_write_status code = NULL_VALUE;
    for (; pattern_size-- > 0; pattern++) {
        code |= write_to_file(file, (*pattern)->header, sizeof(struct key_header));
        code |= write_to_file(file, (*pattern)->key_value, (*pattern)->header->size);
    }
    return code;
}

enum file_write_status write_id_sequence(FILE *file, uint64_t *id_sequence, size_t size) {
    enum file_write_status code = write_to_file(file, id_sequence, size);
    return code;
}

enum file_write_status write_tree_header(FILE *file, struct tree_header *header) {
    fseek(file, 0, SEEK_SET);

    size_t pattern_size = header->subheader->pattern_size;

    enum file_write_status code = write_tree_subheader(file, header->subheader);
    if (code != WRITE_OK) {
        printf("WRITE ERROR\n");
    }

    fseek(file, sizeof(struct tree_subheader), SEEK_SET);
    code |= write_pattern(file, header->pattern, pattern_size);
    size_t real_id_array_size = get_id_array_size(header->subheader->pattern_size, header->subheader->cur_id);
    code |= write_id_sequence(file, header->id_sequence, real_id_array_size * sizeof(uint64_t));


    if (code == 2) {
        printf("WRITE ERROR\n");
    }
    return code;
}

void generate_empty_tree_subheader(struct tree_subheader *subheader, size_t pattern_size) {
    subheader->pattern_size = (uint64_t) pattern_size;
    subheader->cur_id = 1;
    subheader->ASCII_signature = BIG_ENDIAN_SIGNATURE;
}

void copy_string(char *from, char *to, size_t size_from, size_t size_to) {
    while (size_to-- && size_from--) *(to++) = *(from++);
}

void generate_empty_pattern(struct key **key_pattern, char **pattern, uint32_t *types, size_t pattern_size,
                            size_t *key_sizes) {
    struct key *pattern_key;
    size_t real_size;
    for (size_t iter = pattern_size; iter-- > 0; key_pattern++, pattern++, types++, key_sizes++) {
        pattern_key = malloc(sizeof(struct key));
        real_size = (*key_sizes) / FILE_GRANULARITY * FILE_GRANULARITY +
                    ((*key_sizes) % FILE_GRANULARITY ? FILE_GRANULARITY : 0);
        char *appended_string = malloc(sizeof(char) * real_size);
        copy_string(*pattern, appended_string, *key_sizes, real_size);
        pattern_key->key_value = appended_string;
        pattern_key->header = malloc(sizeof(struct key_header));
        pattern_key->header->size = (uint32_t) real_size;
        pattern_key->header->type = *types;
        *key_pattern = pattern_key;
    }
}

void generate_empty_tree_header(char **pattern, uint32_t *types, size_t pattern_size, size_t *key_sizes,
                                struct tree_header *header) {
    header->subheader = (struct tree_subheader *) malloc(sizeof(struct tree_subheader));
    generate_empty_tree_subheader(header->subheader, pattern_size);
    header->pattern = malloc(sizeof(struct key *) * pattern_size);
    generate_empty_pattern(header->pattern, pattern, types, pattern_size, key_sizes);
    header->id_sequence = malloc(
            sizeof(uint64_t) * get_id_array_size(header->subheader->pattern_size, header->subheader->cur_id));
    for (size_t iter = 0;
         iter < get_id_array_size(header->subheader->pattern_size, header->subheader->cur_id); iter++) {
        header->id_sequence[iter] = 0;
    }
}

enum file_write_status init_empty_file(FILE *file, char **pattern, uint32_t *types, size_t pattern_size, size_t *key_sizes) {

    fseek(file, 0, SEEK_SET);
    struct tree_header *header = (struct tree_header *) malloc(sizeof(struct tree_header));
    generate_empty_tree_header(pattern, types, pattern_size, key_sizes, header);
    return write_tree_header(file, header);
}

enum file_write_status write_tuple(FILE *file, struct tuple *tuple, size_t tuple_size) {
    union tuple_header *tuple_header = malloc(sizeof(union tuple_header));
    *tuple_header = tuple->header;
    enum file_write_status code = write_to_file(file, tuple_header, sizeof(union tuple_header));
    free(tuple_header);
    code |= write_to_file(file, tuple->data, tuple_size);
    return code;
}

void print_tree_header_from_file(FILE *file) {
    struct tree_header *header = malloc(sizeof(struct tree_header));
    read_tree_header(header, file);

    printf("File header - {\n");
    printf("%s%s %lu\n", TAB, "Current ID:", header->subheader->cur_id);

    printf("%s%s", TAB, "Fields:\n");
    for (size_t iter = 0; iter < header->subheader->pattern_size; iter++) {
        printf("%s%sType %d - %s\n",
               TAB,
               TAB,
               header->pattern[iter]->header->type,
               header->pattern[iter]->key_value
        );
    }
    printf("}\n");
    free_test_tree_header(header);
}

void print_tuple_array_from_file(FILE *file) {
    struct tree_header header;
    read_tree_header(&header, file);
    uint32_t *types;
    size_t size;
    get_types(file, &types, &size);
    struct tuple *cur_tuple;

    for (size_t i = 0; i < header.subheader->cur_id; i++) {
        if (header.id_sequence[i] == NULL_VALUE) continue;
        fseek(file, header.id_sequence[i], SEEK_SET);
        read_basic_tuple(file, &cur_tuple, size);
        printf("{\n");
        printf("%sid = %zu\n", TAB, i);
        printf("%sparent = %ld\n", TAB, cur_tuple->header.parent);
        for (size_t iter = 0; iter < size; iter++) {
            if (types[iter] == STRING_TYPE) {
                char *s;
                read_string_from_tuple(file, &s, header.subheader->pattern_size, cur_tuple->data[iter]);
                printf("%s%s = %s\n", TAB, header.pattern[iter]->key_value, s);
                free(s);
            } else if (types[iter] == INTEGER_TYPE) {
                printf("%s%s = %ld\n", TAB, header.pattern[iter]->key_value, cur_tuple->data[iter]);
            } else if (types[iter] == FLOAT_TYPE) {
                double res;
                memcpy(&res, &(cur_tuple->data[iter]), sizeof(cur_tuple->data[iter]));
                printf("%s%s = %.3f\n", TAB, header.pattern[iter]->key_value, res);
            } else if (types[iter] == BOOLEAN_TYPE) {
                printf("%s%s = %lu\n", TAB, header.pattern[iter]->key_value, cur_tuple->data[iter]);
            }
        }
        printf("}\n");
        free(cur_tuple->data);
        free(cur_tuple);
    }

    free(types);
}

void free_test_tree_header(struct tree_header *header) {
    for (size_t iter = 0; iter < header->subheader->pattern_size; iter++) {
        free(header->pattern[iter]->key_value);
        free(header->pattern[iter]->header);
        free(header->pattern[iter]);
    }

    free(header->pattern);
    free(header->id_sequence);
    free(header->subheader);
    free(header);
}
