// GRR20191053 Richard Fernando Heise Ferreira  

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"
#define STACKSIZE 64*1024

task_t* _currTask, _mainTask;
queue_t* tasks = NULL;

void ppos_init () {

    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
}

int task_create (task_t *task, void (*start_func)(void *), void *arg) {

    task->prev = NULL;
    task->next = NULL;
    task->preemptable = 1;
    task->status = 1;
    task->id = queue_size(tasks) + 1;

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
    _currTask = task;
    queue_append((queue_t **) &tasks, (queue_t *)task);
}

int task_switch (task_t *task) {
    swapcontext(&_currTask->context, &task->context);
    _currTask = task;
}

void task_exit (int exit_code) {
    task_switch(&_mainTask);
    queue_remove((queue_t **) &tasks, (queue_t *)&_mainTask);

    _currTask = &_mainTask;
}