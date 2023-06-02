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

    while(1) {
        /*Bloqueia o mutex para que nenhum outro thread entre na região crítica*/
        pthread_mutex_lock(&buffer->mutex);

        /* testa se o buffer está cheio */
        while(buffer->len == BUF_SIZE) { 
            // é bloqueado e espera por um sinal que itens foram removidos
            pthread_cond_wait(&buffer->can_produce, &buffer->mutex);
        }

        // Se o buffer não está cheio, produz o item aleatoriamente 
        int item = rand() % 11;
        printf("Produzido: %d\n", item);

        // insere item no buffer 
        buffer->buf[buffer->len] = item;
        ++buffer->len; //incrementa o contador 

        // sinaliza p/ consumidor despertar que um item foi produzido
        pthread_cond_signal(&buffer->can_consume);
        // desbloqueia o mutex (libera a trava)
        pthread_mutex_unlock(&buffer->mutex);

        // espera por um tempo aleatório antes de produzir outro item
        sleep(10);
    }

    return NULL;
}

void* consumer(void *arg) {

    /*Lê os argumentos do buffer_t inicializado no main*/
    buffer_t *buffer = (buffer_t*)arg;

    while(1) {
        /*Bloqueia o mutex para que nenhum outro thread entre na região crítica*/
        pthread_mutex_lock(&buffer->mutex);

        /* testa se o buffer está vazio */
        while(buffer->len == 0) { 
            // é bloqueado e espera por um sinal que itens foram produzidos 
            pthread_cond_wait(&buffer->can_consume, &buffer->mutex);
        }

        // decrementa o contador
        --buffer->len;
        printf("Consumido: %d\n", buffer->buf[buffer->len]);

        // sinaliza p/ produtor despertar que um item foi consumido  
        pthread_cond_signal(&buffer->can_produce);

        // desbloqueia o mutex (libera a trava)
        pthread_mutex_unlock(&buffer->mutex);

        // espera por um tempo aleatório antes de consumir outro item
        sleep(10);
    }

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

    // Cenário 1: 0 produtores e 1 consumidor
    pthread_t consumer1;
    pthread_create(&consumer1, NULL, consumer, (void*)&buffer);
    pthread_join(consumer1, NULL);

    // Cenário 2: 1 produtor e 0 consumidores
    pthread_t producer2;
    pthread_create(&producer2, NULL, producer, (void*)&buffer);
    pthread_join(producer2, NULL);

    // Cenário 3: 3 produtores e 3 consumidores
    pthread_t producers3[3];
    pthread_t consumers3[3];

    for (int i = 0; i < 3; i++) {
        pthread_create(&producers3[i], NULL, producer, (void*)&buffer);
        pthread_create(&consumers3[i], NULL, consumer, (void*)&buffer);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(producers3[i], NULL);
        pthread_join(consumers3[i], NULL);
    }

    // Cenário 4: 3 produtores e 1 consumidor
    pthread_t producers4[3];
    pthread_t consumer4;

    for (int i = 0; i < 3; i++) {
        pthread_create(&producers4[i], NULL, producer, (void*)&buffer);
    }

    pthread_create(&consumer4, NULL, consumer, (void*)&buffer);

    for (int i = 0; i < 3; i++) {
        pthread_join(producers4[i], NULL);
    }

    pthread_join(consumer4, NULL);

    // Cenário 5: 1 produtor e 3 consumidores
    pthread_t consumer5[3];
    pthread_t producer5;

    pthread_create(&producer5, NULL, producer, (void*)&buffer);

    for (int i = 0; i < 3; i++) {
        pthread_create(&consumer5[i], NULL, consumer, (void*)&buffer);
    }

    pthread_join(producer5, NULL);

    for (int i = 0; i < 3; i++) {
        pthread_join(consumer5[i], NULL);
    }

    return 0;
}