// PingPongOS - PingPong Operating System
// Aluno Richard Fernando Heise Ferreira - GRR20191053
// Implementações de uma fila genérica.

#include "queue.h"
#include <stdio.h>

int queue_size (queue_t *queue) {
    if (!queue) return 0;
    int size = 0;
    queue_t *q_aux = queue;
    do  {
        size++;
        q_aux = q_aux->next;
    } while (q_aux != queue);

    return size;
}

void queue_print (char *name, queue_t *queue, void print_elem (void*) ) {

}

int queue_append (queue_t **queue, queue_t *elem) {
    if (!elem) {
        perror("Elemento não existe");
        return -2;
    }

    if ( !(*queue) ) {
        (*queue) = elem;
        elem->next = (*queue);
        elem->prev = (*queue);
        return 0;
    }

    queue_t* q_aux = (*queue);

    while(q_aux != (*queue)) {
        q_aux = q_aux->next;
    }

    elem->next = (*queue);
    elem->prev = q_aux->prev;
    q_aux->prev->next = elem;
    (*queue)->prev = elem;

    return 0;
}

int queue_remove (queue_t **queue, queue_t *elem) {
    int size = queue_size( (*queue) );
    if (!size) return -1;
    if (!elem) return -2;

    queue_t* q_aux = (*queue);
    int i;
    for (i = 1; (i <= size) && (q_aux != elem); i++) {
        q_aux = q_aux->next;
    }

    if (i == 1) {
        (*queue) = q_aux->next;
    }

    q_aux->next->prev = q_aux->prev;
    q_aux->prev->next = q_aux->next;
    q_aux->next = NULL;
    q_aux->prev = NULL;
    
    if (size == 1) {
        (*queue) = NULL;
    }

    return 0;
}