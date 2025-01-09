#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define TABLE_CAPACITY 10

// Zdieľaný stôl
typedef struct {
    int table[TABLE_CAPACITY];
    int count; // Počet jedál na stole
} Table;

Table shared_table = {{0}, 0};

// Mutex a podmienkové premenné
pthread_mutex_t mutex;
pthread_cond_t not_empty;
pthread_cond_t not_full;

// Generovanie náhodného času v intervale <min, max>
float random_time(float min, float max) {
    return min + ((float)rand() / (float)RAND_MAX) * (max - min);
}

// Funkcia kuchára (producenta)
void *chef(void *param) {
    while (1) {
        pthread_mutex_lock(&mutex);

        // Čakanie, ak je stôl plný
        while (shared_table.count == TABLE_CAPACITY) {
            pthread_cond_wait(&not_full, &mutex);
        }

        // Pridanie jedla na stôl
        shared_table.table[shared_table.count++] = 1; // 1 predstavuje jedno jedlo
        printf("Chef prepared a dish. Total dishes on the table: %d\n", shared_table.count);

        // Signalizácia čašníkom
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);

        // Simulácia prípravy jedla (0.5 až 1.5 sekundy)
        usleep(random_time(0.5, 1.5) * 1000000);
    }
    pthread_exit(0);
}

// Funkcia čašníka (konzumenta)
void *waiter(void *param) {
    while (1) {
        pthread_mutex_lock(&mutex);

        // Čakanie, ak je stôl prázdny
        while (shared_table.count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        // Odobratie jedla zo stola
        shared_table.table[--shared_table.count] = 0; // Odoberie jedlo
        printf("Waiter served a dish. Remaining dishes on the table: %d\n", shared_table.count);

        // Signalizácia kuchárovi
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        // Simulácia roznášania jedla (2 až 5 sekúnd)
        usleep(random_time(2, 5) * 1000000);
    }
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <number_of_waiters>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int num_waiters = atoi(argv[1]);
    if (num_waiters <= 0) {
        fprintf(stderr, "Invalid number of waiters. Must be > 0.\n");
        return EXIT_FAILURE;
    }

    // Inicializácia mutexu a podmienkových premenných
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);

    pthread_t chef_thread;
    pthread_t waiter_threads[num_waiters];

    // Vytvorenie vlákna kuchára
    pthread_create(&chef_thread, NULL, chef, NULL);

    // Vytvorenie vlákien čašníkov
    for (int i = 0; i < num_waiters; i++) {
        pthread_create(&waiter_threads[i], NULL, waiter, NULL);
    }

    // Čakanie na ukončenie vlákna kuchára (nekonečný proces)
    pthread_join(chef_thread, NULL);

    // Ukončenie vlákien čašníkov (nekonečné procesy)
    for (int i = 0; i < num_waiters; i++) {
        pthread_cancel(waiter_threads[i]);
    }

    // Uvoľnenie zdrojov
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);

    return EXIT_SUCCESS;
}

