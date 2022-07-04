// GRR20191053 Richard Fernando Heise Ferreira  

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"
#define STACKSIZE 64*1024
#define ERROR_STATUS -5

enum status_t {PRONTA = 1, RODANDO, SUSPENSA, TERMINADA};

task_t* _currTask = NULL;
task_t _mainTask;
int _id = 0;
int userTasks = -1;
task_t task_dispatcher;
task_t* tasks;

task_t* scheduler() {
    tasks = tasks->next;
    return tasks;
}

void dispatcher () {

    task_t* next_task = NULL;
    // enquanto houverem tarefas de usuário
    while ( userTasks ) {

        // escolhe a próxima tarefa a executar
        next_task = scheduler();

        // escalonador escolheu uma tarefa?      
        if (next_task) {

            // transfere controle para a próxima tarefa
            task_switch (next_task);

            // voltando ao dispatcher, trata a tarefa de acordo com seu estado
            switch (next_task->status) {
                case PRONTA:
                    break;
                case RODANDO:
                    break;
                case SUSPENSA:
                    break;
                case TERMINADA:
                    --userTasks;
                    queue_remove( (queue_t **) &tasks, (queue_t *)&_currTask );
                    break;
                default:
                    fprintf(stderr, "Status da tarefa %d não abarcado, abortando.\n", next_task->id);
                    exit(0);
            }         

        }

    }

   // encerra a tarefa dispatcher
   task_exit(0);
}

void ppos_init () {

    _mainTask.prev = NULL;
    _mainTask.next = NULL;
    _mainTask.preemptable = 0;
    _mainTask.status = 1;
    _mainTask.id = 0;
    
    // Redundante, porém escolhi manter consistência
    // não criamos uma stack porque a main já possui uma
    // afinal ela é uma task
    getcontext(&_mainTask.context);

    // Atual é a main
    _currTask = &_mainTask;

    task_create(&task_dispatcher, dispatcher, NULL);

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

    queue_append( (queue_t **)&tasks, (queue_t *) task);

    userTasks += 1;
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
    _currTask->status = TERMINADA;
    if ( &task_dispatcher == _currTask ) {
        task_switch(&_mainTask);
    } else {
        task_yield();
    }
}

int task_id () {
    return _currTask->id;
}

void task_yield () {
    task_switch(&task_dispatcher);
}