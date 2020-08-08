#include "ppos_internal.h"
#include "ppos.h"
#include "ppos_data.h"
#include "queue.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

// Task queues for each state:
// Created, Ready, Running, Waiting, Terminated
task_t *queues[] = {NULL, NULL, NULL, NULL, NULL};

struct sigaction action;
struct itimerval timer;

task_t *scheduler() {
  if (queue_size((queue_t *)queues[READY]) == 0) {
    return NULL;
  }

  task_t *chosen = (task_t *)queue_reduce((queue_t *)queues[READY], NULL,
                                          __highest_prio_task);

  queue_foreach((queue_t *)queues[READY], __apply_aging);

  chosen->prio_d = chosen->prio;

  return chosen;
}

void dispatcher() {
  while (1) {
    task_t *next = scheduler();

    if (next == NULL) {
      break;
    }

    queue_remove((queue_t **)&queues[READY], (queue_t *)next);

    next->state = RUNNING;
    next->tick_budget = 20;
    next->activations += 1;
    task_switch(next);

    queue_append((queue_t **)&queues[next->state], (queue_t *)next);
  }

  task_exit(0);
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

void __timer_tick_handler() {
  system_ticks_count++;
  current_task->tick_count++;

  if (current_task->is_system_task)
    return;

  if (current_task->tick_budget == 0) {
    task_yield();
  }

  current_task->tick_budget -= 1;
}

void __set_up_signals() {
  action.sa_handler = __timer_tick_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  if (sigaction(SIGALRM, &action, 0) < 0) {
    perror("sigaction: ");
    exit(1);
  }
}

void __set_up_timer() {
  timer.it_value.tv_usec = 1000; // First tick, in micro-seconds
  timer.it_value.tv_sec = 0;     // in seconds

  timer.it_interval.tv_usec = 1000; // Following ticks
  timer.it_interval.tv_sec = 0;

  // man setitimer
  if (setitimer(ITIMER_REAL, &timer, 0) < 0) {
    perror("setitimer: ");
    exit(1);
  }
}

void __set_up_main_task() {
  main_task.id = 0;
  getcontext(&(main_task.context));
  main_task.next = NULL;
  main_task.prev = NULL;
}

void __create_dispatcher_task() {
  task_create(&dispatcher_task, (void *)dispatcher, NULL);
  dispatcher_task.is_system_task = 1;
}
