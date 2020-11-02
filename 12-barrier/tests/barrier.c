// PingPongOS - PingPong Operating System
// Prof. Carlos A. Maziero, DINF UFPR
// Versão 1.1 -- Julho de 2016

// barreira de sincronização para N threads

#include "../ppos.h"
#include <stdio.h>
#include <stdlib.h>

task_t Pang, Peng, Ping, Pong, Pung;
barrier_t b;

// corpo das threads
void Body(void *arg) {
  int sleepTime;

  printf("%5d ms: %s: inicio\n", systime(), (char *)arg);

  sleepTime = random() % 20000;
  printf("%5d ms: %s: dorme %ds\n", systime(), (char *)arg, sleepTime);
  task_sleep(sleepTime);

  printf("%5d ms: %s: chega na barreira\n", systime(), (char *)arg);
  barrier_join(&b);
  printf("%5d ms: %s: passa da barreira\n", systime(), (char *)arg);

  printf("%5d ms: %s: fim\n", systime(), (char *)arg);
  task_exit(0);
}

int main(int argc, char *argv[]) {
  ppos_init();

  printf("%5d ms: main: inicio\n", systime());

  // cria barreira
  barrier_create(&b, 6); // main + pang + peng + ... + pung = 6

  // cria tarefas
  task_create(&Pang, Body, "    Pang");
  task_create(&Peng, Body, "        Peng");
  task_create(&Ping, Body, "            Ping");
  task_create(&Pong, Body, "                Pong");
  task_create(&Pung, Body, "                    Pung");

  printf("%5d ms: main: chega na barreira\n", systime());
  barrier_join(&b);
  printf("%5d ms: main: passa da barreira\n", systime());

  printf("%5d ms: main: fim\n", systime());
  task_exit(0);

  exit(0);
}
