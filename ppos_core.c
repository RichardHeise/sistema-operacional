// GRR20191053 Richard Fernando Heise Ferreira

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>
#include "ppos.h"
#include "queue.h"

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

#define STACKSIZE 64 * 1024 // Tamanho da pilha
#define ERROR_STATUS -5     // Erro caso o status da tarefa não existe
#define ERROR_QUEUE -6      // Erro genérico da fila
#define ERROR_STACK -7      // Erro genérico da pilha
#define ALPHA -1            // Fator de envelhecimento (linear) {
#define QUANTUM 20          // Quantum padrão de cada tarefa

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

// Enum dos status
enum status_t {

    READY = 1,
    RUNNING,
    ASLEEP,
    FINISHED

};

struct sigaction action;              // Estrutura que define um tratador de sinal
struct itimerval timer;               // Estrutura de inicialização to timer

task_t *currTask = NULL;              // Tarefa atual
task_t mainTask;                      // Tarefa da main

unsigned int _id = 0;                 // Contador de identificador de tarefas
unsigned int activeTasks = 0;         // Número de tarefas de usuário ativas

task_t *ready_tasks = NULL;           // Fila de tarefas prontas
task_t *bed = NULL;
task_t task_dispatcher;               // Tarefa do despachante

unsigned int ticks = 0;               // Ticks do relógio
unsigned int upTime = 0;              // Tempo ativo de cada tarefa

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

task_t *scheduler() {

    if (!ready_tasks) return NULL;

    // A tarefa com menos drip é a prioritária
    task_t *dripless = ready_tasks;
    task_t *aux = ready_tasks;

    // Percorremos a lista de tarefas
    do {
    
        aux = aux->next;

        // A tarefa com menor drip é escolhida
        if (aux->di_drip < dripless->di_drip) {
        
            dripless->di_drip += ALPHA; 
            dripless = aux;

        }
        else {
        
            // Se a tarefa escolhida não é a com menor drip
            // adicionamos o fator de envelhecimento
            aux->di_drip += ALPHA;
        }

    } while (aux != ready_tasks);

    // Resetamos o drip dinâmico da tarefa escolhida
    dripless->di_drip = dripless->st_drip;

    return dripless;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void dispatcher () {

    task_t *next_task = NULL;

    // Enquanto houverem tarefas de usuário
    while ( activeTasks ) {

        if (bed) {
            task_t *rooster = bed->prev; 
            int napping = queue_size( (queue_t *) bed );

            for (int i = 0; i < napping; i++) {
                rooster = bed->next;
                if (rooster->alarm <= systime()) {
                    task_resume(rooster, &bed);
                } 
            }
        }

        // Escolhe a próxima tarefa a executar
        next_task = scheduler();

        // Escalonador escolheu uma tarefa?      
        if (next_task) {

            // Transfere controle para a próxima tarefa
            task_switch (next_task);

            // Voltando ao dispatcher, trata a tarefa de acordo com seu estado
            switch (next_task->status) {
                case READY:

                    task_dispatcher.activs++;
                    next_task->activs++;
                    next_task->quantum = QUANTUM;

                    break;
                case RUNNING:
                    break;
                case ASLEEP:
                    break;
                case FINISHED:

                    // Removemos a tarefa terminada da fila e ajustamos o número total de tarefas ativas
                    --activeTasks;
                    free(next_task->context.uc_stack.ss_sp);
                    if ( queue_remove( (queue_t **) &ready_tasks, (queue_t *)next_task ) < 0 ) {

                        fprintf(stderr, "Erro ao remover elemento %d da lista, abortando.\n", next_task->id);
                        exit(ERROR_QUEUE);

                    }

                    break;
                default:
                    fprintf(stderr, "Status da tarefa %d não abarcado, abortando.\n", next_task->id);
                    exit(ERROR_STATUS);
            }         

        }

    }

   // encerra a tarefa dispatcher
   task_exit(0);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void chronos() {

    ticks++;

    if (!currTask->preemptable) {
        return;
    }

    if (currTask->quantum > 0) {
    
        --currTask->quantum;
    }
    else {
    
        currTask->status = READY;
        task_switch(&task_dispatcher);
    }
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

static void big_bang() {

    // Registra a ação para o sinal de timer SIGALRM
    action.sa_handler = chronos;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    if (sigaction(SIGALRM, &action, 0) < 0) {
    
        perror("Erro em sigaction: ");
        exit(1);
    }

    timer.it_value.tv_usec = 1000; // Primeiro disparo, em micro-segundos
    timer.it_value.tv_sec = 0;
    timer.it_interval.tv_usec = 1000; // Disparos subsequentes, em micro-segundos
    timer.it_interval.tv_sec = 0;

    // Arma o temporizador ITIMER_REAL
    if (setitimer(ITIMER_REAL, &timer, 0) < 0) {
    
        perror("Erro em setitimer: ");
        exit(1);
    }
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void ppos_init() {

    task_create(&mainTask, NULL, NULL);

    // Atual é a main
    currTask = &mainTask;
    
    // Criando o despachante
    task_create(&task_dispatcher, dispatcher, NULL);

    // Ajustes pós-criação genérica
    task_dispatcher.status = RUNNING;
    task_dispatcher.preemptable = 0;
    if (queue_remove((queue_t **)&ready_tasks, (queue_t *)&task_dispatcher) < 0) {
    
        fprintf(stderr, "Erro ao remover dispatcher da lista, abortando.\n");
        exit(ERROR_QUEUE);
    }
    --activeTasks;

    big_bang();

    /* desativa o buffer da saida padrao (stdout) {, usado pela função printf */
    setvbuf(stdout, 0, _IONBF, 0);

    task_yield();
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int task_create(task_t *task, void (*start_func)(void *), void *arg) {

    // Definimos as variáveis da tarefa
    task->prev = NULL;
    task->next = NULL;
    task->preemptable = 1;
    task->status = READY;
    task->id = ++_id;
    task->st_drip = 0;
    task->di_drip = 0;
    task->quantum = QUANTUM;
    task->exeTime = systime();
    task->procTime = 0;
    task->activs = 0;
    task->exit_code = -1;
    task->sus_tasks = NULL;
    task->alarm = 0;

    // Alocamos uma pilha para a task
    char *stack;

    getcontext(&task->context);

    stack = malloc(STACKSIZE);
    if (stack) {
    
        task->context.uc_stack.ss_sp = stack;
        task->context.uc_stack.ss_size = STACKSIZE;
        task->context.uc_stack.ss_flags = 0;
        task->context.uc_link = 0;

    }
    else {

        perror("Erro na criação da pilha: ");
        exit(ERROR_STACK);
    }

    makecontext(&task->context, (void *)start_func, 1, arg);

    // Adiciona a tarefa na fila
    if (queue_append((queue_t **)&ready_tasks, (queue_t *)task) < 0) {
    
        fprintf(stderr, "Erro ao inserir elemento %d na lista, abortando.\n", task->id);
        exit(ERROR_QUEUE);
    }
    activeTasks += 1;

    return task->id;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int task_switch(task_t *task) {

    if (currTask->status != FINISHED) {
        currTask->procTime += (systime() - upTime);
    }

    upTime = systime();

    // A task atual precisa ser guardada antes de ser reatribuída
    ucontext_t *oldTask = &currTask->context;
    currTask = task;
    currTask->status = RUNNING;

    swapcontext(oldTask, &task->context);

    return 0;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void task_exit(int exit_code) {

    // Status da tarefa agora é terminada
    currTask->status = FINISHED;

    currTask->procTime += (systime() - upTime);
    currTask->exeTime = (systime() - currTask->exeTime);
    currTask->exit_code = exit_code;

    printf("Task %d exit: execution time %d ms, processor time %d ms, %d activations\n", 
    currTask->id, currTask->exeTime, currTask->procTime, currTask->activs);

    task_t* sus_task = currTask->sus_tasks;

    while ( sus_task ) {
        task_resume(sus_task, &currTask->sus_tasks);
        sus_task = currTask->sus_tasks;
    }

    // Se a tarefa atual não é o despachante, passa a ser
    if (currTask != &task_dispatcher) {
    
        task_switch(&task_dispatcher);
    }
    else {
        exit(0);
    }
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int task_id() { 
    return currTask->id;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void task_yield() { 

    task_dispatcher.activs++;
    task_switch(&task_dispatcher);
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void task_setprio(task_t *task, int prio) { 

    // Quando atribuímos o drip estático
    // precisamos atribuir o dinâmico também
    if (!task) {
    
        currTask->di_drip = prio;
        currTask->st_drip = prio;
        return;
    }

    task->di_drip = prio;
    task->st_drip = prio;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int task_getprio(task_t *task) {

    if (!task) {
        return currTask->st_drip;
    }
    return task->st_drip;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

unsigned int systime () { 
    return ticks;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void task_suspend (task_t **queue) {

    if ( queue_remove((queue_t **)&ready_tasks, (queue_t *)currTask) < 0 ) {

        fprintf(stderr, "Erro ao remover tarefa na fila %p, abortando.\n", queue);
        exit(ERROR_QUEUE);
    }

    currTask->status = ASLEEP;
    if (queue_append((queue_t **)queue, (queue_t *)currTask) < 0) {
    
        fprintf(stderr, "Erro ao adicionar tarefa na fila %p, abortando.\n", queue);
        exit(ERROR_QUEUE);
    }

    task_yield();

}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void task_resume (task_t *task, task_t **queue) {

    if (queue_remove((queue_t **)queue, (queue_t *)task) < 0) {
    
        fprintf(stderr, "Erro ao remover tarefa da fila %p, abortando.\n", queue);
        exit(ERROR_QUEUE);
    }

    task->status = READY;
    if (queue_append((queue_t **)&ready_tasks, (queue_t *)task) < 0) {
    
        fprintf(stderr, "Erro ao adicionar tarefa na fila de prontas (%p), abortando.\n", ready_tasks);
        exit(ERROR_QUEUE);
    }

}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

int task_join (task_t *task) {

    if (!task || task->status == FINISHED) return -1;

    task_suspend(&task->sus_tasks);

    return task->exit_code;
}

/* ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ */

void task_sleep (int t) {
    currTask->alarm = systime() + t;
    task_suspend(&bed);
    task_yield();
}