#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>

#define DEFECT_CHANCE 25
#define MAX_GIFT_TYPES 3

// Štruktúry na uchovanie darčekov a detí
typedef struct {
    int type;      // Typ darčeka
    int defective; // 1 ak je darček pokazený, inak 0
} Gift;

typedef struct {
    int id;          // ID dieťaťa
    int prev_gift;   // Predchádzajúci darček
    int gift_count;  // Počet darčekov
} Child;

// Zdieľané premenné
Gift *bag;
int bag_size = 0;
int bag_capacity;
int gifts_made = 0;
int children_count;
int elves_count;
Child *children;
int current_child = 0;
int elves_working = 1;

// Synchronizačné premenné
pthread_mutex_t bag_mutex;
pthread_cond_t bag_not_full;
pthread_cond_t bag_not_empty;

// Funkcia na generovanie náhodného čísla v intervale
int random_between(int min, int max) {
    return rand() % (max - min + 1) + min;
}

// Funkcia skriatka (producent)
void *elf(void *arg) {
    int elf_id = *(int *)arg;
    while (1) {
        pthread_mutex_lock(&bag_mutex);
        if (!elves_working) {
            pthread_mutex_unlock(&bag_mutex);
            break;
        }

        while (bag_size == bag_capacity) {
            pthread_cond_wait(&bag_not_full, &bag_mutex);
        }

        // Vytvorenie darčeka
        Gift gift;
        gift.type = random_between(1, MAX_GIFT_TYPES);
        gift.defective = (random_between(1, 100) <= DEFECT_CHANCE);

        // Vloženie darčeka do vreca
        bag[bag_size++] = gift;
        gifts_made++;
        printf("Elf %d vyrobil darček typu %d (%s).\n", elf_id, gift.type, gift.defective ? "pokazený" : "v poriadku");

        pthread_cond_signal(&bag_not_empty);
        pthread_mutex_unlock(&bag_mutex);

        usleep(random_between(500, 1500) * 1000); // Simulácia času výroby
    }
    free(arg);
    pthread_exit(NULL);
}

// Funkcia Mikuláša (konzument)
void *santa(void *arg) {
    while (current_child < children_count) {
        pthread_mutex_lock(&bag_mutex);
        while (bag_size == 0) {
            pthread_cond_wait(&bag_not_empty, &bag_mutex);
        }

        // Vybratie darčeka z vreca
        Gift gift = bag[--bag_size];
        pthread_cond_signal(&bag_not_full);
        pthread_mutex_unlock(&bag_mutex);

        if (!gift.defective) {
            // Ak nie je darček pokazený
            Child *child = &children[current_child];
            if (child->prev_gift == gift.type) {
                printf("Dieťa %d dostalo druhý rovnaký darček typu %d, odchádza.\n", child->id, gift.type);
                current_child++;
            } else {
                printf("Dieťa %d dostalo darček typu %d.\n", child->id, gift.type);
                child->prev_gift = gift.type;
                child->gift_count++;
            }
        } else {
            // Ak je darček pokazený
            printf("Mikuláš odovzdal pokazený darček.\n");
        }
    }

    pthread_mutex_lock(&bag_mutex);
    elves_working = 0;
    pthread_cond_broadcast(&bag_not_full);
    pthread_mutex_unlock(&bag_mutex);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Použitie: %s <počet skriatkov> <kapacita vreca> <počet detí>\n", argv[0]);
        return EXIT_FAILURE;
    }

    elves_count = atoi(argv[1]);
    bag_capacity = atoi(argv[2]);
    children_count = atoi(argv[3]);

    // Inicializácia pamäte
    bag = malloc(bag_capacity * sizeof(Gift));
    children = malloc(children_count * sizeof(Child));
    for (int i = 0; i < children_count; i++) {
        children[i].id = i + 1;
        children[i].prev_gift = -1;
        children[i].gift_count = 0;
    }

    srand(time(NULL));

    // Inicializácia mutexov a podmienkových premenných
    pthread_mutex_init(&bag_mutex, NULL);
    pthread_cond_init(&bag_not_full, NULL);
    pthread_cond_init(&bag_not_empty, NULL);

    // Vytvorenie vlákien skriatkov
    pthread_t elf_threads[elves_count];
    for (int i = 0; i < elves_count; i++) {
        int *elf_id = malloc(sizeof(int));
        *elf_id = i + 1;
        pthread_create(&elf_threads[i], NULL, elf, elf_id);
    }

    // Vytvorenie vlákna Mikuláša
    pthread_t santa_thread;
    pthread_create(&santa_thread, NULL, santa, NULL);

    // Čakanie na dokončenie vlákien
    for (int i = 0; i < elves_count; i++) {
        pthread_join(elf_threads[i], NULL);
    }
    pthread_join(santa_thread, NULL);

    // Výpočet priemerného počtu darčekov na dieťa
    int total_gifts = 0;
    for (int i = 0; i < children_count; i++) {
        total_gifts += children[i].gift_count;
    }
    printf("Mikuláš skončil. Priemerný počet darčekov na dieťa: %.2f\n", (double)total_gifts / children_count);

    // Uvoľnenie zdrojov
    free(bag);
    free(children);
    pthread_mutex_destroy(&bag_mutex);
    pthread_cond_destroy(&bag_not_full);
    pthread_cond_destroy(&bag_not_empty);

    return EXIT_SUCCESS;
}

