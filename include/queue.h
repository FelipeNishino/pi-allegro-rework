#ifndef QUEUE_H
#define QUEUE_H

#include <stdbool.h>

typedef struct queue_t {
	int size, start, end, capacity;
	char* queue;
} queue;

void queue_init(queue* q, int size);
bool queue_is_empty(queue* q);
bool queue_is_full(queue* q);
void queue_enqueue(queue* q, char val);
char queue_dequeue(queue* q);
char queue_peek(queue* q);

#endif //QUEUE_H

