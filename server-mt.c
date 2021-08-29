#include "packets.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define SERVER_MOD_FILENAME "lastmod_server.date"
#define SERVER_HD_FILENAME "harddisk_server.dat"


void usage(int argc, char **argv) {
    printf("usage: %s <server port>\n", argv[0]);
    printf("example: %s 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

struct client_data {
    int csock;
    struct sockaddr_storage storage;
};


void last_mod_msg2_send(int* socket_ptr, const char* filename){
    // Creates msg1
    // Derreference socket pointer in order to use it
    int s = *socket_ptr;

    // Create buffer and set it to zero
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // Enconde message and copy it to buffer
    char *msg = NULL; 
    size_t size;
    last_mod_msg2_encode(SERVER_MOD_FILENAME, &msg, &size);
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
            last_mod_msg2_send(&(cdata->csock), SERVER_MOD_FILENAME);

            // time_t client_last_date = last_mod_msg2_decode(buf);
            // printf("Last modified time (client): %s\n", ctime(&client_last_date));
            // printf("[msg] code: %i\n", code);
        }else if(code == 3){
            send_file(&(cdata->csock), SERVER_HD_FILENAME);
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

int main(int argc, char **argv){
    // Argument parsing and socket creation
    if (argc < 2) { usage(argc, argv); }
    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init("v4", argv[1], &storage)) {usage(argc, argv);}
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) { logexit("socket");}

    // Binding socket to listen to port
    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, 
                        &enable, sizeof(int))) { logexit("setsockopt"); }
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) { logexit("bind");}
    if (0 != listen(s, 10)) { logexit("listen");}
    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    // Harddisk creation if it doesn't exist    
    if(access(SERVER_HD_FILENAME, F_OK ) == 0){
        // file exists
    }else{
        // file doesn't exist, we'll create a blank one
        FILE *fptr;
	    fptr = fopen(SERVER_HD_FILENAME, "w");
        fclose(fptr);
    }

    // Loop to connect to clients
    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) { logexit("accept"); }

        struct client_data *cdata = malloc(sizeof(*cdata));
        if (!cdata) { logexit("malloc"); }
        cdata->csock = csock;
        memcpy(&(cdata->storage), &cstorage, sizeof(cstorage));

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata);
    }

    exit(EXIT_SUCCESS);
}
