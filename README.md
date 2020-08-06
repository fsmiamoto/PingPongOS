# PingPong OS

Repository for the development of a simple OS

Based on the [excelent material](http://wiki.inf.ufpr.br/maziero/doku.php?id=so:pingpongos) provided by Prof. Carlos Maziero (UFPR)

## Currently implemented:

- [Queue library](http://wiki.inf.ufpr.br/maziero/doku.php?id=so:biblioteca_de_filas)
- [Task control](http://wiki.inf.ufpr.br/maziero/doku.php?id=so:gestao_de_tarefas)
- [Dispatcher](http://wiki.inf.ufpr.br/maziero/doku.php?id=so:dispatcher)
- [Scheduler](http://wiki.inf.ufpr.br/maziero/doku.php?id=so:escalonador_por_prioridades)
- [Metrics](http://wiki.inf.ufpr.br/maziero/doku.php?id=so:contabilizacao)
- [Main Task](http://wiki.inf.ufpr.br/maziero/doku.php?id=so:tarefa_main)

Each directory contains a full 'snapshot' of the OS at an implementation stage.

## Testing 
```bash
# Testing Task Control, for example:
# p.s tests related to scheduling and metrics can fail due to system variations,
# but the overall result should look the same.
$ cd 01-task_control && make test
```
