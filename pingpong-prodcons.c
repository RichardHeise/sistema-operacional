
#include <stdio.h>
#include <stdlib.h>
#include "ppos.h"
#include "queue.h"

task_t produtorTask1;
task_t produtorTask2;
task_t produtorTask3;
task_t consumidorTask1;
task_t consumidorTask2;

semaphore_t s_vaga;
semaphore_t s_buffer;
semaphore_t s_item;

queue_t* buffer;

typedef struct item_s
{
  struct item_t *prev, *next;
  int value;
} item_t ;

void produtor(void *arg) {
   while (1) {
      task_sleep (1000);

      item_t* item = malloc(sizeof(item_t));
      item->value = (rand() % 100 + 1);
      item->next = NULL;
      item->prev = NULL;

      sem_down(&s_vaga);

      sem_down(&s_buffer);

      if(queue_size(buffer) < 5) {
         queue_append(&buffer, (queue_t*)item);
         printf("%s inseriu %d (tem %d)\n", (char*)arg, item->value, queue_size(buffer));
      }

      sem_up(&s_buffer);

      sem_up(&s_item);
   }
}

void consumidor(void *arg) {
   task_sleep(1000);
   while (1) {
      sem_down(&s_item);

      item_t* item;

      sem_down(&s_buffer);

       if(queue_size(buffer) > 0) {
         item = (item_t*)buffer;
         queue_remove(&buffer, buffer);
         printf("%s consumiu %d (tem %d)\n", (char*)arg, item->value, queue_size(buffer));
         free(item);
      }

      sem_up(&s_buffer);

      sem_up(&s_vaga);

      task_sleep(1000);
   }
}

int main() {
      printf ("main: inicio\n") ;

      ppos_init () ;

      sem_create(&s_vaga, 5);
      sem_create(&s_buffer, 1);
      sem_create(&s_item, 0);
      
      task_create(&produtorTask1, produtor, "p1");
      task_create(&produtorTask2, produtor, "  p2");
      task_create(&produtorTask3, produtor, "    p3");

      task_create(&consumidorTask1, consumidor, "               c1");
      task_create(&consumidorTask2, consumidor, "                 c2");

      task_join(&produtorTask1);
      task_join(&consumidorTask1);

      task_join(&produtorTask2);
      task_join(&consumidorTask2);
      
      task_join(&produtorTask3);

      sem_destroy(&s_buffer);
      sem_destroy(&s_item);
      sem_destroy(&s_vaga);

      task_exit(0);

      return 1;
}