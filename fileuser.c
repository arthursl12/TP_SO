#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

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
Puts current date into passed file
*/
void updateDate(const char* filename){
    // Open file
    FILE *fptr;
    fptr = fopen(filename, "wb");

    // Allocate variables and get current time
    time_t mytime = time(NULL);
    char *time_str = ctime(&mytime);

    // Convert to bytes
    char* dump = (char*) malloc(sizeof(mytime));
    memcpy(dump, &mytime, sizeof(mytime));
    
    // Save to file and close it 
    fwrite(dump, sizeof(mytime), 1, fptr);
    fclose(fptr);
}


/*
Get date stored in file
*/
time_t getDate(const char* filename){
    // Open file
    FILE *fptr;
    fptr = fopen(filename, "rb");

    // Allocate variables
    time_t mytime;
    char* buf = (char*) malloc(sizeof(mytime));

    // Get bytes from file and put into variable
    fread(buf , sizeof(mytime), 1, fptr);
    memcpy(&mytime, buf, sizeof(mytime));

    // Close file
    fclose(fptr);

    return mytime;
}


int main(){
    // updateDate("lastmod.date");
    // getDate("lastmod.date");
    time_t mytime = getDate("lastmod.date");
    char *time_str = ctime(&mytime);
    printf("Current Time : %s\n", time_str);
    return 0;
}