#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>

typedef struct AMG_ANGLES {
    float yaw;
    float pitch;
    float roll;
} AMG_ANGLES;


void getFileCreationTime(char *path) {
    struct stat attr;
    stat(path, &attr);
    printf("Last modified time: %s", ctime(&attr.st_mtime));
}

int main(int argc, char* argv[]){
    // Teste 1
    AMG_ANGLES struct_data;

    struct_data.yaw = 87.96;
    struct_data.pitch = -114.58;
    struct_data.roll = 100.50;

    //Sending Side
    char b[sizeof(struct_data)];
    memcpy(b, &struct_data, sizeof(struct_data));

    //Receiving Side
    AMG_ANGLES tmp; //Re-make the struct
    memcpy(&tmp, b, sizeof(tmp));
    printf("%hhx\n", b[1]);
    printf("%.2f\n", tmp.yaw); //Display the yaw to see if it's correct


    // -----------------
    // Teste 2

    int i = 0;
    u_int8_t myint8 = 10; // int de 1byte
    char myintb[sizeof(myint8)];
    memcpy(myintb, &myint8, sizeof(myint8));
    for (i = sizeof(myint8)-1; i >= 0; i--){
        printf("%hhx ", myintb[i]);
    }
    printf("\\\n");
    
    // -----------------
    // Teste 3

    // Pegando last modified date do arquivo
    // Sending side
    u_int16_t myint = 10; // int de 2bytes
    struct stat attr;
    stat("makefile", &attr);
    size_t size = sizeof(myint)+sizeof(attr.st_mtime);
    char mymsgb[size];
    memcpy(mymsgb, &myint, sizeof(myint));
    memcpy(mymsgb+sizeof(myint), &attr.st_mtime, sizeof(attr.st_mtime));
    for (i = 0; i < size; i++){
        printf("%hhx ", mymsgb[i]);
    }
    printf("\\\n");

    // Receiving side
    u_int16_t otherint;
    memcpy(&otherint, mymsgb, sizeof(otherint));
    printf("%i\n",otherint);
    time_t time;
    memcpy(&time, mymsgb+sizeof(otherint), sizeof(time));
    printf("Last modified time: %s\n", ctime(&time));

    return 0;
}