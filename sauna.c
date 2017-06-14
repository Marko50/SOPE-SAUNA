#include "myQueue.h"
#include "sauna.h"


#define generalGender 'G'
#define wait_time_between_requests 3

pthread_mutex_t mut=PTHREAD_MUTEX_INITIALIZER;
int emptySeats; //Critic section
char CurrentGender; //Critic section

int numThreads = 0;

int readRequests = 0;
int femRequests = 0;
int maleRequests = 0;

int rejected = 0;
int femRejct = 0;
int maleRejct = 0;

int serv = 0;
int femServ = 0;
int maleServ = 0;

clock_t begin;

int writeFile(int fd, pid_t processID, pthread_t threadID, int requestID, char* requestGender, int requestDuration, char* type){
        unsigned long characters = 18;
        clock_t end = clock();
        double time_spent = ((double)(end - begin) / CLOCKS_PER_SEC)*1000; //miliseconds
        char pID[100];
        char tID[100];
        char rID[100];
        char rDur[100];
        char timeSpent[100];
        characters += sprintf(pID, "%i", processID);
        characters += sprintf(tID, "%lu", threadID);
        characters += sprintf(rID, "%i", requestID);
        characters += sprintf(rDur, "%d", requestDuration);
        characters += sprintf(timeSpent, "%.2f", time_spent);
        char final[50] = "";
        strcat(final,timeSpent);
        strcat(final, " - ");
        strcat(final, pID);
        strcat(final, " - ");
        strcat(final, tID);
        strcat(final, " - ");
        strcat(final, rID);
        strcat(final, ": ");
        strcat(final, requestGender);
        strcat(final, " - ");
        strcat(final, rDur);
        strcat(final, " - ");
        strcat(final, type);
        strcat(final,"\n");
        characters+= strlen(type) + 1;
        if(write(fd, final, characters) == -1)
        {
                return 1;
        }
        return 0;
}

void* saunaSimulator(void* arg){
        struct Requests* aux = ((struct Requests*) arg);
        sleep(ceil((aux->requestTimeDuration)/1000) + 1);
        pthread_mutex_lock(&mut);
        emptySeats++;
        numThreads--;
        pthread_mutex_unlock(&mut);
        pthread_exit(aux);
}

int sauna(int numSeats)
{
        begin = clock();
        pthread_t tid;
        printf("Sauna has been open for %d people.\n", numSeats);
        int bytesRead;
        emptySeats = numSeats;
        int fd;
        int messageFile;
        int actualPid = getpid();
        char* aux = (char*) malloc(sizeof(int));
        sprintf(aux,"%d", actualPid);
        char* temp = "/tmp/bal.";
        char* pathname = (char*) malloc(50);
        strcat(pathname, temp);
        strcat(pathname, aux);
        free(aux);
        messageFile = open(pathname, O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
        if(messageFile == -1)
        {
                fprintf(stderr, "Failed to create sauna message FILE!\n");
                exit(1);
        }
        free(pathname);
        do
        {
                printf("Trying to open Fifo entrada!\n");
                fd=open("/tmp/entrada",O_RDONLY);
                if (fd==-1) {
                        printf("Failing to open fifo entrada!\n");
                        sleep(1);
                }
        }
        while (fd==-1);
        printf("Fifo entrada has been open!\n");
        int fd2;
        do
        {
                printf("Trying to open Fifo rejeitados!\n");
                fd2=open("/tmp/rejeitados",O_WRONLY);
                if (fd2==-1) {
                        printf("Failing to open fifo rejeitados!\n");
                        sleep(1);
                }
        }
        while (fd2==-1);
        printf("Fifo rejeitados has been open!\n");
        struct Requests* saunaHolderAuxiliar;
        while(1)
        {
                saunaHolderAuxiliar = (struct Requests*) malloc(sizeof(struct Requests));
                printf("Reading requests: \n");
                bytesRead = read(fd, saunaHolderAuxiliar, sizeof(struct Requests));
                if(bytesRead == -1) {
                        fprintf(stderr, "Read to saunaHolder error!\n");
                        exit(1);
                }
                if(bytesRead <= 0)
                {
                        break;
                }
                readRequests++;
                writeFile(messageFile, actualPid, actualPid,saunaHolderAuxiliar->requestID, &(saunaHolderAuxiliar->gender), saunaHolderAuxiliar->requestTimeDuration, "RECEBIDO");
                if(emptySeats == numSeats)
                {
                        CurrentGender = generalGender;
                }
                printf("Reading some random request for the sauna:%lu......%c\n", saunaHolderAuxiliar->requestID, saunaHolderAuxiliar->gender);
                if((CurrentGender == saunaHolderAuxiliar->gender || CurrentGender == generalGender) && emptySeats > 0) {
                        serv++;
                        if(saunaHolderAuxiliar->gender == 'M') {
                                maleRequests++;
                                maleServ++;
                        }
                        else if(saunaHolderAuxiliar->gender == 'F') {
                                femRequests++;
                                femServ++;
                        }
                        pthread_mutex_lock(&mut);
                        CurrentGender = saunaHolderAuxiliar->gender;
                        emptySeats--;
                        pthread_mutex_unlock(&mut);
                        printf("This request has been acepted! ID: %lu, Gender: %c\n",saunaHolderAuxiliar->requestID, saunaHolderAuxiliar->gender);
                        printf("CurrentGender: %c, emptySeats: %d\n", CurrentGender, emptySeats);
                        if(pthread_create(&tid, NULL, saunaSimulator, saunaHolderAuxiliar) != 0)
                        {
                                fprintf(stderr, "pthread_create saunaSimulator thread error!\n");
                                exit(1);
                        }
                        numThreads++;
                        saunaHolderAuxiliar->accepted = 1;
                        if(write(fd2, saunaHolderAuxiliar, bytesRead) == -1) {
                                fprintf(stderr, "Write on fifo accepted error!\n");
                                exit(1);
                        }
                        writeFile(messageFile, actualPid, tid,saunaHolderAuxiliar->requestID, &(saunaHolderAuxiliar->gender), saunaHolderAuxiliar->requestTimeDuration, "SERVIDO");
                }
                else{
                        rejected++;
                        if(saunaHolderAuxiliar->gender == 'M') {
                                maleRequests++;
                                maleRejct++;
                        }
                        else if(saunaHolderAuxiliar->gender == 'F') {
                                femRequests++;
                                femRejct++;
                        }
                        printf("This request has been denied!ID: %lu, Gender: %c\n",saunaHolderAuxiliar->requestID, saunaHolderAuxiliar->gender);
                        printf("CurrentGender: %c, emptySeats: %d\n", CurrentGender, emptySeats);
                        saunaHolderAuxiliar->accepted = 0;
                        saunaHolderAuxiliar->numRejections++;
                        if(write(fd2, saunaHolderAuxiliar, bytesRead) == -1) {
                                fprintf(stderr, "Write on fifo rejected error!\n");
                                exit(1);
                        }
                        sleep(wait_time_between_requests);
                        writeFile(messageFile, actualPid, actualPid,saunaHolderAuxiliar->requestID, &(saunaHolderAuxiliar->gender), saunaHolderAuxiliar->requestTimeDuration, "REJEITADO");
                }
                free(saunaHolderAuxiliar);
        }
        int i = 0;
// Waiting for other threads
//  sleep(30);//try and do in some other way
        while(i < numThreads) {
                sleep(1);
        }
        printf("Sauna will now terminate!\n");
        printf("TOTAL READ REQUESTS: %d\n", readRequests);
        printf("MALE REQUESTS: %d\n", maleRequests);
        printf("FEMALE REQUESTS: %d\n", femRequests);
        printf("TOTAL SERVED REQUESTS: %d\n", serv);
        printf("MALE SERVED: %d\n", maleServ);
        printf("FEMALE SERVED: %d\n", femServ);
        printf("TOTAL REJECTED REQUESTS: %d\n", rejected);
        printf("MALE REJECTIONS: %d\n", maleRejct);
        printf("FEMALE REJECTIONS: %d\n", femRejct);
        close(messageFile);
        close(fd);
        close(fd2);
        return 0;
}
