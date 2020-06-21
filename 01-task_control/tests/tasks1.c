// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// Teste da gestão básica de tarefas

#include "../ppos.h"
#include <stdio.h>
#include <stdlib.h>

task_t Ping, Pong;

// corpo da thread Ping
void BodyPing(void *arg) {
  int i;
  char *name = (char *)arg;

  printf("%s: inicio\n", name);
  for (i = 0; i < 4; i++) {
    printf("%s: %d\n", name, i);
    task_switch(&Pong);
  }
  printf("%s: fim\n", name);
  task_exit(0);
}

// corpo da thread Pong
void BodyPong(void *arg) {
  int i;
  char *name = (char *)arg;

  printf("%s: inicio\n", name);
  for (i = 0; i < 4; i++) {
    printf("%s: %d\n", name, i);
    task_switch(&Ping);
  }
  printf("%s: fim\n", name);
  task_exit(0);
}

int main(int argc, char *argv[]) {
  printf("main: inicio\n");

  ppos_init();

  task_create(&Ping, BodyPing, "    Ping");
  task_create(&Pong, BodyPong, "        Pong");

  task_switch(&Ping);
  task_switch(&Pong);

  printf("main: fim\n");

  exit(0);
}
