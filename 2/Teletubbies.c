#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define TOTAL_NUMBERS 1000    // Celkový počet generovaných čísel
#define BUFFER_CAPACITY 8     // Kapacita bufferu

// Štruktúra pre zdieľaný buffer
typedef struct {
    int buffer[BUFFER_CAPACITY]; // Buffer na uchovávanie čísel
    int count;                   // Počet čísel v buffere
} SharedBuffer;

SharedBuffer shared_buffer = {{0}, 0};

// Počítadlá pre párne a nepárne čísla
int odd_count = 0;
int even_count = 0;

// Mutex a podmienkové premenné
pthread_mutex_t mutex;
pthread_cond_t not_empty;
pthread_cond_t not_full;

// Funkcia na generovanie náhodného čísla
int generate_random_number() {
    return rand() % 1000; // Náhodné číslo medzi 0 a 999
}

// Funkcia Tinky-Winkyho (producenta)
void *tinky_winky(void *param) {
    for (int i = 0; i < TOTAL_NUMBERS; i++) {
        pthread_mutex_lock(&mutex);

        // Čakanie, ak je buffer plný
        while (shared_buffer.count == BUFFER_CAPACITY) {
            pthread_cond_wait(&not_full, &mutex);
        }

        // Generovanie čísla a uloženie do bufferu
        int number = generate_random_number();
        shared_buffer.buffer[shared_buffer.count++] = number;
        printf("Tinky-Winky generated number: %d\n", number);

        // Signalizácia Laa-Laa
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);

        usleep(1000); // Krátka pauza na simuláciu generovania čísel
    }

    pthread_exit(0);
}

// Funkcia Laa-Laa (konzumenta)
void *laa_laa(void *param) {
    for (int i = 0; i < TOTAL_NUMBERS; i++) {
        pthread_mutex_lock(&mutex);

        // Čakanie, ak je buffer prázdny
        while (shared_buffer.count == 0) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        // Odobratie čísla z bufferu a jeho spracovanie
        int number = shared_buffer.buffer[--shared_buffer.count];
        if (number % 2 == 0) {
            even_count++;
            printf("Laa-Laa processed number: %d (Even)\n", number);
        } else {
            odd_count++;
            printf("Laa-Laa processed number: %d (Odd)\n", number);
        }

        // Signalizácia Tinky-Winky
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        usleep(1000); // Krátka pauza na simuláciu spracovania čísel
    }

    pthread_exit(0);
}

int main() {
    srand(time(NULL)); // Inicializácia náhodného generátora

    // Inicializácia mutexu a podmienkových premenných
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);

    pthread_t tinky_winky_thread, laa_laa_thread;

    // Spustenie vlákien
    pthread_create(&tinky_winky_thread, NULL, tinky_winky, NULL);
    pthread_create(&laa_laa_thread, NULL, laa_laa, NULL);

    // Čakanie na ukončenie vlákien
    pthread_join(tinky_winky_thread, NULL);
    pthread_join(laa_laa_thread, NULL);

    // Vyhodnotenie výsledku
    printf("\nFinal Results:\n");
    printf("Odd numbers: %d\n", odd_count);
    printf("Even numbers: %d\n", even_count);

    if (odd_count > (TOTAL_NUMBERS * 0.6)) {
        printf("Winner: Tinky-Winky (More than 60%% of numbers are odd)\n");
    } else {
        printf("Winner: Laa-Laa (Less than 60%% of numbers are odd)\n");
    }

    // Uvoľnenie zdrojov
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);

    return EXIT_SUCCESS;
}

