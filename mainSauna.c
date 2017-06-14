#include "sauna.h"

void printUsage(){
        printf("USAGE\n./sauna saunaSpace(int)\n");
}

int main(int argc, char* argv[])
{
        if(argc != 2)
        {
                printUsage();
                return 1;
        }
        int saunaSpace = atoi(argv[1]);
        if(mkfifo("/tmp/entrada", 0660) == -1)
        {
                fprintf(stderr, "mkfifo /tmp/entrada error!\n");
                exit(1);
        }
        else printf("FIFO entrada created!\n");
        if(mkfifo("/tmp/rejeitados", 0660) == -1)
        {
                fprintf(stderr, "mkfifo /tmp/rejeitados error!\n");
                exit(1);
        }
        else printf("FIFO rejeitados created!\n");
        sauna(saunaSpace);
        unlink("/tmp/entrada");
        unlink("/tmp/rejeitados");
        return 0;
}
