#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

#define NUM_OF_SLOTS 5

typedef struct result {
  struct result *prev, *next;
  int value;
} result_t;

semaphore_t mutex;
semaphore_t slot;
semaphore_t item;

task_t p1, p2, p3;
task_t c1, c2;

result_t *queue = NULL;

void add_to_queue(int value);
int get_from_queue();

void producer(void *arg) {
  for (;;) {
    task_sleep(1000);
    int value = random() % 100;

    sem_down(&slot);

    sem_down(&mutex);
    add_to_queue(value);
    sem_up(&mutex);

    sem_up(&item);

    printf("%s produced %d\n", (char *)arg, value);
  }
}

void consumer(void *arg) {
  for (;;) {
    sem_down(&item);

    sem_down(&mutex);
    int value = get_from_queue();
    sem_up(&mutex);

    sem_up(&slot);

    printf("                %s consumed %d\n", (char *)arg, value);
    task_sleep(1000);
  }
}

// The two functions below abstract the queue from the producer and consumer
void add_to_queue(int value) {
  result_t *r = (result_t *)malloc(sizeof(result_t));
  r->value = value;
  r->next = NULL;
  r->prev = NULL;
  return queue_append((queue_t **)&queue, (queue_t *)r);
}

int get_from_queue() {
  result_t *r = (result_t *)queue_remove((queue_t **)&queue, (queue_t *)queue);
  int value = r->value;
  free(r);
  return value;
}

int main() {
  ppos_init();

  sem_create(&mutex, 1);
  sem_create(&item, 0);
  sem_create(&slot, NUM_OF_SLOTS);

  task_create(&p1, producer, "P1");
  task_create(&p2, producer, "P2");
  task_create(&p3, producer, "P3");

  task_create(&c1, consumer, "C1");
  task_create(&c2, consumer, "C2");

  task_join(&p1);
}
