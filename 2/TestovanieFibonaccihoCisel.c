#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <time.h> // Pre srand a rand

#define BUFFER_SIZE 10

// Zdieľaný buffer
typedef struct {
    int buffer[BUFFER_SIZE];
    int count;
} Buffer;

Buffer shared_buffer = {{0}, 0};

// Mutex a podmienkové premenné
pthread_mutex_t mutex;
pthread_cond_t not_empty;
pthread_cond_t not_full;

// Funkcia na kontrolu, či je číslo Fibonacciho číslo
bool is_fibonacci(int num) {
    if (num < 0) return false;

    int a = 0, b = 1;
    while (b < num) {
        int temp = b;
        b = a + b;
        a = temp;
    }
    return b == num || num == 0;
}

// Producent
void *producer(void *param) {
    int a = ((int *)param)[0];
    int b = ((int *)param)[1];
    int n = ((int *)param)[2]; // Počet čísel na generovanie

    for (int i = 0; i < n; i++) {
        int random_number = a + rand() % (b - a + 1); // Náhodné číslo medzi a a b

        pthread_mutex_lock(&mutex);

        // Čakanie, ak je buffer plný
        while (shared_buffer.count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &mutex);
        }

        // Pridanie čísla do bufferu
        shared_buffer.buffer[shared_buffer.count++] = random_number;
        printf("Producer produced: %d\n", random_number);

        // Signalizácia konzumentovi
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);

        sleep(1); // Simulácia oneskorenia
    }

    pthread_exit(0);
}

// Konzument
void *consumer(void *param) {
    while (1) {
        pthread_mutex_lock(&mutex);

        // Čakanie, ak je buffer prázdny
        while (shared_buffer.count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        // Odobratie čísla z bufferu
        int num = shared_buffer.buffer[--shared_buffer.count];
        printf("Consumer consumed: %d\n", num);

        // Kontrola, či je číslo Fibonacciho číslo
        if (is_fibonacci(num)) {
            printf("Number %d is a Fibonacci number!\n", num);
        } else {
            printf("Number %d is NOT a Fibonacci number.\n", num);
        }

        // Signalizácia producentovi
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        sleep(1); // Simulácia oneskorenia
    }

    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <a> <b> <c>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    int c = atoi(argv[3]);

    if (a > b || c <= 0) {
        fprintf(stderr, "Invalid arguments. Ensure a <= b and c > 0.\n");
        return EXIT_FAILURE;
    }

    // Inicializácia náhodného generátora
    srand(time(NULL));

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);

    pthread_t producer_thread, consumer_thread;

    // Vytvorenie vlákien
    int params[3] = {a, b, c};
    pthread_create(&producer_thread, NULL, producer, params);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // Čakanie na dokončenie producenta
    pthread_join(producer_thread, NULL);

    // Ukončenie konzumenta (nekonečná slučka)
    pthread_cancel(consumer_thread);

    // Uvoľnenie zdrojov
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);

    return EXIT_SUCCESS;
}

