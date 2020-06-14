#include "ppos.h"
#include "ppos_data.h"
#include <stdio.h>
#include <stdlib.h>

// Thread stack size
#define STACKSIZE 32768

int next_task_id = 1; // IDs for other tasks start at 1
task_t main_task;
task_t *current_task;

void ppos_init() {
  main_task.id = 0;
  getcontext(&(main_task.context));
  main_task.next = NULL;
  main_task.prev = NULL;

  current_task = &main_task;

  // Deactivate the stdout buffer used by the printf function
  setvbuf(stdout, 0, _IONBF, 0);
}

int task_create(task_t *task, void (*start_routine)(void *), void *arg) {
  current_task->next = task;

  getcontext(&(task->context));

  char *stack = malloc(STACKSIZE);
  if (stack) {
    task->context.uc_stack.ss_sp = stack;
    task->context.uc_stack.ss_size = STACKSIZE;
    task->context.uc_stack.ss_flags = 0;
    task->context.uc_link = 0;
  } else {
    perror("task_create: error on stack allocation");
    return -1;
  }

  makecontext(&(task->context), (void *)start_routine, 1, arg);

  task->id = next_task_id++;
  task->prev = current_task;

#ifdef DEBUG
  printf("task_create: created task %d\n", task->id);
#endif

  return task->id;
}

int task_switch(task_t *task) {
  task->prev = current_task;
  current_task->next = task;

  current_task = task;

  int status = swapcontext(&(task->prev->context), &(task->context));
  if (status < 0) {
    perror("task_switch: error on swapcontext call");
    return status;
  }

#ifdef DEBUG
  printf("task_switch: changing context %d -> %d\n", task->prev->id, task->id);
#endif

  return 0;
}

void task_exit(int exit_code) { task_switch(&main_task); }

int task_id() { return current_task->id; }

void task_yield() { return; }
