// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// Estruturas de dados internas do sistema operacional

#ifndef __PPOS_DATA__
#define __PPOS_DATA__

#include "queue.h"    // biblioteca de filas genéricas
#include <ucontext.h> // biblioteca POSIX de trocas de contexto

typedef enum { CREATED, READY, RUNNING, WAITING, TERMINATED } state_t;

// Estrutura que define um Task Control Block (TCB)
typedef struct task_t {
  struct task_t *prev, *next; // ponteiros para usar em filas
  int id;                     // identificador da tarefa
  ucontext_t context;         // contexto armazenado da tarefa
  state_t state;              // estado atual da tarefa
  short prio;                 // prioridade estática da tarefa
  short prio_d;      // prioridade dinâmica da tarefa (afetada pelo aging)
  short preemptible; // indica se a tarefa é preemptável
  unsigned int tick_budget; // quantidade de ticks disponíveis
  unsigned int tick_count;  // contagem de ticks disponíveis
  unsigned int activations;
  unsigned int start_tick;
  struct task_t *waiting;
  int exit_code;

} task_t;

// estrutura que define um semáforo
typedef struct {
  // preencher quando necessário
} semaphore_t;

// estrutura que define um mutex
typedef struct {
  // preencher quando necessário
} mutex_t;

// estrutura que define uma barreira
typedef struct {
  // preencher quando necessário
} barrier_t;

// estrutura que define uma fila de mensagens
typedef struct {
  // preencher quando necessário
} mqueue_t;

#endif
