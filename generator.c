#include "myQueue.h"
#include "generator.h"

#define REJECTED_MAX_SIZE 50

int requests;

struct generations {
        int seriesNum;
        int dur;
} generation;

int fdRejected;
int fdEntrada;

int generatedRequests = 0;
int femRequests = 0;
int maleRequests = 0;

int generatedReject = 0;
int femRejct = 0;
int maleRejct = 0;

int generatedRejectIgnored = 0;
int femRejctIgnored = 0;
int maleRejectIgnored = 0;

int messageFile;
clock_t begin;

int writeFile(int fd, pid_t processID, int requestID, char* requestGender, int requestDuration, char* type){
        unsigned long characters = 15;
        clock_t end = clock();
        double time_spent = ((double)(end - begin) / CLOCKS_PER_SEC)*1000; //miliseconds
        char pID[100];
        char rID[100];
        char rDur[100];
        char timeSpent[100];
        characters += sprintf(pID, "%i", processID);
        characters += sprintf(rID, "%i", requestID);
        characters += sprintf(rDur, "%d", requestDuration);
        characters += sprintf(timeSpent, "%.2f", time_spent);
        char final[50] = "";
        strcat(final,timeSpent);
        strcat(final, " - ");
        strcat(final, pID);
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


void* readRejected(void* arg)
{
        struct Requests* rejected = (struct Requests*) malloc(sizeof(struct Requests));
        int bytesRead;
        int actualPid = getpid();
        bytesRead = read(fdRejected,rejected,sizeof(struct Requests));
        if(bytesRead == -1)
        {
                fprintf(stderr, "Error on read in readRejected function!\n");
                exit(1);
        }

        if(rejected->numRejections < 3 && rejected->accepted == 0)
        {
                generatedReject++;
                writeFile(messageFile, actualPid, rejected->requestID, &(rejected->gender), rejected->requestTimeDuration, "REJEITADO");
                if(rejected->gender == 'M')
                {
                        maleRejct++;
                        maleRequests++;
                }
                else if(rejected->gender == 'F')
                {
                        femRejct++;
                        femRequests++;
                }
                writeFile(messageFile, actualPid, rejected->requestID, &(rejected->gender), rejected->requestTimeDuration, "PEDIDO");
                generatedRequests++;
                printf("Request number %lu, with gender %c, will now be added again to the queue\n", rejected->requestID, rejected->gender);
                addData(*rejected, (struct myQueues*) arg);
        }
        else if(rejected->numRejections >= 3 && rejected->accepted == 0)
        {
                writeFile(messageFile, actualPid, rejected->requestID, &(rejected->gender), rejected->requestTimeDuration, "REJEITADO");
                generatedReject++;
                writeFile(messageFile, actualPid, rejected->requestID, &(rejected->gender), rejected->requestTimeDuration, "DESCARTADO");
                printf("Request number %lu, with gender %c, will now be ignored\n", rejected->requestID, rejected->gender);
                generatedRejectIgnored++;
                if(rejected->gender == 'M')
                {
                        maleRejct++;
                        maleRejectIgnored++;
                }
                else if(rejected->gender == 'F')
                {
                        femRejct++;
                        femRejctIgnored++;
                }
                requests--;
        }
        else{
                printf("Request number %lu, with gender %c, has been accepted\n", rejected->requestID, rejected->gender);
                requests--;
        }
        pthread_exit(NULL);
}

void* threadGenerator(void* arg){
        struct Requests* newRequest = (struct Requests*) malloc(sizeof(struct Requests));
        newRequest->requestID = ((struct generations*) arg)->seriesNum;
        int random = rand() % 2; // random number generator between 0 and 1
        if(random == 0)
        {
                printf("This request is male.\n");
                newRequest->gender = 'M';
        }
        else if(random == 1) {
                printf("This request is female.\n");
                newRequest->gender = 'F';
        }
        newRequest->numRejections = 0;
        newRequest->requestTimeDuration = (rand() % ((struct generations*) arg)->dur + 1);
        newRequest->accepted = 0;
        pthread_exit(newRequest);
}

int generator( int numRequests, int duration){
        begin = clock();
        requests = numRequests;
        printf("Begining to generate random requests.\n");
        do
        {
                printf("Trying to open Fifo entrada!\n");
                fdEntrada=open("/tmp/entrada",O_WRONLY);
                if (fdEntrada==-1) sleep(1);
        }
        while (fdEntrada==-1);
        printf("Fifo entrada has been open!\n");
        do
        {
                printf("Trying to open Fifo rejeitados!\n");
                fdRejected=open("/tmp/rejeitados",O_RDONLY);
                if (fdRejected==-1) sleep(1);
        }
        while (fdRejected == -1);
        printf("Fifo rejeitados has been open!\n");
        int actualPid = getpid();
        char* aux2 = (char*) malloc(sizeof(int));
        sprintf(aux2,"%d", actualPid);
        char* temp = "/tmp/ger.";
        char* pathname = (char*) malloc(50);
        strcat(pathname, temp);
        strcat(pathname, aux2);
        free(aux2);
        messageFile = open(pathname, O_WRONLY|O_CREAT,S_IRUSR|S_IWUSR);
        if(messageFile == -1)
        {
                fprintf(stderr, "Failed to create sauna message FILE!\n");
                exit(1);
        }
        free(pathname);
        struct myQueues* queue = (struct myQueues*) malloc(sizeof(struct myQueues));
        queue->firstMemberPointer = 0;
        queue->itemCount = 0;
        queue->lastMemberPointer = -1;
        srand(time(NULL));
        int i = 0;
        struct generations aux;
        pthread_t threadGeneratorId;
        pthread_t threadGeneratorId2;
        aux.dur = duration;
        while( i < numRequests) {
                printf("Processing request number %d\n", i);
                aux.seriesNum = i;
                if(pthread_create(&threadGeneratorId, NULL, threadGenerator, &aux) != 0) {
                        fprintf(stderr, "pthread_create genereatorThread error!\n");
                        exit(1);
                }
                void* auxiliar = (struct Requests*) malloc(sizeof(struct Requests));
                if(pthread_join(threadGeneratorId, &auxiliar) != 0) {
                        fprintf(stderr, "pthread_join genereatorThread error!\n");
                        exit(1);
                }
                writeFile(messageFile, actualPid, ((struct Requests*)auxiliar)->requestID, &(((struct Requests*)auxiliar)->gender), ((struct Requests*)auxiliar)->requestTimeDuration, "PEDIDO");
                addData(*(struct Requests*)auxiliar,queue);
                generatedRequests++;
                if(((struct Requests*)auxiliar)->gender == 'M') {
                        maleRequests++;
                }
                else if(((struct Requests*)auxiliar)->gender == 'F')
                {
                        femRequests++;
                }
                i++;
        }

        while(requests > 0)
        {
                while (isEmpty(queue) == 0) {
                        printf("Adding to queue.....\n");
                        if(write(fdEntrada, &(queue->queue[queue->firstMemberPointer]), sizeof(struct Requests)) == -1) {
                                fprintf(stderr, "Write on fifo entrada  error!\n");
                                exit(1);
                        }
                        removeTop(queue);
                }
                if(pthread_create(&threadGeneratorId2, NULL, readRejected, queue) != 0)
                {
                        fprintf(stderr, "pthread_create readRejected thread error!\n");
                        exit(1);
                }
                if(pthread_join(threadGeneratorId2, NULL) != 0) {
                        fprintf(stderr, "pthread_join readRejected error!\n");
                        exit(1);
                }
                printf("\n");
        }

        printf("TOTAL GENERATED REQUESTS: %d\n", generatedRequests);
        printf("MALE REQUESTS: %d\n", maleRequests);
        printf("FEMALE REQUESTS: %d\n", femRequests);
        printf("TOTAL REJECTED REQUESTS RECEIVED: %d\n", generatedReject);
        printf("MALE REJECTIONS: %d\n", maleRejct);
        printf("FEMALE REJECTIONS: %d\n", femRejct);
        printf("TOTAL REJECTED REQUESTS IGNORED: %d\n", generatedRejectIgnored);
        printf("FEMALE REJECTED IGNORED: %d\n", femRejctIgnored);
        printf("MALE REJECT IGNORED: %d\n", maleRejectIgnored);
        close(fdEntrada);
        close(fdRejected);
        close(messageFile);

        return 0;
}
