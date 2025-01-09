#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Počet dôkazov a podozrivých
#define TOTAL_EVIDENCE 30
#define SUSPECTS 3

// Štruktúra na sledovanie počtu dôkazov pre každého podozrivého
typedef struct {
    int evidence[SUSPECTS]; // Pole pre počet dôkazov proti každému podozrivému
    int current_evidence;  // Celkový počet prezentovaných dôkazov
} EvidenceTracker;

EvidenceTracker tracker = {{0, 0, 0}, 0};

// Mutex na synchronizáciu
pthread_mutex_t mutex;

// Funkcia na prezentovanie dôkazu
void *present_evidence(void *param) {
    while (1) {
        pthread_mutex_lock(&mutex);

        // Skončíme, ak už bolo prezentovaných všetkých 30 dôkazov
        if (tracker.current_evidence >= TOTAL_EVIDENCE) {
            pthread_mutex_unlock(&mutex);
            break;
        }

        // Náhodne priradíme dôkaz jednému z podozrivých
        int suspect = rand() % SUSPECTS;
        tracker.evidence[suspect]++;
        tracker.current_evidence++;

        printf("Evidence %d presented against Suspect %d\n", tracker.current_evidence, suspect + 1);

        pthread_mutex_unlock(&mutex);

        // Simulácia oneskorenia
        usleep(300000); // 0.3 sekundy
    }

    pthread_exit(0);
}

// Funkcia Hercula Poirota na vyhodnotenie vinníka
void *hercule_poirot(void *param) {
    while (1) {
        pthread_mutex_lock(&mutex);

        // Skončíme, keď bude prezentovaných všetkých 30 dôkazov
        if (tracker.current_evidence >= TOTAL_EVIDENCE) {
            // Vyhodnotenie vinníka
            int max_evidence = 0;
            int guilty_suspect = -1;

            for (int i = 0; i < SUSPECTS; i++) {
                if (tracker.evidence[i] > max_evidence) {
                    max_evidence = tracker.evidence[i];
                    guilty_suspect = i;
                }
            }

            printf("\nHercule Poirot: Suspect %d is guilty with %d pieces of evidence.\n",
                   guilty_suspect + 1, max_evidence);

            pthread_mutex_unlock(&mutex);
            break;
        }

        pthread_mutex_unlock(&mutex);

        // Hercule Poirot kontroluje stav každých 0.5 sekundy
        usleep(500000);
    }

    pthread_exit(0);
}

int main() {
    srand(time(NULL)); // Inicializácia náhodného generátora

    // Inicializácia mutexu
    pthread_mutex_init(&mutex, NULL);

    pthread_t evidence_thread, hercule_thread;

    // Spustenie vlákien
    pthread_create(&evidence_thread, NULL, present_evidence, NULL);
    pthread_create(&hercule_thread, NULL, hercule_poirot, NULL);

    // Čakanie na ukončenie vlákien
    pthread_join(evidence_thread, NULL);
    pthread_join(hercule_thread, NULL);

    // Uvoľnenie mutexu
    pthread_mutex_destroy(&mutex);

    return EXIT_SUCCESS;
}

