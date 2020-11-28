#include "ppos_internal.h"
#include "ppos.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

void __wake_up_first_waiting_task(semaphore_t *s) {
  queue_append((queue_t **)&queues[READY],
               queue_remove((queue_t **)&s->waiting, (queue_t *)s->waiting));
}

void __wake_up_first__task(queue_t *q) {
  queue_append((queue_t **)&queues[READY],
               queue_remove((queue_t **)&q, (queue_t *)q));
};

void __move_to_ready_queue(task_t *queue) {
  int num_of_tasks = queue_size((queue_t *)queue);

  if (num_of_tasks == 0)
    return;

  task_t *task = queue;
  for (int i = 0; i < num_of_tasks; i++) {
    task_t *next = task->next;
    queue_remove((queue_t **)&queue, (queue_t *)task);
    queue_append((queue_t **)&queues[READY], (queue_t *)task);
    task = next;
  }
}

// Return the task with the highest priority
void *__highest_prio_task(void *prev, void *next) {
  if (prev == NULL)
    return next;

  task_t *prev_task = (task_t *)prev;
  task_t *next_task = (task_t *)next;

  if (next_task->prio_d < prev_task->prio_d) {
    return next;
  }

  return prev;
}

// Apply the aging factor on tasks
void __apply_aging(void *ptr) {
  task_t *task = (task_t *)ptr;
  task->prio_d -= SCHEDULER_AGING_ALPHA;
}

void __set_up_and_queue_main_task() {
  getcontext(&(main_task.context));

  main_task.id = 0;
  main_task.next = NULL;
  main_task.prev = NULL;
  main_task.start_tick = systime();
  main_task.activations = 0;
  main_task.prio = 0;
  main_task.prio_d = 0;
  main_task.preemptible = 1;
  main_task.state = READY;

  queue_append((queue_t **)&queues[READY], (queue_t *)&main_task);
}

void __enter_sem_cs(semaphore_t *s) {
#ifdef DEBUG
  printf("Task %d is waiting\n", current_task->id);
#endif
  // Busy waiting
  while (__sync_fetch_and_or(&(s->lock), 1))
    ;
#ifdef DEBUG
  printf("Task %d got the lock\n", current_task->id);
#endif
}

void __leave_sem_cs(semaphore_t *s) {
#ifdef DEBUG
  printf("Task %d released the lock \n", current_task->id);
#endif
  s->lock = 0;
}

void __create_dispatcher_task() {
  task_create(&dispatcher_task, (void *)dispatcher, NULL);
  dispatcher_task.preemptible = 0;
  dispatcher_task.tick_count = 0;
  dispatcher_task.activations = 0;
  dispatcher_task.start_tick = systime();
}

void __timer_tick_handler() {
  system_ticks_count++;
  current_task->tick_count++;

  if (!current_task->preemptible)
    return;

  current_task->tick_budget -= 1;

  if (current_task->tick_budget == 0) {
    task_yield();
  }
}

void __set_up_timer() {
  timer.it_value.tv_usec = 1000; // First tick, in micro-seconds
  timer.it_value.tv_sec = 0;     // in seconds

  timer.it_interval.tv_usec = 1000; // Following ticks
  timer.it_interval.tv_sec = 0;

  // man setitimer
  if (setitimer(ITIMER_REAL, &timer, 0) < 0) {
    perror("setitimer");
    exit(1);
  }
}

void __set_up_signals() {
  sig.sa_handler = __timer_tick_handler;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;
  if (sigaction(SIGALRM, &sig, 0) < 0) {
    perror("sigaction");
    exit(1);
  }
}

unsigned int __queue_up_tasks_that_should_wake_up() {
  int sleeping_tasks;
  if ((sleeping_tasks = queue_size((queue_t *)queues[SLEEPING])) <= 0) {
    return 0;
  }

  int still_sleeping = 0;

  task_t *task = queues[SLEEPING];
  for (int i = 0; i < sleeping_tasks; i++) {
    task_t *next = task->next;
    if (task->should_wakeup_at <= systime()) {
      queue_remove((queue_t **)&queues[SLEEPING], (queue_t *)task);
      queue_append((queue_t **)&queues[READY], (queue_t *)task);
    } else {
      still_sleeping += 1;
    }
    task = next;
  }

  return still_sleeping;
}

unsigned short __is_in_another_queue(task_t *t) {
  if (t->next == NULL && t->prev == NULL)
    return 0;
  return 1;
}

void __wait_in_semaphore_queue(semaphore_t *s) {
  current_task->state = WAITING;
  queue_append((queue_t **)&s->waiting, (queue_t *)current_task);
  task_yield();
}
