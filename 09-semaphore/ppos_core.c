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

  printf("Task %d exit: execution time %4d ms, processor time %4d ms, %4d "
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

  __wake_up_waiting_tasks(s);

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
  __leave_sem_cs(s);

  __wake_up_first_waiting_task(s);

  return 0;
}

int sem_down(semaphore_t *s) {
#ifdef DEBUG
  printf("Task %d called sem_down\n", current_task->id);
#endif
  if (s == NULL || s->is_destroyed)
    return -1;

  if (s->value == 0) {
    current_task->state = SLEEPING;
    queue_append((queue_t **)&s->waiting, (queue_t *)current_task);
    task_yield();
  }

  __enter_sem_cs(s);
  if (s->is_destroyed)
    return -1;
  s->value -= 1;
  __leave_sem_cs(s);

  return 0;
}

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
  int sleeping_tasks;
  task_t *next_ready;

  for (;;) {
    // Queue up sleeping tasks that should wake up
    if ((sleeping_tasks = queue_size((queue_t *)queues[SLEEPING])) > 0) {
      task_t *task = queues[SLEEPING];
      for (int i = 0; i < sleeping_tasks; i++) {
        task_t *next = task->next;
        if (task->should_wakeup_at <= systime()) {
          queue_remove((queue_t **)&queues[SLEEPING], (queue_t *)task);
          queue_append((queue_t **)&queues[READY], (queue_t *)task);
        }
        task = next;
      }
    }

    if ((next_ready = scheduler()) == NULL) {
      if (sleeping_tasks == 0)
        task_exit(0);
      continue;
    }

    queue_remove((queue_t **)&queues[READY], (queue_t *)next_ready);
    next_ready->tick_budget = DEFAULT_TICK_BUDGET;
    next_ready->activations += 1;
    task_switch(next_ready);
    if (next_ready->next == NULL && next_ready->prev == NULL)
      queue_append((queue_t **)&queues[next_ready->state],
                   (queue_t *)next_ready);

    dispatcher_task.activations++;
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

void __set_up_signals() {
  action.sa_handler = __timer_tick_handler;
  sigemptyset(&action.sa_mask);
  action.sa_flags = 0;
  if (sigaction(SIGALRM, &action, 0) < 0) {
    perror("sigaction");
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
    perror("setitimer");
    exit(1);
  }
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

void __create_dispatcher_task() {
  task_create(&dispatcher_task, (void *)dispatcher, NULL);
  dispatcher_task.preemptible = 0;
  dispatcher_task.tick_count = 0;
  dispatcher_task.activations = 0;
  dispatcher_task.start_tick = systime();
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

void __wake_up_waiting_tasks(semaphore_t *s) {
  int waiting_tasks = queue_size((queue_t *)s->waiting);

  if (waiting_tasks == 0)
    return;

  task_t *task = s->waiting;
  for (int i = 0; i < waiting_tasks; i++) {
    task_t *next = task->next;
    queue_remove((queue_t **)&s->waiting, (queue_t *)task);
    queue_append((queue_t **)&queues[READY], (queue_t *)task);
    task = next;
  }
}

void __wake_up_first_waiting_task(semaphore_t *s) {
  if (s->waiting == NULL)
    return;

  task_t *waiting_task =
      (task_t *)queue_remove((queue_t **)&s->waiting, (queue_t *)s->waiting);
  queue_append((queue_t **)&queues[READY], (queue_t *)waiting_task);
}
