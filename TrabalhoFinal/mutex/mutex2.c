#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define BUF_SIZE 3 //tamanho do buffer

typedef struct {
    int buf[BUF_SIZE]; // definindo o buffer
    size_t len; // controla o número de itens do buffer
    pthread_mutex_t mutex; // variável mutex 
    pthread_cond_t can_produce; // variável condicional indicando que item(s) foram removidos
    pthread_cond_t can_consume; // variável condicional indicando que item(s) foram adicionados 
} buffer_t;

void* producer(void *arg) {

  /*Lê os argumentos do buffer_t inicializado no main*/ 
  buffer_t *buffer = (buffer_t*)arg;

  /*Bloqueia o mutex para que nenhum outro thread entre na região crítica*/
  pthread_mutex_lock(&buffer->mutex);

  /* testa se o buffer está cheio */
  while(buffer->len == BUF_SIZE) { 
    printf("Buffer esta cheio\n");
    printf("Pressione Enter para continuar...");
    fflush(stdout);
    getchar();
    // é bloqueado e espera por um sinal que itens foram removidos
    pthread_cond_wait(&buffer->can_produce, &buffer->mutex);
  }

  // Se o buffer não está cheio, produz o item aleatoriamente
    printf("Buffer Vazio\n");
    printf("Pressione Enter para continuar...");
    fflush(stdout);
    getchar();
  int item = rand() % 11;
  printf("Produzido: %d\n", item);

  // insere item no buffer 
  buffer->buf[buffer->len] = item;
  ++buffer->len; //incrementa o contador 

  // sinaliza p/ consumidor despertar que um item foi produzido 
  pthread_cond_signal(&buffer->can_consume);
  // desbloqueia o mutex (libera a trava)
  pthread_mutex_unlock(&buffer->mutex);

  return NULL;
}


int main(int argc, char *argv[]) {

  srand(time(NULL));
  // inicializa elementos do buffer_t
  buffer_t buffer = {
    .len = 0,
    .mutex = PTHREAD_MUTEX_INITIALIZER,
    .can_produce = PTHREAD_COND_INITIALIZER,
    .can_consume = PTHREAD_COND_INITIALIZER
  };

  pthread_t prod; //declara produtor
  pthread_create(&prod, NULL, producer, (void*)&buffer); //cria produtor
  
  pthread_join(prod, NULL);// espera o thread produtor terminar

  printf("Fim do Programa\n");
  return 0;
}