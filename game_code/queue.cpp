#include "queue.h"
#include <Arduino.h>

typedef struct queueNode *queue;

struct queueNode {
    byte x;
    byte y;
    queue next;
};

struct queueHeaderNode {
    queue head;
    queue tail;
};

queueHeader initializeQueue() {
    queueHeader q = (queueHeader)malloc(sizeof(struct queueHeaderNode));
    if (!q) return NULL;
    q->head = q->tail = NULL;
    return q;
}

void enQueue(queueHeader q, byte x, byte y) {
    queue newq = (queue)malloc(sizeof(struct queueNode));
    if (!newq) return;
    
    newq->next = NULL;
    newq->x = x;
    newq->y = y;

    if (!q->head) {
        q->head = q->tail = newq;
    } else {
        q->tail->next = newq;
        q->tail = newq;
    }
}

bool dequeue(queueHeader q) {
    if (!q->head) return false;

    queue temp = q->head;
    q->head = q->head->next;
    if (!q->head) q->tail = NULL;  // Update tail when the last element is removed

    free(temp);
    return true;
}

void printQueue(queueHeader q) {
    queue temp = q->head;
    while (temp) {
        Serial.print("( ");
        Serial.print(temp->x);
        Serial.print(", ");
        Serial.print(temp->y);
        Serial.print(" ) - ");
        temp = temp->next;
    }
    Serial.println();
}

void freeQueue(queueHeader q) {
    queue temp = q->head;
    while (temp) {
        queue temp2 = temp;
        temp = temp->next;
        free(temp2);
    }
    q->head = q->tail = NULL;
}

bool isInTheQueue(queueHeader q, byte x) {
    queue temp = q->head;
    while (temp) {
        if (temp->x == x) return true;
        temp = temp->next;
    }
    return false;
}

void GetQueueHead(queueHeader q, byte *head) {
  byte x = 255, y = 255;
  if (q->head)
  {
    x = q->head->x;
    y = q->head->y;
  }

  head[0] = x; head[1] = y;
}
