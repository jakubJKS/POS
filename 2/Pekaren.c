#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_BREAD_ON_COUNTER 2 // Maximálny počet chlebov na pulte
#define DEFAULT_CUSTOMERS 10   // Predvolený počet zákazníkov
#define DEFAULT_BREAD 4        // Predvolený počet chlebov na pečenie

// Zdieľané premenné
int bread_on_counter = 0;       // Počet chlebov na pulte
int total_bread = DEFAULT_BREAD; // Celkový počet chlebov na upečenie
int total_customers_served = 0;
int customers_count = DEFAULT_CUSTOMERS;

// Synchronizačné nástroje
pthread_mutex_t mutex;
pthread_cond_t bread_available;
pthread_cond_t space_available;

// Generovanie náhodného času v intervale
int random_time(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Pekár (producent)
void *baker(void *param) {
    while (1) {
        pthread_mutex_lock(&mutex);

        // Ak už nie je chlieb na pečenie a pult je prázdny, pekár končí
        if (total_bread == 0 && bread_on_counter == 0) {
            printf("Baker finished baking all bread.\n");
            pthread_cond_broadcast(&bread_available); // Prebudenie čakajúcich zákazníkov
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Čakanie, ak je pult plný
        while (bread_on_counter == MAX_BREAD_ON_COUNTER && total_bread > 0) {
            pthread_cond_wait(&space_available, &mutex);
        }

        // Ak už nie je chlieb na pečenie, ukončí cyklus
        if (total_bread == 0) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Pečenie chleba
        int baking_time = random_time(2, 6); // Náhodný čas pečenia
        printf("Baker is baking bread. It will take %d seconds...\n", baking_time);
        sleep(baking_time);

        // Uloženie chleba na pult
        bread_on_counter++;
        total_bread--;
        printf("Baker placed bread on the counter. Bread on counter: %d, Bread left to bake: %d\n",
               bread_on_counter, total_bread);

        // Signalizácia zákazníkom
        pthread_cond_signal(&bread_available);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}

// Zákazník (konzument)
void *customer_thread(void *param) {
    int id = *(int *)param;
    free(param); // Uvoľnenie dynamickej pamäte pre ID zákazníka

    sleep(random_time(2, 6)); // Čas príchodu zákazníka

    pthread_mutex_lock(&mutex);

    // Čakanie na chlieb alebo ukončenie pekára
    while (bread_on_counter == 0 && total_bread > 0) {
        printf("Customer %d is waiting for bread...\n", id);
        pthread_cond_wait(&bread_available, &mutex);
    }

    // Ak nie je dostupný chlieb a pekár skončil, zákazník odíde
    if (bread_on_counter == 0 && total_bread == 0) {
        pthread_mutex_unlock(&mutex);
        printf("Customer %d left without bread.\n", id);
        pthread_exit(0);
    }

    // Odobratie chleba z pultu
    bread_on_counter--;
    total_customers_served++;
    printf("Customer %d took a bread. Bread on counter: %d\n", id, bread_on_counter);

    // Signalizácia pekárovi
    pthread_cond_signal(&space_available);
    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    // Vstupy pre počet zákazníkov a chlebov
    if (argc > 1) {
        customers_count = atoi(argv[1]);
        if (argc > 2) {
            total_bread = atoi(argv[2]);
        }
    }

    srand(time(NULL)); // Inicializácia náhodného generátora

    // Inicializácia mutexu a podmienkových premenných
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&bread_available, NULL);
    pthread_cond_init(&space_available, NULL);

    pthread_t baker_thread;
    pthread_t customer_threads[customers_count];

    // Spustenie vlákna pekára
    pthread_create(&baker_thread, NULL, baker, NULL);

    // Spustenie vlákien zákazníkov
    for (int i = 0; i < customers_count; i++) {
        int *id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&customer_threads[i], NULL, customer_thread, id);
    }

    // Čakanie na ukončenie vlákien zákazníkov
    for (int i = 0; i < customers_count; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    // Čakanie na ukončenie vlákna pekára
    pthread_join(baker_thread, NULL);

    // Výpis záverečnej správy
    printf("\nBakery closed. Total customers served: %d\n", total_customers_served);

    // Uvoľnenie zdrojov
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&bread_available);
    pthread_cond_destroy(&space_available);

    return EXIT_SUCCESS;
}

