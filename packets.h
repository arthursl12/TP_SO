#ifndef PACKETS_H
#define PACKETS_H
#pragma once

#include <stdlib.h>

#include <arpa/inet.h>

void logexit(const char *msg);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage);


void last_mod_msg_encode(const char* path, char** msg, size_t* size);
uint16_t msg_code(char* msg);
time_t last_mod_msg_decode(char* msg);
void print_bytes(char* bytearray, size_t size);

#endif /* PACKETS */
