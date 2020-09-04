#ifndef __MAIN_H
#define __MAIN_H

#include <arduino.h>

void camCB(void* pvParameters);
void streamCB(void * pvParameters);
void handleJPGSstream(void);
void handleJPG(void);
void handleNotFound();
char* allocateMemory(char* aPtr, size_t aSize);
#endif