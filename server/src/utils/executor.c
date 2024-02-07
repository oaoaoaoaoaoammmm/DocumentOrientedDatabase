#include "../../include/utils/executor.h"

void get_text_tuple(FILE *file, struct tuple *tuple, char *response) {

    char *curr = calloc(256, sizeof(char));

    char *str;
    read_string_from_tuple(file, &str, (uint64_t) 4, tuple->data[0]);

    double res;
    memcpy(&res, &(tuple->data[2]), sizeof(tuple->data[2]));


    snprintf(curr, 256,
             "{\n\tid - %lu\n\tparent_id - %lu\n\tname - %s\n\tage - %lu\n\theight - %.3f\n\thealthy - %lu\n}\n",
             tuple->header.alloc,
             tuple->header.parent,
             str,
             tuple->data[1],
             res,
             tuple->data[3]
    );

    strcat(response, curr);
    free(str);
}


void get_text_tuple_from_list(FILE *file, struct result_list_tuple *list, char *response) {
    if (list != NULL) {
        struct result_list_tuple *next;
        while (list != NULL) {
            next = list->prev;
            get_text_tuple(file, list->value, response);
            list = next;
        }
    }
}

void execute_request(char xml_request[], FILE *file, int fd) {

    struct tree_header *treeHeader = malloc(sizeof(struct tree_header));
    read_tree_header(treeHeader, file);

    xmlDocPtr request_tree = xmlReadMemory(xml_request, MAX_REQUEST_SIZE, 0, NULL, XML_PARSE_RECOVER);
    xmlNodePtr lastNode = request_tree->last;

    while (lastNode->last != NULL) {
        lastNode = lastNode->last;
    }

    char *response = calloc(MAX_RESPONSE_SIZE, sizeof(char));

    if (xmlStrEqual(request_tree->last->name, xmlCharStrdup("add"))) {

        uint64_t *data = malloc(sizeof(uint64_t) * 4);
        data[0] = (uint64_t) xmlGetProp(lastNode, xmlCharStrdup(treeHeader->pattern[0]->key_value));
        data[1] = (uint64_t) atoi((char *) xmlGetProp(lastNode, xmlCharStrdup(treeHeader->pattern[1]->key_value)));
        double d = atof((char *) xmlGetProp(lastNode, xmlCharStrdup(treeHeader->pattern[2]->key_value)));
        memcpy(&data[2], &d, sizeof(double));
        data[3] = (uint64_t) atoi((char *) xmlGetProp(lastNode, xmlCharStrdup(treeHeader->pattern[3]->key_value)));


        if (add_tuple(file, data, atoi((char *) xmlGetProp(lastNode->parent, xmlCharStrdup(TUPLE_ID)))) == 0) {
            response = "added!\n";
        }

    } else if (xmlStrEqual(request_tree->last->name, xmlCharStrdup("remove"))) {
        if (remove_tuple_by_id(file, atoi((char *) xmlGetProp(lastNode, xmlCharStrdup(TUPLE_ID))), 0) == 0) {
            response = "deleted!\n";
        }
    } else if (xmlStrEqual(request_tree->last->name, xmlCharStrdup("update"))) {

        uint64_t param = atoi((char *) xmlGetProp(lastNode, xmlCharStrdup(treeHeader->pattern[1]->key_value)));

        if (update_tuple_field_by_id(
                file,
                INTEGER_TYPE,
                &param,
                atoi((char *) xmlGetProp(lastNode->parent, xmlCharStrdup(TUPLE_ID)))
        ) == 0) {
            response = "updated\n";
        }

    } else if (xmlStrEqual(request_tree->last->name, xmlCharStrdup("find"))) {

        if (xmlStrEqual(xmlGetProp(request_tree->last->last, xmlCharStrdup(TUPLE_ID)), xmlCharStrdup(STAR))) {

            if (request_tree->last->children == lastNode) {

                struct result_list_tuple *list = NULL;
                find_all(file, &list);
                get_text_tuple_from_list(file, list, response);
                free_result_list(list);

            } else {
                struct result_list_tuple *list = NULL;
                uint64_t cond = atoi((char *) xmlGetProp(lastNode,
                                                         xmlCharStrdup(treeHeader->pattern[1]->key_value)));
                find_tuple_by_field(file,
                                    INTEGER_TYPE,
                                    (uint64_t *) cond,
                                    &list);
                get_text_tuple_from_list(file, list, response);
                free_result_list(list);
            }
        } else {
            if (request_tree->last->children == lastNode) {

                struct tuple *tuple;
                uint64_t id = atoi((char *) xmlGetProp(lastNode, xmlCharStrdup(TUPLE_ID)));
                find_tuple_by_id(file, &tuple, id);
                get_text_tuple(file, tuple, response);

            } else {

                struct result_list_tuple *list = NULL;
                uint64_t id = atoi((char *) xmlGetProp(request_tree->last->children, xmlCharStrdup(TUPLE_ID)));
                find_tuple_by_parent(file, id, &list);
                get_text_tuple_from_list(file, list, response);
                free_result_list(list);
            }
        }
    }
    send_response(response, fd);
}