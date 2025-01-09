#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h> // Pre srand a time

#define BUFFER_SIZE 10

// Zdieľaný buffer
int buffer[BUFFER_SIZE];
int count = 0; // Počet prvkov v buffere

// Mutex a podmienkové premenné
pthread_mutex_t mutex;
pthread_cond_t not_empty;
pthread_cond_t not_full;

// Funkcia na prvočíselný rozklad čísla
void prime_factorization(int num) {
    printf("Prime factorization of %d: ", num);
    if (num < 2) {
        printf("None (not valid for factorization)\n");
        return;
    }

    for (int i = 2; i * i <= num; i++) {
        while (num % i == 0) {
            printf("%d ", i);
            num /= i;
        }
    }
    if (num > 1) {
        printf("%d", num);
    }
    printf("\n");
}

// Producent
void *producer(void *param) {
    int a = ((int *)param)[0];
    int b = ((int *)param)[1];
    int n = ((int *)param)[2];

    for (int i = 0; i < n; i++) {
        int random_number = a + rand() % (b - a + 1);

        pthread_mutex_lock(&mutex);

        // Čakanie, ak je buffer plný
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &mutex);
        }

        // Pridanie čísla do bufferu
        buffer[count++] = random_number;
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
        while (count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        // Odobratie čísla z bufferu
        int num = buffer[--count];
        printf("Consumer consumed: %d\n", num);

        // Vykonanie prvočíselného rozkladu
        prime_factorization(num);

        // Signalizácia producentovi
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        sleep(1); // Simulácia oneskorenia
    }

    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <a> <b> <n>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int a = atoi(argv[1]);
    int b = atoi(argv[2]);
    int n = atoi(argv[3]);

    if (a >= b || n <= 0) {
        fprintf(stderr, "Invalid arguments. Ensure a < b and n > 0.\n");
        return EXIT_FAILURE;
    }

    // Inicializácia náhodného generátora
    srand(time(NULL));

    pthread_t producer_thread, consumer_thread;
    int params[3] = {a, b, n};

    // Inicializácia mutexu a podmienkových premenných
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);

    // Vytvorenie vlákien
    pthread_create(&producer_thread, NULL, producer, params);
    pthread_create(&consumer_thread, NULL, consumer, NULL);

    // Čakanie na ukončenie vlákien
    pthread_join(producer_thread, NULL);
    pthread_cancel(consumer_thread); // Konzument beží neustále

    // Vyčistenie zdrojov
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);

    return EXIT_SUCCESS;
}

