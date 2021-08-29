#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "filedate.h"

#define BUFSZ 1024
#define SERVER_MOD_FILENAME "lastmod_server.date"
#define SERVER_HD_FILENAME "harddisk_server.dat"
#define CLIENT_MOD_FILENAME "lastmod_client.date"
#define CLIENT_HD_FILENAME "harddisk_client.dat"
// #define CLIENT_FILENAME "lastmod_client_fs.date"

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
Discover code of given message. A message's code is an 2-byte
integer composed by the first two message bytes.
*/
uint16_t msg_code(char* msg){
    u_int16_t otherint;
    memcpy(&otherint, msg, sizeof(otherint));
    return otherint;
}

/*
Creates a message requesting last modified date of file hierarchy archive.
This message has code 1.

Returns void.
Created message will be allocated in given 'msg' pointer.
Its size will be placed in given 'size' pointer.
*/
void last_mod_msg1_encode(char** msg, size_t* size){
    // 2-byte integer: message code
    u_int16_t code = 1; 

    // Create byte-like message
    *size = sizeof(code);
    *msg = (char*) malloc(*size);

    // Copying contents to message
    memcpy(*msg, &code, sizeof(code));

    // print_bytes(*msg, *size);
}

/*
Creates a message requesting file hierarchy archive.
This message has code 3.

Returns void.
Created message will be allocated in given 'msg' pointer.
Its size will be placed in given 'size' pointer.
*/
void hard_disk_req_msg3_encode(char** msg, size_t* size){
    // 2-byte integer: message code
    u_int16_t code = 3; 

    // Create byte-like message
    *size = sizeof(code);
    *msg = (char*) malloc(*size);

    // Copying contents to message
    memcpy(*msg, &code, sizeof(code));

    // print_bytes(*msg, *size);
}

/*
Creates a message with last modified date written in passed file.
This message has code 2.

Returns void.
Created message will be allocated in given 'msg' pointer.
Its size will be placed in given 'size' pointer.
*/
void last_mod_msg2_encode(const char* path, char** msg, size_t* size){
    // 2-byte integer: message code
    u_int16_t code = 2; 

    // Get last mod date from file
    time_t date;
    if(access(path, F_OK) == 0){
        // file exists
        date = getDate(path);
    }else{
        date = time(NULL);
        updateDate(path, date);
    }

    // Create byte-like message
    *size = sizeof(code)+sizeof(date);
    *msg = (char*) malloc(*size);

    // Copying contents to message
    memcpy(*msg, &code, sizeof(code));
    memcpy(*msg+sizeof(code), &date, sizeof(date));

    // print_bytes(*msg, *size);
}

/*
Creates a message with last modified date written in passed file.
This message has code 7.

Returns void.
Created message will be allocated in given 'msg' pointer.
Its size will be placed in given 'size' pointer.
*/
void last_mod_msg7_encode(const char* path, char** msg, size_t* size){
    // 2-byte integer: message code
    u_int16_t code = 7; 

    // Get last mod date from file
    time_t date;
    if(access(path, F_OK) == 0){
        // file exists
        date = getDate(path);
    }else{
        date = time(NULL);
        updateDate(path, date);
    }

    // Create byte-like message
    *size = sizeof(code)+sizeof(date);
    *msg = (char*) malloc(*size);

    // Copying contents to message
    memcpy(*msg, &code, sizeof(code));
    memcpy(*msg+sizeof(code), &date, sizeof(date));

    // print_bytes(*msg, *size);
}

/*
Decodes a message with last modified date.
Returns enconded date.
*/
time_t last_mod_msg2_decode(char* msg){
    u_int16_t otherint;
    time_t time;
    // print_bytes((msg+sizeof(otherint)), sizeof(time));
    memcpy(&time, msg+sizeof(otherint), sizeof(time));
    printf("Last modified time: %s", ctime(&time));
    return time;
}

void send_file(int* socket_ptr, const char* filename){
    // Create buffer and set it to zero
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // Derreference socket pointer in order to use it
    int s = *socket_ptr;

    // Read from file
    FILE *ptr;
    ptr = fopen(filename,"rb");  // r for read, b for binary

    int end = 0;
    int nread;
    // Read BUFSZ blocks of 1 byte
    while( 0 < (nread = fread(buf, 1, BUFSZ, ptr))){
        // print_bytes(buf, BUFSZ);
        // printf("read: %i\n", nread);

        size_t count = send(s, buf, nread, 0);
        // printf("[log] sent: %li",count);
        if (count != strlen(buf)){ logexit("send");}
        memset(buf, 0, BUFSZ);
    }
    memcpy(buf, "DONE", sizeof("DONE"));
    sleep(10);
    size_t count = send(s, buf, BUFSZ, 0);
    // printf("[log] sent: %li",count);
    if (count != strlen(buf)){ logexit("send");}
    memset(buf, 0, BUFSZ);
    fclose(ptr);

}



/*
Creates a packet with code 42, 4 bytes indicating bufsize and then the buffer
itself

Returns void.
Created message will be allocated in given 'msg' pointer.
Its size will be placed in given 'size' pointer.
*/
void file_packet_encode(char* buf, int bufsize, char** msg, size_t* size){
    // 2-byte integer: message code
    u_int16_t code = 8; 

    // Create byte-like message
    *size = sizeof(code)+sizeof(bufsize)+bufsize;
    *msg = (char*) malloc(*size);

    // Copying contents to message
    memcpy(*msg, &code, sizeof(code));
    memcpy(*msg+sizeof(code), &bufsize, sizeof(bufsize));
    memcpy(*msg+sizeof(code)+sizeof(bufsize), buf, bufsize);
}

/*
Decodes a message with portion of file.
Returns buffer and size in passed variables
*/
void file_packet_decode(char* msg, int size, char** buf, int* bufsize){
    u_int16_t otherint;
    int bufsize_int;

    // Recover size of buffer
    memcpy(&bufsize_int, msg+sizeof(otherint), sizeof(bufsize_int));
    *bufsize = bufsize_int;

    // Recover buffer
    *buf = (char*) malloc(bufsize_int);
    memcpy(*buf, msg+sizeof(otherint)+sizeof(bufsize_int), *bufsize);
}

void send_file_to_server(int* socket_ptr, const char* filename){
    // Create buffer and set it to zero
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // Derreference socket pointer in order to use it
    int s = *socket_ptr;

    // Read from file
    FILE *ptr;
    ptr = fopen(filename,"rb");  // r for read, b for binary

    int end = 0;
    int nread;
    // Read BUFSZ blocks of 1 byte
    while( 0 < (nread = fread(buf, 1, BUFSZ, ptr))){
        // print_bytes(buf, BUFSZ);
        // printf("read: %i\n", nread);
        char* msg;
        size_t size;
        file_packet_encode(buf, nread, &msg, &size);
        size_t count = send(s, msg, size, 0);
        // printf("[log] sent: %li",count);
        if (count != size){ logexit("send");}
        memset(buf, 0, BUFSZ);
    }
    char* msg;
    size_t size;
    memcpy(buf, "DONE", sizeof("DONE"));
    file_packet_encode(buf, sizeof("DONE"), &msg, &size);
    size_t count = send(s, msg, size, 0);
    // printf("[log] sent: %li",count);
    if (count != size){ logexit("send");}
    memset(buf, 0, BUFSZ);
    fclose(ptr);

}

void recv_file(int* socket_ptr, const char* filename){
    printf("[log] Receiving file %s\n", filename);
    // Create buffer and set it to zero
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // Derreference socket pointer in order to use it
    int s = *socket_ptr;

    size_t count = recv(s, buf, BUFSZ, 0);

    // Read from file
    FILE *ptr;
    ptr = fopen(filename,"wb");  // r for read, b for binary
    if (ptr == NULL){ exit(1); }

    int end = 0;
    int nread;
    printf("[msg] %d bytes: %s\n", (int)count, buf);        

    // Read BUFSZ blocks of 1 byte
    while (strcmp(buf, "DONE") != 0){
        printf("In loop\n");

        size_t count = recv(s, buf, BUFSZ, 0);
        printf("[msg] %d bytes: %s\n", (int)count, buf);
        uint16_t code = msg_code(buf);
        printf("[msg] code: %i\n", code);
        if (strcmp(buf, "DONE") == 0 || (msg_code(buf) == 1 && count == 2)){
            printf("We must break\n");
            break;
        }


        nread = fwrite(buf, 1, count, ptr); 
        // printf("wrote: %d\n", nread);
        memset(buf, 0, BUFSZ);
    }
    printf("Closing now\n");
    fclose(ptr);


    // while( 0 < (nread = fread(buf, 1, BUFSZ, ptr))){
    //     print_bytes(buf, BUFSZ);
    //     printf("read: %i\n", nread);

    //     // size_t count = send(s, buf, BUFSZ, 0);
    //     // printf("[log] sent: %li",count);
    //     // if (count != strlen(buf)){ logexit("send");}
    //     memset(buf, 0, BUFSZ);
    // }

}

/*
Sends a message with code 1 through socket
*/
void last_mod_msg1_send(int* socket_ptr){
    // Creates msg1
    // Derreference socket pointer in order to use it
    int s = *socket_ptr;

    // Create buffer and set it to zero
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // Enconde message and copy it to buffer
    char *msg = NULL; 
    size_t size;
    last_mod_msg1_encode(&msg, &size);
    memcpy(buf, msg, strlen(msg));

    // Send it
    size_t count = send(s, buf, size, 0);
    printf("[log] sent: %li\n",count);
    if (count != strlen(buf)){ logexit("send");}
}

/*
Sends a message with code 3 through socket
*/
void hard_disk_req_msg3_send(int* socket_ptr){
    // Creates msg1
    // Derreference socket pointer in order to use it
    int s = *socket_ptr;

    // Create buffer and set it to zero
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // Enconde message and copy it to buffer
    char *msg = NULL; 
    size_t size;
    hard_disk_req_msg3_encode(&msg, &size);
    memcpy(buf, msg, strlen(msg));

    // Send it
    size_t count = send(s, buf, size, 0);
    printf("[log] sent: %li\n",count);
    if (count != strlen(buf)){ logexit("send");}
}

/*
Updates the file which holds file hierarchy, if needed.
The updated version will come from server
*/
void update_if_needed(int* socket_ptr){
    // Send msg1
    last_mod_msg1_send(socket_ptr);


    // Receive message
    char buf[BUFSZ];
    int s = *socket_ptr;
    memset(buf, 0, BUFSZ);
    size_t count = recv(s, buf, BUFSZ, 0);
    // print_bytes(buf, count);
    uint16_t code = msg_code(buf);
    printf("[msg] code: %i\n", code);
    if (code == 2){
        // Get date sent from server
        time_t server_last_mod_date = last_mod_msg2_decode(buf);
        printf("Last modified time (server): %s", ctime(&server_last_mod_date));
        
        // Get client last modified date
        time_t client_last_mod_date;

        if(access(CLIENT_MOD_FILENAME, F_OK ) == 0){
            // file exists
            client_last_mod_date = getDate(CLIENT_MOD_FILENAME);
            printf("Last modified time (client): %s", ctime(&client_last_mod_date));
        }else{
            // file doesn't exist
            // we'll need to update, so just make the variable smaller so
            // we are caught in next if clause 
            client_last_mod_date = server_last_mod_date - 1;
        }

        if (client_last_mod_date < server_last_mod_date){
            printf("We need to update\n");
            hard_disk_req_msg3_send(&s);
            recv_file(&s, CLIENT_HD_FILENAME);
            updateDate(CLIENT_MOD_FILENAME, server_last_mod_date);
        }else{
            printf("No update needed\n");
        }
    }else{
        printf("Couldn't get last modified date from server");
    }
}


/*
Creates a message telling the server that the client will send the updated 
harddisk to him.
This message has code 6.

Returns void.
Created message will be allocated in given 'msg' pointer.
Its size will be placed in given 'size' pointer.
*/
void hard_disk_send_msg6_encode(char** msg, size_t* size){
    // 2-byte integer: message code
    u_int16_t code = 6; 

    // Create byte-like message
    *size = sizeof(code);
    *msg = (char*) malloc(*size);

    // Copying contents to message
    memcpy(*msg, &code, sizeof(code));

    // print_bytes(*msg, *size);
}

/*
Executes protocol to send hard disk and update mod date in server
*/
void hard_disk_send(int* socket_ptr){
    // Creates msg1
    // Derreference socket pointer in order to use it
    int s = *socket_ptr;

    // Create buffer and set it to zero
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // Enconde message and copy it to buffer
    char *msg = NULL; 
    size_t size;
    hard_disk_send_msg6_encode(&msg, &size);
    memcpy(buf, msg, strlen(msg));

    // Send it
    size_t count = send(s, buf, size, 0);
    if (count != strlen(buf)){ logexit("send");}
    printf("[hdsend] Msg6 sent\n");

    // Send harddisk file after
    printf("[hdsend] Sending harddisk...\n");
    send_file(&s, CLIENT_HD_FILENAME);
    printf("[hdsend] Harddisk sent\n");

    // Send date 
    printf("[hdsend] Sending last modified date...\n");
    time_t date = time(NULL);
    updateDate(CLIENT_MOD_FILENAME, date);
    msg = NULL; 
    last_mod_msg7_encode(CLIENT_MOD_FILENAME, &msg, &size);
    // print_bytes(msg, size);
    memcpy(buf, msg, size);
    count = send(s, buf, size, 0);
    printf("[hdsend] Sent\n");
}

