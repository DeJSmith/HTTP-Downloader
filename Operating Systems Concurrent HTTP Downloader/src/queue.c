#include "queue.h"
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <assert.h>

#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

#define handle_error(msg) \
        do { perror(msg); exit(EXIT_FAILURE); } while (0)



typedef struct QueueStruct {

	int head, tail, length;
	unsigned capacity;
	void **array;
	sem_t put;
	sem_t get;
	pthread_mutex_t lock;
	
} Queue;


Queue *queue_alloc(int size) {

	printf("creating queue\n");
    
	Queue *queue = (Queue*) malloc(sizeof(Queue));
	queue->capacity = size;
	queue->head = queue->length = 0;
	queue->tail = (size - 1);
	queue->array = (void *) malloc(queue->capacity * sizeof(void**));
	sem_init(&queue->put, 0, size);
	sem_init(&queue->get, 0, 0);
	pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
	queue->lock = lock;
	
	return queue;
}

void queue_free(Queue *queue) {

    free(queue->array);
    free(queue);
}

void queue_put(Queue *queue, void *item) {

    
	sem_wait(&queue->put);
	
	pthread_mutex_lock(&queue->lock);

	queue->tail = (queue->tail + 1)%queue->capacity;
	queue->array[queue->tail] = item;
	queue->length = queue->length + 1;

	pthread_mutex_unlock(&queue->lock);

	sem_post(&queue->get);
	

}
void *queue_get(Queue *queue) {


	sem_wait(&queue->get);
	
	pthread_mutex_lock(&queue->lock);
	
	void *item = queue->array[queue->head];
	queue->head = (queue->head + 1)%queue->capacity;
	queue->length = queue->length - 1;
	
	pthread_mutex_unlock(&queue->lock);
	
	sem_post(&queue->put);

	
	return item;

}
