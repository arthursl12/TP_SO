
#include "serverfs.h"
#include "packets.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define SERVER_FILENAME "lastmod_server.date"

void usage(int argc, char **argv) {
    printf("usage: %s <server port>\n", argv[0]);
    printf("example: %s 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void last_mod_msg2_send(int* socket_ptr, const char* filename){
    // Creates msg1
    // Derreference socket pointer in order to use it
    int s = *socket_ptr;
    printf("Aqui0\n");

    // Create buffer and set it to zero
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // Enconde message and copy it to buffer
    char *msg = NULL; 
    size_t size;
    printf("Aqui\n");
    last_mod_msg2_encode(SERVER_FILENAME, &msg, &size);
    print_bytes(msg, size);
    memcpy(buf, msg, size);

    // Send it
    size_t count = send(s, buf, size, 0);
    printf("[log] sent: %li",count);
    if (count != strlen(buf)){ logexit("send");}
}

void* client_thread(void *data) {
    struct client_data *cdata = (struct client_data *)data;
    struct sockaddr *caddr = (struct sockaddr *)(&cdata->storage);

    char caddrstr[BUFSZ];
    addrtostr(caddr, caddrstr, BUFSZ);
    printf("[log] connection from %s\n", caddrstr);

    while(1){
        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        size_t count = recv(cdata->csock, buf, BUFSZ - 1, 0);
        printf("[msg] %s, %d bytes: %s\n", caddrstr, (int)count, buf);
        uint16_t code = msg_code(buf);
        printf("[msg] code: %i\n", code);
        if (code == 1){
            last_mod_msg2_send(&(cdata->csock), SERVER_FILENAME);

            // time_t client_last_date = last_mod_msg2_decode(buf);
            // printf("Last modified time (client): %s\n", ctime(&client_last_date));
            // printf("[msg] code: %i\n", code);
        }else if(code == 3){
            send_file(&(cdata->csock), "file_server.txt");
        }


        sprintf(buf, "remote endpoint: %.1000s\n", caddrstr);
        count = send(cdata->csock, buf, strlen(buf) + 1, 0);
        if (count != strlen(buf) + 1) {
            logexit("send");
        }
    }
    close(cdata->csock);
    pthread_exit(EXIT_SUCCESS);
}
