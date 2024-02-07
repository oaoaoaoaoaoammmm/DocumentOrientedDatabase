#include "../include/utils/file_manager.h"
#include "../include/utils/data_manager.h"
#include "../include/utils/wrapper.h"
#include "../include/server.h"
#include "../include/utils/executor.h"

void print_tuple(FILE *file, struct tuple *tuple) {
    printf("{\n");

    printf("%sid - %lu\n", TAB, tuple->header.alloc);
    printf("%sparent - %lu\n", TAB, tuple->header.parent);

    char *s;
    read_string_from_tuple(file, &s, (uint64_t) 4, tuple->data[0]);
    printf("%sname - %s\n", TAB, s);
    free(s);

    printf("%sage = %ld\n", TAB, tuple->data[1]);

    double res;
    memcpy(&res, &(tuple->data[2]), sizeof(tuple->data[2]));
    printf("%sheight = %.3f\n", TAB, res);

    printf("%shealthy = %ld\n", TAB, tuple->data[3]);

    printf("}\n");
}


void print_tuple_list(FILE *file, struct result_list_tuple *list) {
    if (list != NULL) {
        struct result_list_tuple *next;
        while (list != NULL) {
            next = list->prev;
            print_tuple(file, list->value);
            list = next;
        }
    }
}

void execute(FILE *file) {

    uint64_t *fields = malloc(sizeof(uint64_t * ) * 4);
    fields[0] = (uint64_t) "new";
    fields[1] = 30;
    double d = 170.0;
    memcpy(&fields[2], &d, sizeof(d));
    fields[3] = 0;
    uint64_t parent_id = 1;

    printf("################################ ADD TUPLE ################################\n");
    add_tuple(file, fields, parent_id);
    print_tuple_array_from_file(file);

    printf("################################ FIND TUPLE BY ID 4 ################################\n");
    struct tuple *tuple;
    find_tuple_by_id(file, &tuple, 4);
    print_tuple(file, tuple);
    free(tuple);

    printf("################################ FIND TUPLE BY FIELD INT = 21 ################################\n");
    struct result_list_tuple *res = NULL;
    find_tuple_by_field(file, (uint64_t) 1, (uint64_t *) 21, &res);
    print_tuple_list(file, res);
    free_result_list(res);

    printf("################################ UPDATE TUPLE'S FIELD BY ID 4 INT = 22 ################################\n");
    update_tuple_field_by_id(file, (uint64_t) 1, (uint64_t *) 22, (uint64_t) 4);
    print_tuple_array_from_file(file);

    printf("################################ FIND BY PARENT ID 1 ################################\n");
    struct result_list_tuple *res2 = NULL;
    find_tuple_by_parent(file, (uint64_t) 1, &res2);
    print_tuple_list(file, res2);
    free_result_list(res2);

    printf("################################ REMOVE TUPLE BY ID 4 ################################\n");
    remove_tuple_by_id(file, 4, 0);
    print_tuple_array_from_file(file);
}

int main(int argc, char **argv) {

    FILE *file;
    open_file_anyway(&file, "data.txt");

    if (argv[2] != NULL && strcmp(argv[2], "init") == 0) {

        char **pattern = malloc(4 * sizeof(char *));
        uint32_t *types = malloc(4 * sizeof(uint32_t));
        size_t *sizes = malloc(4 * sizeof(size_t));
        size_t pattern_size;

        char name[5] = "name";
        pattern[0] = name;

        char age[4] = "age";
        pattern[1] = age;

        char height[7] = "height";
        pattern[2] = height;

        char healthy[10] = "healthy";
        pattern[3] = healthy;

        types[0] = 0;
        types[1] = 1;
        types[2] = 3;
        types[3] = 2;
        pattern_size = 4;
        sizes[0] = 8;
        sizes[1] = 8;
        sizes[2] = 8;
        sizes[3] = 8;

        init_empty_file(file, pattern, types, pattern_size, sizes);

        free(pattern);
        free(types);
        free(sizes);
    }

    print_tree_header_from_file(file);
    print_tuple_array_from_file(file);

    int server = -1;
    if (argc > 1 && argv[1] != NULL) {
        server = start_server(atoi(argv[1]));
    }

    if (server == -1) {
        printf("error");
        exit(1);
    }

    printf("server stated\n");

    while (1) {

        char *request_xml = calloc(MAX_REQUEST_SIZE, sizeof(char));

        int fd = handler_request(server, request_xml);

        execute_request(request_xml, file, fd);

        print_tuple_array_from_file(file);
    }

    close_file(file);

    return 0;
}
