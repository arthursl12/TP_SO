#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include "filedate.h"

/*
Puts given date into passed file
*/
void updateDate(const char* filename, time_t mytime){
    // Open file
    FILE *fptr;
    fptr = fopen(filename, "wb");

    // Allocate variables and get current time
    // time_t mytime = time(NULL);

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

void printDate(time_t mytime){
    char *time_str = ctime(&mytime);
    printf("Current Time : %s\n", time_str);
}

// int main(){
//     // updateDate("lastmod.date");
//     // getDate("lastmod.date");
//     time_t mytime = getDate("lastmod.date");
//     char *time_str = ctime(&mytime);
//     printf("Current Time : %s\n", time_str);
//     return 0;
// }