#ifndef queue_h
#define queue_h

#include <Arduino.h>

typedef struct queueHeaderNode *queueHeader;

queueHeader initializeQueue();
void enQueue(queueHeader q, byte x, byte y);
bool dequeue(queueHeader);
void printQueue(queueHeader);
void freeQueue(queueHeader);
bool isInTheQueue(queueHeader, byte);
void GetQueueHead(queueHeader, byte *);

#endif