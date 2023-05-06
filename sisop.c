// Import necessary libraries
#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>

// Define the size of the matrix
#define ROW 4
#define COL 5

// Function to calculate the factorial of a number
int factorial(int n) {
    if (n == 0) {
        return 1;
    } else {
        return n * factorial(n - 1);
    }
}

// Function to display the matrix
void display_matrix(int (*result)[COL], char *message) {
    printf("%s:\n", message);
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            printf("%d ", result[i][j]);
        }
        printf("\n");
    }
}

// Function to attach to shared memory
int (*attach_shared_memory(int shmid)) [COL] {
    int (*result)[COL];

    if ((result = shmat(shmid, NULL, 0)) == (int (*)[COL]) -1) {
        perror("shmat");
        exit(1);
    }

    return result;
}

// Function to detach from shared memory
void detach_shared_memory(int (*result)[COL]) {
    shmdt(result);
}

// Function to delete shared memory segment
void delete_shared_memory(int shmid) {
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
    int (*result)[COL] = attach_shared_memory(shmid);

    // Display the original matrix
    display_matrix(result, "Matrix before calculating factorial");

    // Calculate the factorial of each element in the matrix
    for (int i = 0; i < ROW; i++) {
        for (int j = 0; j < COL; j++) {
            result[i][j] = factorial(result[i][j]);
        }
    }

    // Display the resulting matrix
    display_matrix(result, "Matrix after calculating factorial");

    // Detach from shared memory
    detach_shared_memory(result);

    // Delete shared memory segment
    delete_shared_memory(shmid);

    return 0;
}
