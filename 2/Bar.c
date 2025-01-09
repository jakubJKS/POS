#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define MAX_DRINKS_ON_COUNTER 1 // Maximálny počet drinkov na bare
#define TOTAL_DRINKS 10         // Predvolený počet drinkov na prípravu

// Štruktúra na reprezentáciu zákazníka
typedef struct {
    int id;  // ID zákazníka
} Customer;

// Zdieľané dáta
int drinks_on_counter = 0; // Počet drinkov na pulte
int total_drinks = TOTAL_DRINKS;
int total_customers_served = 0;
int customers_count = 10;

// Synchronizačné nástroje
pthread_mutex_t mutex;
pthread_cond_t drink_available;
pthread_cond_t space_available;

// Funkcia na generovanie náhodného času v intervale
int random_time(int min, int max) {
    return min + rand() % (max - min + 1);
}

// Barman (producent)
void *barman(void *param) {
    while (1) {
        pthread_mutex_lock(&mutex);

        // Ak už nie sú drinky na prípravu a pult je prázdny, barman končí
        if (total_drinks == 0 && drinks_on_counter == 0 && total_customers_served == customers_count) {
            printf("Barman finished all drinks.\n");
            pthread_cond_broadcast(&drink_available); // Prebudenie všetkých zákazníkov
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Čakanie, ak je pult plný
        while (drinks_on_counter == MAX_DRINKS_ON_COUNTER && total_drinks > 0) {
            pthread_cond_wait(&space_available, &mutex);
        }

        // Ak už nie sú drinky alebo zákazníci, ukončí cyklus
        if (total_drinks == 0 || total_customers_served == customers_count) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Príprava drinku
        int drink_type = rand() % 2; // 0 alebo 1
        int preparation_time = (drink_type == 0) ? 2 : 1;
        printf("Barman is preparing a drink of type %d. It will take %d seconds...\n", drink_type, preparation_time);
        sleep(preparation_time);

        // Položenie drinku na pult
        drinks_on_counter++;
        total_drinks--;
        printf("Barman placed a drink on the counter. Drinks on counter: %d, Drinks left: %d\n",
               drinks_on_counter, total_drinks);

        // Signalizácia zákazníkom
        pthread_cond_signal(&drink_available);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(0);
}


// Zákazník (konzument)
void *customer_thread(void *param) {
    Customer *customer = (Customer *)param;
    sleep(random_time(1, 8)); // Čas, kým zákazník príde do baru

    pthread_mutex_lock(&mutex);

    // Čakanie na drink alebo ukončenie barmana
    while (drinks_on_counter == 0 && total_drinks > 0) {
        printf("Customer %d is waiting for a drink...\n", customer->id);
        pthread_cond_wait(&drink_available, &mutex);
    }

    // Ak drinky nie sú k dispozícii a barman skončil, zákazník odíde
    if (drinks_on_counter == 0 && total_drinks == 0) {
        pthread_mutex_unlock(&mutex);
        printf("Customer %d left without a drink.\n", customer->id);
        free(customer);
        pthread_exit(0);
    }

    // Odobratie drinku z pultu
    drinks_on_counter--;
    total_customers_served++;
    printf("Customer %d took a drink. Drinks on counter: %d\n", customer->id, drinks_on_counter);

    // Signalizácia barmanovi
    pthread_cond_signal(&space_available);
    pthread_mutex_unlock(&mutex);

    free(customer);
    pthread_exit(0);
}

int main(int argc, char *argv[]) {
    // Vstupy pre počet zákazníkov a drinkov
    if (argc > 1) {
        customers_count = atoi(argv[1]);
        if (argc > 2) {
            total_drinks = atoi(argv[2]);
        }
    }

    srand(time(NULL)); // Inicializácia náhodného generátora

    // Inicializácia mutexu a podmienkových premenných
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&drink_available, NULL);
    pthread_cond_init(&space_available, NULL);

    pthread_t barman_thread;
    pthread_t customer_threads[customers_count];

    // Spustenie vlákna barmana
    pthread_create(&barman_thread, NULL, barman, NULL);

    // Spustenie vlákien zákazníkov
    for (int i = 0; i < customers_count; i++) {
        Customer *customer = malloc(sizeof(Customer));
        customer->id = i + 1;
        pthread_create(&customer_threads[i], NULL, customer_thread, (void *)customer); // Použitie správneho názvu funkcie
    }

    // Čakanie na ukončenie vlákien zákazníkov
    for (int i = 0; i < customers_count; i++) {
        pthread_join(customer_threads[i], NULL);
    }

    // Čakanie na ukončenie vlákna barmana
    pthread_join(barman_thread, NULL);

    // Výpis záverečnej správy
    printf("\nBar closed. Total customers served: %d\n", total_customers_served);

    // Uvoľnenie zdrojov
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&drink_available);
    pthread_cond_destroy(&space_available);

    return EXIT_SUCCESS;
}

