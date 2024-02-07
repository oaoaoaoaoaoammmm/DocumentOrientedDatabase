#ifndef XML_SERVER_H
#define XML_SERVER_H

#include <netinet/in.h>

#include "apios.h"

int start_server(int port);

int handler_request(int server, char buf[]);

void send_response(char* msg, int fd);

void finish_server(int server);

#endif //XML_SERVER_H
