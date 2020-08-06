// Queue library for PingPong OS
// Author: Francisco Miamoto
// Version 1.0 - March 2020

#include "queue.h"
#include <stdio.h>

// Internal functions
queue_t *__get_last_elem(queue_t **queue);
int __queue_has_elem(queue_t **queue, queue_t *elem);

void queue_append(queue_t **queue, queue_t *elem) {
  if (queue == NULL) {
    fprintf(stderr, "ERROR(queue_append): queue does not exist\n");
    return;
  }

  if (elem == NULL) {
    fprintf(stderr, "ERROR(queue_append): element does not exist\n");
    return;
  }

  if (elem->next != NULL || elem->prev != NULL) {
    fprintf(stderr, "ERROR(queue_append): element belongs to another queue\n");
    return;
  }

  // The queue is empty
  if (*queue == NULL) {
    *queue = elem;
    elem->next = elem;
    elem->prev = elem;
    return;
  }

  queue_t *first = *queue;
  queue_t *last = __get_last_elem(queue);

  last->next = elem;
  elem->prev = last;

  first->prev = elem;
  elem->next = first;
}

queue_t *queue_remove(queue_t **queue, queue_t *elem) {
  if (queue == NULL) {
    fprintf(stderr, "ERROR(queue_remove): queue does not exist\n");
    return NULL;
  }

  if (queue_size(*queue) == 0) {
    fprintf(stderr, "ERROR(queue_remove): queue is empty\n");
    return NULL;
  }

  if (elem == NULL) {
    fprintf(stderr, "ERROR(queue_remove): element does not exist\n");
    return NULL;
  }

  if (!__queue_has_elem(queue, elem)) {
    fprintf(stderr,
            "ERROR(queue_remove): element does not belong to the queue\n");
    return NULL;
  }

  // Queue has only one element
  if (elem->prev == elem && elem->next == elem) {
    *queue = NULL;
    elem->prev = NULL;
    elem->next = NULL;
    return elem;
  }

  elem->prev->next = elem->next;
  elem->next->prev = elem->prev;

  // Move the queue's head if the removed elem is at the beginning
  if (*queue == elem) {
    *queue = elem->next;
  }

  elem->prev = NULL;
  elem->next = NULL;

  return elem;
}

int queue_size(queue_t *queue) {
  if (queue == NULL)
    return 0;

  int queueSize = 0;
  queue_t *firstElem = queue;
  queue_t *currentElem = queue;

  do {
    queueSize++;
    currentElem = currentElem->next;
  } while (currentElem != firstElem);

  return queueSize;
}

void queue_print(char *name, queue_t *queue, void (*print_elem)(void *)) {
  int size = queue_size(queue);
  queue_t *current = queue;

  printf("%s: [", name);
  for (int i = 0; i < queue_size(queue); i++) {
    print_elem(current);
    if (i < size - 1) {
      printf(" ");
    }
    current = current->next;
  }
  printf("]\n");
}

void queue_foreach(queue_t *queue, void (*func)(void *)) {
  queue_t *current = queue;
  for (int i = 0; i < queue_size(queue); i++) {
    func(current);
    current = current->next;
  }
}

void *queue_reduce(queue_t *queue, void *acc, void *(*func)(void *, void *)) {
  queue_t *current = queue;
  for (int i = 0; i < queue_size(queue); i++) {
    acc = func(acc, current);
    current = current->next;
  }
  return acc;
}

// Returns the last element in the queue
queue_t *__get_last_elem(queue_t **queue) {
  if (*queue == NULL) {
    return NULL;
  }

  queue_t *firstElem = *queue;
  queue_t *lastElem = *queue;

  do {
    lastElem = lastElem->next;
  } while (lastElem->next != firstElem);

  return lastElem;
}

// Returns 1 if the queue has the element in it, 0 otherwise
int __queue_has_elem(queue_t **queue, queue_t *elemToBeFound) {
  if (*queue == NULL) {
    return 0;
  }

  queue_t *firstElem = *queue;
  queue_t *currentElem = *queue;

  do {
    if (currentElem == elemToBeFound) {
      return 1;
    }
    currentElem = currentElem->next;
  } while (currentElem != firstElem);

  return 0;
}
