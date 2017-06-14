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

void* saunaSimulator(void* arg);
int sauna(int numSeats);
int writeFile(int fd, pid_t processId, pthread_t  threadID, int requestID, char* requestGender, int requestDuration, char* type);
