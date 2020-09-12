#include "ppos.h"
#include "ppos_data.h"
#include "ppos_internal.h"

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
    current_task->state = WAITING;
    queue_append((queue_t **)&s->waiting, (queue_t *)current_task);
    task_yield();
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
