#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <math.h>

int generator( int numRequests, int duration);
void* readRejected(void* arg);
void* threadGenerator(void* arg);
int writeFile(int fd, pid_t processId, int requestID, char* requestGender, int requestDuration, char* type);
