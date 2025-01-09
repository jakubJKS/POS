#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 20

// Zdieľaný buffer
typedef struct {
    int s, r; // Dvojica čísel
} Pair;

Pair buffer[BUFFER_SIZE];
int count = 0; // Počet prvkov v buffere

// Mutex a podmienkové premenné
pthread_mutex_t mutex;
pthread_cond_t not_empty;
pthread_cond_t not_full;

// Funkcia na výpočet faktoriálu
int factorial(int n) {
    if (n == 0 || n == 1) return 1;
    return n * factorial(n - 1);
}

// Funkcia na výpočet kombinačného čísla
int combination(int n, int k) {
    return factorial(n) / (factorial(k) * factorial(n - k));
}

// Producent
void *producer(void *param) {
    int a = ((int *)param)[0];
    int b = ((int *)param)[1];
    int n = ((int *)param)[2];

    for (int i = 0; i < n; i++) {
        int s = a + rand() % (b - a + 1);
        int r = a + rand() % (b - a + 1);

        pthread_mutex_lock(&mutex);

        // Čakanie, ak je buffer plný
        while (count == BUFFER_SIZE) {
            pthread_cond_wait(&not_full, &mutex);
        }

        // Pridanie dvojice do bufferu
        if (s < r) { int temp = s; s = r; r = temp; } // Zabezpečíme, že s >= r
        buffer[count++] = (Pair){s, r};
        printf("Producer produced pair: (%d, %d)\n", s, r);

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

        // Odobratie dvojice z bufferu
        Pair pair = buffer[--count];
        printf("Consumer consumed pair: (%d, %d)\n", pair.s, pair.r);

        // Výpočet kombinačného čísla
        int result = combination(pair.s, pair.r);
        printf("Combination C(%d, %d) = %d\n", pair.s, pair.r, result);

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

