#pragma once

#ifndef SERVERFS_H
#define SERVERFS_H

#include "packets.h"
#include <sys/socket.h>
#include <sys/types.h>

struct client_data {
    int csock;
    struct sockaddr_storage storage;
};

void usage(int argc, char **argv);
void last_mod_msg2_send(int* socket_ptr, const char* filename); 
void* client_thread(void *data);

#endif /* SERVERFS */