#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>

void logexit(const char *msg){
    perror(msg);
    // exit(EXIT_FAILURE);
}

int addrparse(const char *addrstr,
              const char *portstr,
              struct sockaddr_storage *storage)
{
    if (addrstr == NULL || portstr == NULL){
        return -1;
    }

    uint16_t port = (uint16_t)atoi(portstr);    // unsigned short
    if (port == 0){
        return -1;
    }
    port = htons(port);     // host to network short

    struct in_addr inaddr4;    // 32-bit IPv4 address
    if (inet_pton(AF_INET, addrstr, &inaddr4)) {
        struct sockaddr_in* addr4 = (struct sockaddr_in*) storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    struct in6_addr inaddr6;    // 128-bit IPv6 address
    if (inet_pton(AF_INET6, addrstr, &inaddr6)) {
        struct sockaddr_in6* addr6 = (struct sockaddr_in6*) storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        // addr6->sin6_addr = inaddr6;
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if (addr->sa_family == AF_INET) {
        version = 4;
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr4->sin_port); // network to host short
    } else if (addr->sa_family == AF_INET6) {
        version = 6;
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr,
                       INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        port = ntohs(addr6->sin6_port); // network to host short
    } else {
        logexit("unknown protocol family.");
    }
    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}

int server_sockaddr_init(const char *proto, const char *portstr,
                         struct sockaddr_storage *storage) {
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    memset(storage, 0, sizeof(*storage));
    if (0 == strcmp(proto, "v4")) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY;
        addr4->sin_port = port;
        return 0;
    } else if (0 == strcmp(proto, "v6")) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any;
        addr6->sin6_port = port;
        return 0;
    } else {
        return -1;
    }
}


/*
Prints a byte array in hexadecimal
*/
void print_bytes(char* bytearray, size_t size){
    int i;
    for (i = 0; i < size; i++){
        printf("%hhx ", bytearray[i]);
    }
    printf("\\\n");
}

/*
Creates a message with last modified date of given file.
This message has code 1.

Returns void.
Created message will be allocated in given 'msg' pointer.
Its size will be placed in given 'size' pointer.
*/
void last_mod_msg_encode(const char* path, char** msg, size_t* size){
    // 2-byte integer: message code
    u_int16_t code = 1; 

    // Get file attributes
    struct stat attr;
    stat(path, &attr);
    time_t date = attr.st_mtime;

    // Create byte-like message
    *size = sizeof(code)+sizeof(date);
    *msg = (char*) malloc(*size);

    // Copying contents to message
    memcpy(*msg, &code, sizeof(code));
    memcpy(*msg+sizeof(code), &date, sizeof(date));

    print_bytes(*msg, *size);
}

/*
Discover code of given message. A message's code is an 2-byte
integer composed by the first two message bytes.
*/
uint16_t msg_code(char* msg){
    u_int16_t otherint;
    memcpy(&otherint, msg, sizeof(otherint));
    return otherint;
}

/*
Decodes a message with last modified date.
Returns enconded date.
*/
time_t last_mod_msg_decode(char* msg){
    u_int16_t otherint;
    time_t time;
    memcpy(&time, msg+sizeof(otherint), sizeof(time));
    // printf("Last modified time: %s\n", ctime(&time));
    return time;
}
