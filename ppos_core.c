// GRR20191053 Richard Fernando Heise Ferreira  

#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"
#define STACKSIZE 64*1024
#define ERROR_STATUS -5
#define ALPHA -1

enum status_t {PRONTA = 1, RODANDO, SUSPENSA, TERMINADA};

task_t* _currTask = NULL;
task_t _mainTask;
int _id = 0;
int userTasks = 0;
task_t task_dispatcher;
task_t* tasks;

task_t* scheduler() {
    task_t* dripless = tasks;
    task_t* aux = tasks;
    do 
    {
        aux = aux->next;
        if (aux->di_drip < dripless->di_drip) {
            dripless = aux;
        } else {
            aux->di_drip += ALPHA;
        }

    } while (aux != tasks);

    dripless->di_drip = dripless->st_drip;

    return dripless;
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
                    free(next_task->context.uc_stack.ss_sp);
                    if ( queue_remove( (queue_t **) &tasks, (queue_t *)next_task ) < 0 ) {
                        fprintf(stderr, "Erro ao remover elemento %d da lista, abortando.\n", next_task->id);
                        exit(0);
                    }
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
    _mainTask.status = RODANDO;
    _mainTask.id = 0;
    _mainTask.st_drip = 0;
    _mainTask.di_drip = 0;
    
    // Redundante, porém escolhi manter consistência
    getcontext(&_mainTask.context);

    // não criamos uma stack porque a main já possui uma
    // afinal ela é uma task

    // Atual é a main
    _currTask = &_mainTask;

    task_create(&task_dispatcher, dispatcher, NULL);

    task_dispatcher.status = RODANDO;
    task_dispatcher.preemptable = 0;
    if ( queue_remove( (queue_t **) &tasks, (queue_t *)&task_dispatcher ) < 0 ) {
        fprintf(stderr, "Erro ao remover dispatcher da lista, abortando.\n");
        exit(0);
    }
    --userTasks;

    /* desativa o buffer da saida padrao (stdout), usado pela função printf */
    setvbuf (stdout, 0, _IONBF, 0) ;
}

int task_create (task_t *task, void (*start_func)(void *), void *arg) {

    // Definimos as variáveis da task
    task->prev = NULL;
    task->next = NULL;
    task->preemptable = 1;
    task->status = PRONTA;
    task->id = ++_id;
    task->st_drip = 0;
    task->di_drip = 0;

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

    if ( queue_append( (queue_t **)&tasks, (queue_t *) task) < 0 ) {
        fprintf(stderr, "Erro ao inserir elemento %d na lista, abortando.\n", task->id);
        exit(0);
    }

    userTasks += 1;
    return task->id;
}

int task_switch (task_t *task) {
    // A task atual precisa ser guardada antes de ser reatribuída
    // pois não podemos atribuir após o swapcontext devido à forma 
    // como a função funciona

    ucontext_t* oldTask = &_currTask->context;
    _currTask = task;
    _currTask->status = RODANDO;
    swapcontext(oldTask, &task->context);

    return 0;
}

void task_exit (int exit_code) {
    _currTask->status = TERMINADA;

    if ( _currTask == &task_dispatcher ) {
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

void task_setprio (task_t *task, int prio) {
    if (!task) {
        _currTask->di_drip = prio;
        _currTask->st_drip = prio;  
        return;
    }

    task->di_drip = prio;
    task->st_drip = prio;
}

int task_getprio (task_t *task) {
    if (!task) return _currTask->st_drip;

    return task->st_drip;
}