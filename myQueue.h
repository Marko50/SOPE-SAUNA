#include <limits.h>
#include <stdio.h>
#include "request.h"

#define MAX_PROC 50

struct myQueues{
  struct Requests queue[MAX_PROC];
  int firstMemberPointer;
  int lastMemberPointer;
  int itemCount;
}myQueue;

int isFull(struct myQueues* self) {
  if(self->itemCount == MAX_PROC)
    return 1;
  else return 0;
}

int addData(struct Requests req, struct myQueues* self){
  if(isFull(self) == 0) {
     if(self->lastMemberPointer == MAX_PROC-1) {
        self->lastMemberPointer= -1;
     }
     self->queue[++self->lastMemberPointer] = req;
     self->itemCount++;
     return 0;
  }

  else return 1;
}
struct Requests getFront(struct myQueues* self) {
   return self->queue[self->firstMemberPointer];
}

int isEmpty(struct myQueues* self) {
  if(self->itemCount == 0)
    return 1;
  else return 0;
}

int size(struct myQueues* self) {
   return self->itemCount;
}
int removeTop(struct myQueues* self){
  if(isEmpty(self) == 1)
  {
    return 1;
  }
  self->firstMemberPointer++;
  if(self->firstMemberPointer == MAX_PROC) {
     self->firstMemberPointer = 0;
  }
  self->itemCount--;
  if(self->itemCount < 0)
    self->itemCount = 0;
  return 0;
}
