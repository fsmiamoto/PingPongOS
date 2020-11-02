#include "ppos.h"
#include "ppos_data.h"
#include "ppos_internal.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define check(x)                                                               \
  if (x)                                                                       \
    return -1;

/*
 * Semaphores
 */
int sem_down(semaphore_t *s) {
#ifdef DEBUG
  printf("Task %d called sem_down\n", current_task->id);
#endif
  if (s == NULL || s->is_destroyed)
    return -1;

  __enter_sem_cs(s);
  if (s->is_destroyed)
    return -1;

  s->value -= 1;

  __leave_sem_cs(s);

  if (s->value < 0) {
    __wait_in_semaphore_queue(s);
  }

  return 0;
}

int sem_create(semaphore_t *s, int value) {
  if (s == NULL)
    return -1;

  s->value = value;
  s->waiting = NULL;
  s->is_destroyed = 0;
  s->lock = 0;

  return 0;
}

int sem_destroy(semaphore_t *s) {
#ifdef DEBUG
  printf("Semaphore %p called to be destroyed\n", s);
#endif
  if (s == NULL || s->is_destroyed)
    return -1;

  s->is_destroyed = 1;

  __wake_up_all_waiting_tasks(s);

  return 0;
}

int sem_up(semaphore_t *s) {
#ifdef DEBUG
  printf("Task %d called sem_up\n", current_task->id);
#endif
  if (s == NULL || s->is_destroyed)
    return -1;

  __enter_sem_cs(s);
  if (s->is_destroyed)
    return -1;

  s->value += 1;
  if (s->waiting != NULL)
    __wake_up_first_waiting_task(s);

  __leave_sem_cs(s);

  return 0;
}

/*
 * Barrier
 */

int barrier_create(barrier_t *b, int N) {
  b->current_count = 0;
  b->expected_count = N;
  b->waiting = NULL;
  check(sem_create(&b->mutex, 1));
  return 0;
}

int barrier_join(barrier_t *b) {

  check(b == NULL);
  check(sem_down(&(b->mutex)));

  b->current_count += 1;

  if (b->current_count == b->expected_count) {
    __wake_up_barrier_tasks(b);
    check(sem_up(&(b->mutex)));
    return 0;
  }

  current_task->state = WAITING;
  queue_append((queue_t **)&(b->waiting), (queue_t *)current_task);
  check(sem_up(&(b->mutex)));

  task_yield();
  check(b == NULL);

  return 0;
}

int barrier_destroy(barrier_t *b) {
  check(b == NULL);
  b = NULL;
  return 0;
}

/*
 * Message Queues
 */
int mqueue_create(mqueue_t *queue, int max, int size) {
  queue->msg_size = size;
  queue->capacity = max;
  queue->length = 0;
  queue->is_destroyed = 0;
  queue->tail = queue->head = 0;

  check(sem_create(&queue->prod_sem, max));
  check(sem_create(&queue->cons_sem, 0));
  check(sem_create(&queue->mutex, 1));
  check((queue->buffer = malloc(max * size)) == NULL);

  return 0;
}

int mqueue_send(mqueue_t *queue, void *msg) {
  check(queue == NULL || queue->is_destroyed);

  check(sem_down(&queue->prod_sem));

  check(sem_down(&queue->mutex));
  memcpy(queue->buffer + (queue->msg_size * queue->head), msg, queue->msg_size);
  queue->head = (queue->head + 1) % queue->capacity;
  queue->length += 1;
  check(sem_up(&queue->mutex));

  check(sem_up(&queue->cons_sem));

  return 0;
}

int mqueue_recv(mqueue_t *queue, void *msg) {
  check(queue == NULL || queue->is_destroyed);

  check(sem_down(&queue->cons_sem));

  check(sem_down(&queue->mutex));
  memcpy(msg, queue->buffer + (queue->msg_size * queue->tail), queue->msg_size);
  queue->tail = (queue->tail + 1) % queue->capacity;
  queue->length -= 1;
  check(sem_up(&queue->mutex));

  check(sem_up(&queue->prod_sem));

  return 0;
}

int mqueue_destroy(mqueue_t *queue) {
  queue->is_destroyed = 1;
  check(sem_destroy(&queue->mutex));
  check(sem_destroy(&queue->prod_sem));
  check(sem_destroy(&queue->cons_sem));

  return 0;
}

int mqueue_msgs(mqueue_t *queue) {
  check(queue == NULL || queue->is_destroyed);
  return queue->length;
}
