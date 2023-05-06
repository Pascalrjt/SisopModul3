#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <pthread.h>

#define ROW 4
#define COL 5

int **create_matrix() {
    int **mat = malloc(ROW * sizeof(int *));
    for (int i = 0; i < ROW; i++) {
        mat[i] = malloc(COL * sizeof(int));
    }
    return mat;
}

void free_matrix(int **mat) {
    for (int i = 0; i < ROW; i++) {
        free(mat[i]);
    }
    free(mat);
}

void init_matrix(int **mat) {
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            mat[i][j] = rand() % 10 + 1;
        }
    }
}

void print_matrix(int **mat, char *title) {
    printf("%s:\n", title);
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            printf("%d ", mat[i][j]);
        }
        printf("\n");
    }
}

int faktorial(int n) {
    if (n == 0) {
        return 1;
    } else {
        return n * faktorial(n - 1);
    }
}

void *hitung_faktorial(void *arg) {
    int **mat = (int **)arg;

    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            mat[i][j] = faktorial(mat[i][j]);
        }
    }

    pthread_exit(NULL);
}

int main() {
    key_t key = 5678;
    int shmid;
    int **mat;
    int i, j;

    // Buat shared memory segment
    if ((shmid = shmget(key, sizeof(int[ROW][COL]), IPC_CREAT | 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // Hubungkan ke shared memory segment
    if ((mat = shmat(shmid, NULL, 0)) == (int **) -1) {
        perror("shmat");
        exit(1);
    }

    // Inisialisasi matriks dengan angka random
    init_matrix(mat);

    // Tampilkan matriks sebelum dihitung faktorialnya
    print_matrix(mat, "Matriks sebelum dihitung faktorial");

    // Hitung faktorial menggunakan thread
    pthread_t tid;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_create(&tid, &attr, hitung_faktorial, mat);
    pthread_join(tid, NULL);

    // Tampilkan matriks setelah dihitung faktorialnya
    print_matrix(mat, "Matriks setelah dihitung faktorial");

    // Detach shared memory
    shmdt(mat);

    // Hapus shared memory segment
    shmctl(shmid, IPC_RMID, NULL);

    // Free matrix memory
    free_matrix(mat);

    return 0;
}