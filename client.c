#include "packets.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

#define BUFSZ 1024


void* send_msg_handler(void* data) {
    int s = *((int *) data);
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // Entrada da mensagem
    printf("message1> ");
    fflush(stdout);
    // fgets(buf, BUFSZ-1, stdin);
    char *msg = NULL; 
    size_t size;
    last_mod_msg_encode("makefile", &msg, &size);
    printf("[log] size: %li\n",size);
    print_bytes(msg, size);
    memcpy(buf, msg, strlen(msg));
    size_t count = send(s, buf, size, 0);
    printf("[log] sent: %li",count);
    if (count != strlen(buf)){ logexit("send");}
    memset(buf, 0, BUFSZ);

    while(1) {
        // Envia a mensagem
        size_t count = send(s, buf, strlen(buf), 0);
        if (count != strlen(buf)){ logexit("send");}

        // Entrada da mensagem
        printf("message3> ");
        fflush(stdout);
        memset(buf, 0, BUFSZ);
        fgets(buf, BUFSZ-1, stdin);
    }
    pthread_exit(EXIT_SUCCESS);
}

void* recv_msg_handler(void* data) {
    int s = *((int *) data);
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    unsigned total = 0;

    while (1) {
        int count = recv(s, buf, BUFSZ, 0);
        // puts("");
        if (count > 0) {
            // Imprime mensagem recebida do servidor
            puts(buf);

            // Imprime texto de input novamente
            printf("message2> ");
            fflush(stdout);
        }else if (count == 0){
            printf("\n");
            printf("[log] Servidor encerrou a conexão.\n");
            close(s);
            exit(EXIT_FAILURE);
            break;
        }else{
            // count == -1
            logexit("recv (client)");
        }
        total += count;
        memset(buf, 0, BUFSZ);
    }
    printf("[log] Recebeu um total de %i bytes\n", total);
    pthread_exit(EXIT_SUCCESS);
}


void last_mod_msg_send(int* socket_ptr, const char* filename){
    // Creates msg1
    // Derreference socket pointer in order to use it
    int s = *socket_ptr;

    // Create buffer and set it to zero
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);

    // Enconde message and copy it to buffer
    char *msg = NULL; 
    size_t size;
    last_mod_msg_encode(filename, &msg, &size);
    memcpy(buf, msg, strlen(msg));

    // Send it
    size_t count = send(s, buf, size, 0);
    printf("[log] sent: %li",count);
    if (count != strlen(buf)){ logexit("send");}
}

int main(int argc, char **argv) {
	if (argc < 3){
        usage(argc, argv);
    }

    struct sockaddr_storage storage;
    if (addrparse(argv[1], argv[2], &storage) != 0){
        usage(argc, argv);
    }

    // Conexão com o servidor
    int s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) { logexit("socket");}
    struct sockaddr* addr = (struct sockaddr*)(&storage);
    if (connect(s, addr, sizeof(storage)) != 0){ logexit("connect");}
    char addrstr[BUFSZ];       
    addrtostr(addr, addrstr, BUFSZ);        // Imprimir o IP do servidor
    printf("Sucessfully connected to %s.\n", addrstr);

    // Protocol test
    // Send msg1
    last_mod_msg_send(&s, "makefile");


    // Receive message
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    size_t count = recv(s, buf, BUFSZ, 0);

    last_mod_msg_send(&s, "makefile");
    memset(buf, 0, BUFSZ);
    count = recv(s, buf, BUFSZ, 0);








    // pthread_t recv_msg_thread;
    // int *arg = (int*) malloc(sizeof(*arg));
    // if (arg == NULL) { logexit("malloc");}
    // *arg = s;
    // if(pthread_create(&recv_msg_thread, NULL, recv_msg_handler, arg) != 0){
    //     logexit("pthread");
    // }

    // pthread_t send_msg_thread;
    // arg = (int*) malloc(sizeof(*arg));
    // if (arg == NULL) { logexit("malloc");}
    // *arg = s;

    // pthread_create(&send_msg_thread, NULL, send_msg_handler, arg);
    // // Esperar a thread de enviar mensagem terminar, para que o programa espere
    // // que usuário digite
    // (void)pthread_join(send_msg_thread, NULL); 

    // pthread_create(&send_msg_thread, NULL, double_send_msg_handler, arg);
    // // Esperar a thread de enviar mensagem terminar, para que o programa espere
    // // que usuário digite
    // (void)pthread_join(send_msg_thread, NULL); 
    return 0;
}