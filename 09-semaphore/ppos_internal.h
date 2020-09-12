#include "ppos_data.h"

#ifndef __PPOS_INTERNAL__
#define __PPOS_INTERNAL__

#define STACKSIZE 32768

#define SCHEDULER_AGING_ALPHA 1
#define DEFAULT_TICK_BUDGET 20

extern task_t *scheduler();
extern void dispatcher();

extern task_t *queues[];
extern task_t main_task;
extern task_t dispatcher_task;
extern task_t *current_task;

extern struct sigaction action;
extern struct itimerval timer;

extern unsigned int system_ticks_count;

void *__highest_prio_task(void *prev, void *next);
void __apply_aging(void *ptr);
void __set_up_signals();
void __set_up_timer();
void __set_up_and_queue_main_task();
void __create_dispatcher_task();
void __timer_tick_handler();
void __enter_sem_cs(semaphore_t *s);
void __leave_sem_cs(semaphore_t *s);
void __wake_up_all_waiting_tasks(semaphore_t *s);
void __wake_up_first_waiting_task(semaphore_t *s);
unsigned int __queue_up_tasks_that_should_wake_up();
unsigned short __is_in_another_queue(task_t *t);

#endif
