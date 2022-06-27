// GRR20191053 Richard Fernando Heise Ferreira  

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"
#define STACKSIZE 64*1024

task_t* _currTask, _mainTask;
int _id = 0;
queue_t* tasks = NULL;

void ppos_init () {

    // Redundante, porém escolhi manter consistência
    // não criamos uma stack porque a main já possui uma
    // afinal ela é uma task
    _mainTask.prev = NULL;
    _mainTask.next = NULL;
    _mainTask.preemptable = 0;
    _mainTask.status = 1;
    _mainTask.id = 0;
    getcontext(&_mainTask.context);

    // Atual é a main
    _currTask = &_mainTask;

    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
}

int task_create (task_t *task, void (*start_func)(void *), void *arg) {

    // Definimos as variáveis da task
    task->prev = NULL;
    task->next = NULL;
    task->preemptable = 1;
    task->status = 1;
    task->id = ++_id;

    // alocamos uma pilha para a task
    char *stack ;

    getcontext (&task->context) ;

    stack = malloc (STACKSIZE) ;
    if (stack)
    {
        task->context.uc_stack.ss_sp = stack ;
        task->context.uc_stack.ss_size = STACKSIZE ;
        task->context.uc_stack.ss_flags = 0 ;
        task->context.uc_link = 0 ;
    }
    else
    {
        perror ("Erro na criação da pilha: ") ;
        exit (1) ;
    }

    makecontext (&task->context, (void*)start_func, 1, arg) ;

    return task->id;
}

int task_switch (task_t *task) {
    // A task atual precisa ser guardada antes de ser reatribuída
    // pois não podemos atribuir após o swapcontext devido à forma 
    // como a função funciona

    ucontext_t* oldTask = &_currTask->context;
    _currTask = task;
    swapcontext(oldTask, &task->context);

    return 0;
}

void task_exit (int exit_code) {
    task_switch(&_mainTask);
}

int task_id () {
    return _currTask->id;
}