#include "ppos_data.h"

#ifndef __PPOS_INTERNAL__
#define __PPOS_INTERNAL__

#define STACKSIZE 32768

#define SCHEDULER_AGING_ALPHA 1
#define DEFAULT_TICK_BUDGET 20

task_t *scheduler();
void dispatcher();

void *__highest_prio_task(void *prev, void *next);
void __apply_aging(void *ptr);
void __set_up_signals();
void __set_up_timer();
void __set_up_and_queue_main_task();
void __create_dispatcher_task();
void __timer_tick_handler();

#endif
