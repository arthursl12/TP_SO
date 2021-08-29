#ifndef PACKETS_H
#define PACKETS_H
#pragma once

#include <stdlib.h>
#include <time.h>

#include <arpa/inet.h>

void logexit(const char *msg);

int addrparse(const char *addrstr, const char *portstr,
              struct sockaddr_storage *storage);

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize);

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage);


void last_mod_msg1_encode(char** msg, size_t* size);
void last_mod_msg2_encode(const char* path, char** msg, size_t* size);
time_t last_mod_msg2_decode(char* msg);

void hard_disk_req_msg3_encode(char** msg, size_t* size);
void send_file(int* socket_ptr, const char* filename);
void recv_file(int* socket_ptr, const char* filename);

void hard_disk_send_msg6_encode(char** msg, size_t* size);
void hard_disk_send(int* socket_ptr);
void file_packet_encode(char* buf, int bufsize, char** msg, size_t* size);
void file_packet_decode(char* msg, int size, char** buf, int* bufsize);

uint16_t msg_code(char* msg);
void print_bytes(char* bytearray, size_t size);

/* Client functions */
void last_mod_msg1_send(int* socket_ptr);
void hard_disk_req_msg3_send(int* socket_ptr);
void update_if_needed(int* socket_ptr);

#endif /* PACKETS */


