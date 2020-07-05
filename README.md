# PingPong OS

Repository for the development of a simple OS

Based on the [excelent material](http://wiki.inf.ufpr.br/maziero/doku.php?id=so:pingpongos) provided by Prof. Carlos Maziero (UFPR)

## Currently implemented:

- [Queue library](http://wiki.inf.ufpr.br/maziero/doku.php?id=so:biblioteca_de_filas)
- [Task control](http://wiki.inf.ufpr.br/maziero/doku.php?id=so:gestao_de_tarefas)
- [Dispatcher](http://wiki.inf.ufpr.br/maziero/doku.php?id=so:dispatcher)
- [Priority Scheduler](http://wiki.inf.ufpr.br/maziero/doku.php?id=so:escalonador_por_prioridades)

Each directory contains a full 'snapshot' of the OS at an implementation stage.

## Testing 
```bash
# Testing Task Control, for example:
$ cd 01-task_control && make test
```
