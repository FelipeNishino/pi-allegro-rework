// #include <stdbool.h>
#include "queue.h"
#include <stdlib.h>

void queue_init(queue* q, int size) {
	q->start = 0;
	q->end = 0;
	q->size = 0;
	q->capacity = size;
    q->queue = (char*) malloc(size * sizeof(char));
}

bool queue_is_empty(queue* q) {
    return q->size == 0;
}

bool queue_is_full(queue* q) {
	return q->size >= q->capacity;
}

void queue_enqueue(queue* q, char val) {
	if (!queue_is_full(q)) {
		q->queue[q->end++] = val;
		q->size++;
		if (q->end >= q->capacity) {
			q->end = 0;
		}
	}
}

void queue_force_enqueue(queue* q, char val) {
	if (queue_is_full(q)) queue_dequeue(q);
	
	q->queue[q->end++] = val;
	q->size++;
	if (q->end >= q->capacity) {
		q->end = 0;
	}	
}

char queue_dequeue(queue* q) {
	char x = ' ';

	if (!queue_is_empty(q)) {
		x = q->queue[q->start++];
		q->size--;
		if (q->start >= q->capacity) {
			q->start = 0;
		}
	}
	return x;
}

char queue_peek(queue* q) {
	return q->queue[q->start];
}