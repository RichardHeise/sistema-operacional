// PingPongOS - PingPong Operating System
// Aluno Richard Fernando Heise Ferreira - GRR20191053
// Implementações de uma fila genérica.

#include "queue.h"
#include <stdio.h>

#define ERRO_FILA_VAZIA -1
#define ERRO_ELEMENTO_NULO -2
#define ERRO_ELEMENTO_NAO_ISOLADO -3 
#define ERRO_ELEMENTO_NAO_ENCONTRADO -4

int queue_size (queue_t *queue) {
    // se a fila não aponta para nada, temos tamanho 0
    if (!queue) return 0;

    int size = 0;
    // percorremos a fila e contamos os elementos
    queue_t *q_aux = queue;
    do  {
        size++;
        q_aux = q_aux->next;
    } while (q_aux != queue);

    return size;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {
    // se a fila ta vazia printa o vetor vazio
    if (!queue) {
        printf("%s[]\n", name);
        return;
    }
    
    // percorremos a fila e printamos os elementos
    queue_t* q_aux = queue;
    printf("%s[", name);

    do {
        print_elem(q_aux);
        q_aux = q_aux->next;

        // se n estamos no ultimo elemento damos um espaço entre os prints
        if (q_aux != queue)  
            printf(" ");
        else 
            break;

    } while(1);

    printf("]\n");
}

int queue_append (queue_t **queue, queue_t *elem) {
    // checagem de erros
    // elemento deve existir
    if (!elem) return ERRO_ELEMENTO_NULO;

    // elemento deve ser isolado, isto é, não pode apontar para outros elementos
    if (elem->next || elem->prev) return ERRO_ELEMENTO_NAO_ISOLADO;

    // se a fila é vaiza, simplesmente fazemos o início apontar para o elemento
    if ( !(*queue) ) {
        (*queue) = elem;
        elem->next = (*queue);
        elem->prev = (*queue);
        return 0;
    }

    // não precisamos percorrer a lista, só reatribuir ponteiros
    (*queue)->prev->next = elem;
    elem->prev = (*queue)->prev;
    elem->next = (*queue);
    (*queue)->prev = elem;

    return 0;
}

int queue_remove (queue_t **queue, queue_t *elem) {
    // checagem de erros

    // se a fila não existe não podemos remover um elemento
    if (!(*queue)) return ERRO_FILA_VAZIA;
    if (!elem) return ERRO_ELEMENTO_NULO;

    queue_t* q_aux = (*queue);

    // checamos se a remoção será do primeiro elemento
    // teremos dois casos:
    if (q_aux == elem) {

        // 1. nossa fila só tem um elemento
        if (q_aux == q_aux->next) 
            (*queue) = NULL;    
        // 2. nossa fila tem mais de um elemento
        else 
            (*queue) = q_aux->next;
     
    // se não vamos remover o primeiro elemento, precisamos
    // percorrer a fila
    } else {
        do 
            q_aux = q_aux->next;
        while (q_aux != elem && q_aux != (*queue));

        // se não encontramos o elemento na fila, temos um erro
        if (q_aux != elem) return ERRO_ELEMENTO_NAO_ENCONTRADO;
    }
    
    // operação-se os ponteiros
    q_aux->next->prev = q_aux->prev;
    q_aux->prev->next = q_aux->next;
    q_aux->next = NULL;
    q_aux->prev = NULL;

    return 0;
}