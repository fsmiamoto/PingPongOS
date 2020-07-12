#include "ppos_data.h"

#ifndef __PPOS_INTERNAL__
#define __PPOS_INTERNAL__

task_t *scheduler();
void dispatcher();

void *__highest_prio_task(void *prev, void *next);
void __apply_aging(void *ptr);
void __set_up_signals();
void __set_up_timer();
void __set_up_main_task();
void __create_dispatcher_task();
void __timer_tick_handler();

#endif
