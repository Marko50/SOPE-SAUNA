#include "generator.h"

void printUsage(){
  printf("USAGE:\n./generator numRequests(int) maxRequestDuration(int)\n");
}

int main(int argc, char* argv[])
{
  if(argc != 3)
  {
    printUsage();
    return 1;
  }
  int numRequests = atoi(argv[1]);
  int maxRequestDuration = atoi(argv[2]);
  generator(numRequests, maxRequestDuration);
  return 0;
}
