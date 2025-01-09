#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Počet dôkazov a kapacita bufferu
#define TOTAL_EVIDENCE 50
#define BUFFER_CAPACITY 3

// Štruktúra bufferu
typedef struct {
    int buffer[BUFFER_CAPACITY]; // Buffer na ukladanie dôkazov
    int count;                   // Počet dôkazov v buffere
} EvidenceBuffer;

EvidenceBuffer evidence_buffer = {{0}, 0};

// Sčítavanie dôkazov pre podozrivých
int suspect_evidence[2] = {0, 0}; // [Podozrivý 1, Podozrivý 2]

// Mutex a podmienkové premenné
pthread_mutex_t mutex;
pthread_cond_t not_empty;
pthread_cond_t not_full;

// Funkcia na generovanie dôkazu
int generate_evidence() {
    return rand() % 3; // 0 = nepoužiteľný, 1 = podozrivý 1, 2 = podozrivý 2
}

// Funkcia polície (producenta)
void *police(void *param) {
    for (int i = 0; i < TOTAL_EVIDENCE; i++) {
        pthread_mutex_lock(&mutex);

        // Čakanie, ak je buffer plný
        while (evidence_buffer.count == BUFFER_CAPACITY) {
            pthread_cond_wait(&not_full, &mutex);
        }

        // Generovanie a uloženie dôkazu do bufferu
        int evidence = generate_evidence();
        evidence_buffer.buffer[evidence_buffer.count++] = evidence;
        printf("Police generated evidence: %d\n", evidence);

        // Signalizácia Columbovi
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);

        usleep(300000); // Simulácia generovania dôkazu (0.3 sekundy)
    }

    pthread_exit(0);
}

// Funkcia Columba (konzumenta)
void *columbo(void *param) {
    int processed_evidence = 0;

    while (processed_evidence < TOTAL_EVIDENCE) {
        pthread_mutex_lock(&mutex);

        // Čakanie, ak je buffer prázdny a ešte sú dôkazy na spracovanie
        while (evidence_buffer.count == 0 && processed_evidence < TOTAL_EVIDENCE) {
            pthread_cond_wait(&not_empty, &mutex);
        }

        // Ak už všetky dôkazy boli spracované, ukončime slučku
        if (processed_evidence >= TOTAL_EVIDENCE) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Odobratie dôkazu z bufferu a spracovanie
        int evidence = evidence_buffer.buffer[--evidence_buffer.count];
        printf("Columbo processed evidence: %d\n", evidence);

        // Sčítavanie dôkazov pre podozrivých
        if (evidence == 1) {
            suspect_evidence[0]++;
        } else if (evidence == 2) {
            suspect_evidence[1]++;
        }

        processed_evidence++;

        // Signalizácia polícii
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mutex);

        usleep(500000); // Simulácia spracovania dôkazu (0.5 sekundy)
    }

    // Vyhodnotenie vinníka
    pthread_mutex_lock(&mutex);
    if (suspect_evidence[0] > suspect_evidence[1]) {
        printf("\nColumbo: Suspect 1 is guilty with %d valid pieces of evidence.\n", suspect_evidence[0]);
    } else if (suspect_evidence[1] > suspect_evidence[0]) {
        printf("\nColumbo: Suspect 2 is guilty with %d valid pieces of evidence.\n", suspect_evidence[1]);
    } else {
        printf("\nColumbo: It's a tie! Both suspects have %d valid pieces of evidence.\n", suspect_evidence[0]);
    }
    pthread_mutex_unlock(&mutex);

    pthread_exit(0);
}

int main() {
    srand(time(NULL)); // Inicializácia náhodného generátora

    // Inicializácia mutexu a podmienkových premenných
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_empty, NULL);
    pthread_cond_init(&not_full, NULL);

    pthread_t police_thread, columbo_thread;

    // Spustenie vlákien
    pthread_create(&police_thread, NULL, police, NULL);
    pthread_create(&columbo_thread, NULL, columbo, NULL);

    // Čakanie na ukončenie vlákien
    pthread_join(police_thread, NULL);
    pthread_join(columbo_thread, NULL);

    // Uvoľnenie zdrojov
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_empty);
    pthread_cond_destroy(&not_full);

    return EXIT_SUCCESS;
}

