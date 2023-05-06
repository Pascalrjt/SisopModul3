#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

#define ROW 4
#define COL 5

// Function to calculate the factorial of a number
int faktorial(int n) {
    if (n == 0) {
        return 1;
    } else {
        return n * faktorial(n - 1);
    }
}

// Function to display the matrix
void tampilkan_matriks(int (*hasil)[COL], char *pesan) {
    printf("%s:\n", pesan);
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            printf("%d ", hasil[i][j]);
        }
        printf("\n");
    }
}

// Function to attach to shared memory
int (*hubungkan_shared_memory(int shmid)) [COL] {
    int (*hasil)[COL];

    if ((hasil = shmat(shmid, NULL, 0)) == (int (*)[COL]) -1) {
        perror("shmat");
        exit(1);
    }

    return hasil;
}

// Function to detach from shared memory
void detach_shared_memory(int (*hasil)[COL]) {
    shmdt(hasil);
}

// Function to delete shared memory segment
void hapus_shared_memory(int shmid) {
    shmctl(shmid, IPC_RMID, NULL);
}

int main() {
    key_t key = 5678;
    int shmid;

    // Create shared memory segment
    if ((shmid = shmget(key, sizeof(int[ROW][COL]), 0666)) < 0) {
        perror("shmget");
        exit(1);
    }

    // Attach to shared memory segment
    int (*hasil)[COL] = hubungkan_shared_memory(shmid);

    // Display the original matrix
    tampilkan_matriks(hasil, "Matriks sebelum dihitung faktorial");

    // Calculate the factorial of each element in the matrix
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            hasil[i][j] = faktorial(hasil[i][j]);
        }
    }

    // Display the resulting matrix
    tampilkan_matriks(hasil, "Matriks setelah dihitung faktorial");

    // Detach from shared memory
    detach_shared_memory(hasil);

    // Delete shared memory segment
    hapus_shared_memory(shmid);

    return 0;
}
