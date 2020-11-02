#include "ppos.h"
#include "ppos_data.h"
#include "ppos_internal.h"
#include "queue.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

int next_task_id = 1; // IDs for other tasks start at 1
unsigned int system_ticks_count = 0;

task_t main_task;
task_t dispatcher_task;
task_t *current_task;

// Task queues for each state:
// Ready, Waiting, Sleeping, Terminated
task_t *queues[] = {NULL, NULL, NULL, NULL};

struct sigaction action;
struct itimerval timer;

void ppos_init() {
  // Deactivate the stdout buffer used by the printf function
  setvbuf(stdout, 0, _IONBF, 0);

  __set_up_signals();
  __set_up_timer();
  __set_up_and_queue_main_task();
  __create_dispatcher_task();

  current_task = &main_task;
  task_switch(&dispatcher_task);
}

int task_create(task_t *task, void (*start_routine)(void *), void *arg) {
  getcontext(&(task->context));

  char *stack = malloc(STACKSIZE);
  if (stack) {
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;
  } else {
    perror("task_create - malloc");
    return -1;
  }

  makecontext(&(task->context), (void *)start_routine, 1, arg);

  task->id = next_task_id++;
  task->start_tick = systime();
  task->activations = 0;
  task->prev = NULL;
  task->next = NULL;
  task->prio = 0;
  task->prio_d = 0;
  task->preemptible = 1;

#ifdef DEBUG
  printf("task_create: created task %d\n", task->id);
#endif

  if (task != &dispatcher_task) {
    task->state = READY;
    queue_append((queue_t **)&queues[READY], (queue_t *)task);
  }

  return task->id;
}

int task_switch(task_t *task) {
  task_t *previous = current_task;
  current_task = task;

#ifdef DEBUG
  printf("task_switch: changing context %d -> %d\n", previous->id, task->id);
#endif

  int status = swapcontext(&(previous->context), &(current_task->context));
  if (status < 0) {
    perror("task_switch - swapcontext");
    return status;
  }

  return 0;
}

void task_exit(int exit_code) {
#ifdef DEBUG
  printf("task_exit: exiting task %d\n", current_task->id);
#endif

  printf("Task %d exit: running time %4d ms, cpu time %2d ms, %d "
         "activations\n",
         current_task->id, systime() - current_task->start_tick,
         current_task->tick_count, current_task->activations);

  if (current_task == &dispatcher_task) {
    task_switch(&main_task);
    return;
  }

  current_task->state = TERMINATED;
  current_task->exit_code = exit_code;

  // Queue up the waiting tasks
  while (queue_size((queue_t *)current_task->waiting) > 0) {
    queue_t *task = queue_remove((queue_t **)&current_task->waiting,
                                 (queue_t *)current_task->waiting);
    queue_append((queue_t **)&queues[READY], task);
  }

  queue_append((queue_t **)&queues[TERMINATED], (queue_t *)current_task);
  task_switch(&dispatcher_task);
}

int task_id() { return current_task->id; }

void task_yield() {
#ifdef DEBUG
  printf("task_yield: called from task %d\n", current_task->id);
#endif
  current_task->state = READY;
  task_switch(&dispatcher_task);
}

int task_join(task_t *task) {
  if (task->state == TERMINATED)
    return task->exit_code;

  current_task->state = WAITING;
  queue_append((queue_t **)&task->waiting, (queue_t *)current_task);
  task_switch(&dispatcher_task);
  return task->exit_code;
}

void task_setprio(task_t *task, int prio) {
  if (prio > 19 || prio < -20)
    fprintf(stderr,
            "task_setprio: invalid priority, must be between -20 and 19");

  if (task == NULL) {
    task = current_task;
  }

  task->prio = (short)prio;
  task->prio_d = (short)prio;
}

int task_getprio(task_t *task) {
  if (task == NULL) {
    task = current_task;
  }
  return (int)task->prio;
}

void task_sleep(int t_ms) {
  current_task->state = SLEEPING;
  current_task->should_wakeup_at = systime() + t_ms;
  task_switch(&dispatcher_task);
}

unsigned int systime() { return system_ticks_count; }

task_t *scheduler() {
  if (queue_size((queue_t *)queues[READY]) == 0) {
    return NULL;
  }

  task_t *chosen = (task_t *)queue_reduce((queue_t *)queues[READY], NULL,
                                          __highest_prio_task);

  queue_foreach((queue_t *)queues[READY], __apply_aging);

  chosen->prio_d = chosen->prio;

  return (task_t *)queue_remove((queue_t **)&queues[READY], (queue_t *)chosen);
}

void dispatcher() {
  for (;;) {
    int sleeping_tasks = __queue_up_tasks_that_should_wake_up();

    task_t *task = scheduler();
    if (task == NULL) {
      if (sleeping_tasks == 0)
        task_exit(0);
      else
        continue;
    }

    task->tick_budget = DEFAULT_TICK_BUDGET;
    task->activations += 1;

    task_switch(task);

    if (!__is_in_another_queue(task))
      queue_append((queue_t **)&queues[task->state], (queue_t *)task);

    dispatcher_task.activations++;
  }
}
