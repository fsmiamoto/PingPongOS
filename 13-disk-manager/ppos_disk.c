#include "ppos_disk.h"
#include "disk.h"
#include "ppos.h"
#include "ppos_internal.h"
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#define check(x)                                                               \
  if (x)                                                                       \
    return -1;

struct sigaction sig;
task_t disk_manager;
disk_t disk;

int __setup_signal_handler();
void __wake_up_manager();

void diskManagerBody(void *arg) {
  for (;;) {
    sem_down(&disk.mutex);

    if (disk.signal_fired) {
      task_t *request_by =
          (task_t *)queue_remove((queue_t **)&queues[WAITING],
                                 (queue_t *)disk.current_request->requested_by);
      queue_append((queue_t **)&queues[READY], (queue_t *)request_by);

      // Clean up
      free(disk.current_request);
      disk.signal_fired = 0;
    }

    int disk_idle = disk_cmd(DISK_CMD_STATUS, 0, 0) == DISK_STATUS_IDLE;

    if (disk_idle && disk.queue != NULL) {
      disk.current_request = (disk_request_t *)queue_remove(
          (queue_t **)&disk.queue, (queue_t *)disk.queue);

      int cmd;
      if (disk.current_request->type == READ) {
        cmd = DISK_CMD_READ;
      } else {
        cmd = DISK_CMD_WRITE;
      }

      disk_cmd(cmd, disk.current_request->block, disk.current_request->buffer);
    }

    sem_up(&disk.mutex);
    task_yield();
  }
}

int disk_mgr_init(int *num_blocks, int *block_size) {
  check(disk_cmd(DISK_CMD_INIT, 0, 0));

  // TODO: Check for errors here
  *num_blocks = disk_cmd(DISK_CMD_DISKSIZE, 0, 0);
  *block_size = disk_cmd(DISK_CMD_BLOCKSIZE, 0, 0);

  check(__setup_signal_handler());
  check(sem_create(&disk.mutex, 1));

  // Create disk manager task
  task_create(&disk_manager, (void *)diskManagerBody, NULL);
  disk_manager.preemptible = 0;
  disk_manager.tick_count = 0;
  disk_manager.activations = 0;
  disk_manager.start_tick = systime();

  return 0;
}

int disk_block_read(int block, void *buffer) {
#ifdef DEBUG
  printf("Read request for block %d\n", block);
#endif

  // Create disk request
  sem_down(&disk.mutex);
  disk_request_t *request = malloc(sizeof(disk_request_t));
  request->prev = NULL;
  request->next = NULL;
  request->requested_by = current_task;
  request->block = block;
  request->buffer = buffer;

  queue_append((queue_t **)&disk.queue, (queue_t *)request);

  __wake_up_manager();

  sem_up(&disk.mutex);

  // Suspend current task
  current_task->state = WAITING;
  task_switch(&dispatcher_task);

  return 0;
}

int disk_block_write(int block, void *buffer) { return 0; }

void __handle_disk_signal() {
  sem_down(&disk.mutex);
  disk.signal_fired = 1;
  __wake_up_manager();
  sem_up(&disk.mutex);
}

int __setup_signal_handler() {
  sig.sa_handler = __handle_disk_signal;
  sigemptyset(&sig.sa_mask);
  sig.sa_flags = 0;

  if (sigaction(SIGUSR1, &sig, 0) < 0) {
    return -1;
  }
  return 0;
}

void __wake_up_manager() {
  if (disk_manager.prev == NULL && disk_manager.next == NULL) {
    queue_append((queue_t **)&queues[READY], (queue_t *)&disk_manager);
  }
}
