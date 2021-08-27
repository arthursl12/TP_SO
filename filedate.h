#ifndef FILEDATE_H
#define FILEDATE_H

#include <time.h>

void updateDate(const char* filename, time_t date);
time_t getDate(const char* filename);
void printDate(time_t mytime);

#endif /* FILEDATE */