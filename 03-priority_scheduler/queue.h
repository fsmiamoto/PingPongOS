// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.0 -- Março de 2015

// Definição e operações em uma fila genérica.

#ifndef __QUEUE__
#define __QUEUE__

#ifndef NULL
#define NULL ((void *)0)
#endif

//------------------------------------------------------------------------------
// estrutura de uma fila genérica, sem conteúdo definido.
// Veja um exemplo de uso desta estrutura em testafila.c

typedef struct queue_t {
  struct queue_t *prev; // aponta para o elemento anterior na fila
  struct queue_t *next; // aponta para o elemento seguinte na fila
} queue_t;

//------------------------------------------------------------------------------
// Insere um elemento no final da fila.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - o elemento deve existir
// - o elemento nao deve estar em outra fila

void queue_append(queue_t **queue, queue_t *elem);

//------------------------------------------------------------------------------
// Remove o elemento indicado da fila, sem o destruir.
// Condicoes a verificar, gerando msgs de erro:
// - a fila deve existir
// - a fila nao deve estar vazia
// - o elemento deve existir
// - o elemento deve pertencer a fila indicada
// Retorno: apontador para o elemento removido, ou NULL se erro

queue_t *queue_remove(queue_t **queue, queue_t *elem);

//------------------------------------------------------------------------------
// Conta o numero de elementos na fila
// Retorno: numero de elementos na fila

int queue_size(queue_t *queue);

//------------------------------------------------------------------------------
// Percorre a fila e imprime na tela seu conteúdo. A impressão de cada
// elemento é feita por uma função externa, definida pelo programa que
// usa a biblioteca.
//
// Essa função deve ter o seguinte protótipo:
//
// void print_elem (void *ptr) ; // ptr aponta para o elemento a imprimir

void queue_print(char *name, queue_t *queue, void print_elem(void *));

//------------------------------------------------------------------------------
// Percorre a fila aplicando a função a cada elemento da mesma
//
// O protótipo de func recebe um ponteiro para o elemento da fila
// void func(void* ptr)

void queue_foreach(queue_t *queue, void (*func)(void *));

//------------------------------------------------------------------------------
// Percorre a fila aplicando a função reducer a cada elemento da mesma
//
// O protótipo de func recebe um ponteiro para o acumulador e o elemento da fila
// void* reducer(void* acc, void* elem)

void *queue_reduce(queue_t *queue, void *acc, void *(*reduc)(void *, void *));
#endif