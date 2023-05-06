#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/shm.h>

#define ROWS 4
#define COLS 5

int (*hasil)[COLS];
int matriks1[ROWS][2];
int matriks2[2][COLS];

void *multiply(void *arg) {
    int thread_id = *(int *)arg;
    int start = thread_id * ROWS / 4;
    int end = (thread_id + 1) * ROWS / 4;

    for (int i = start; i < end; i++) {
        for (int j = 0; j < COLS; j++) {
            int sum = 0;
            for (int k = 0; k < 2; k++) {
                sum += matriks1[i][k] * matriks2[k][j];
            }
            hasil[i][j] = sum;
        }
    }

    pthread_exit(NULL);
}

int main() {
    key_t key = 5678;
    int shmid;
    pthread_t threads[4];
    int thread_ids[4] = {0, 1, 2, 3};

    // Create shared memory segment
    if ((shmid = shmget(key, sizeof(int[ROWS][COLS]), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // Attach to shared memory segment
    if ((hasil = shmat(shmid, NULL, 0)) == (int (*)[COLS]) -1) {
        perror("shmat");
        exit(1);
    }

    // Initialize matrices with random numbers
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < 2; j++) {
            matriks1[i][j] = rand() % 5 + 1;
        }
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < COLS; j++) {
            matriks2[i][j] = rand() % 4 + 1;
        }
    }

    // Create threads
    for (int i = 0; i < 4; i++) {
        if (pthread_create(&threads[i], NULL, multiply, (void *)&thread_ids[i])) {
            perror("pthread_create");
            exit(1);
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < 4; i++) {
        if (pthread_join(threads[i], NULL)) {
            perror("pthread_join");
            exit(1);
        }
    }

    // Detach from shared memory
    if (shmdt(hasil) == -1) {
        perror("shmdt");
        exit(1);
    }

    return 0;
}