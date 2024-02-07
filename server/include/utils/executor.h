#ifndef XML_EXECUTOR_H
#define XML_EXECUTOR_H

#include "crud_interface.h"
#include "../server.h"

void execute_request(char xml_request[], FILE* file, int fd);

#endif //XML_EXECUTOR_H
