#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

// Počet dôkazov a podozrivých
#define TOTAL_EVIDENCE 25
#define SUSPECTS 2

// Štruktúra na sledovanie počtu dôkazov pre každého podozrivého
typedef struct {
    int evidence[SUSPECTS]; // Pole pre počet dôkazov proti každému podozrivému
    int current_evidence;  // Celkový počet prezentovaných dôkazov
} EvidenceTracker;

EvidenceTracker tracker = {{0, 0}, 0};

// Mutex na synchronizáciu
pthread_mutex_t mutex;

// Funkcia na prezentovanie dôkazu
void *present_evidence(void *param) {
    while (1) {
        pthread_mutex_lock(&mutex);

        // Skončíme, ak už bolo prezentovaných všetkých 25 dôkazov
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
        usleep(500000); // 0.5 sekundy
    }

    pthread_exit(0);
}

// Funkcia Sherlocka Holmesa na vyhodnotenie vinníka
void *sherlock(void *param) {
    while (1) {
        pthread_mutex_lock(&mutex);

        // Skončíme, keď bude prezentovaných všetkých 25 dôkazov
        if (tracker.current_evidence >= TOTAL_EVIDENCE) {
            // Vyhodnotenie vinníka
            if (tracker.evidence[0] > tracker.evidence[1]) {
                printf("\nSherlock: Suspect 1 is guilty with %d pieces of evidence.\n", tracker.evidence[0]);
            } else if (tracker.evidence[1] > tracker.evidence[0]) {
                printf("\nSherlock: Suspect 2 is guilty with %d pieces of evidence.\n", tracker.evidence[1]);
            } else {
                printf("\nSherlock: It's a tie! Both suspects have %d pieces of evidence.\n", tracker.evidence[0]);
            }

            pthread_mutex_unlock(&mutex);
            break;
        }

        pthread_mutex_unlock(&mutex);

        // Sherlock kontroluje stav každých 0.5 sekundy
        usleep(500000);
    }

    pthread_exit(0);
}

int main() {
    srand(time(NULL)); // Inicializácia náhodného generátora

    // Inicializácia mutexu
    pthread_mutex_init(&mutex, NULL);

    pthread_t evidence_thread, sherlock_thread;

    // Spustenie vlákien
    pthread_create(&evidence_thread, NULL, present_evidence, NULL);
    pthread_create(&sherlock_thread, NULL, sherlock, NULL);

    // Čakanie na ukončenie vlákien
    pthread_join(evidence_thread, NULL);
    pthread_join(sherlock_thread, NULL);

    // Uvoľnenie mutexu
    pthread_mutex_destroy(&mutex);

    return EXIT_SUCCESS;
}

