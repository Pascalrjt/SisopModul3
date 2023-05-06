#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

// Struct to hold the file name and its extension
typedef struct {
    char filename[256];
    char extension[256];
} File;

// Function to read the extensions from extensions.txt and store them in an array
void readExtensions(char extensions[][256]) {
    FILE *fp = fopen("extensions.txt", "r");
    if (fp == NULL) {
        perror("fopen error");
        exit(EXIT_FAILURE);
    }

    int i = 0;
    while (fgets(extensions[i], 256, fp)) {
        // Remove the newline character from the end of the string
        extensions[i][strcspn(extensions[i], "\n")] = 0;
        printf("%s\n", extensions[i]);
        i++;
    }

    fclose(fp);
    return i;
}

// Function to check if a file has a specified extension
int hasExtension(char *filename, char *extension) {
    char *dot = strrchr(filename, '.');
    if (dot && !strcmp(dot + 1, extension)) {
        printf("file found\n");
        return 1;
    }
    return 0;
}

// Function to sort a file based on its extension
void *sortFile(void *arg) {
    File *file = (File *) arg;

    char dirName[256];
    snprintf(dirName, sizeof(dirName), "./%s", file->extension);

    // Create the directory if it doesn't exist
    if (access(dirName, F_OK) == -1) {
        mkdir(dirName, 0777);
    }

    // Move the file to the appropriate directory
    char command[512];
    snprintf(command, sizeof(command), "mv files/%s %s/%s", file->filename, dirName, file->filename);
    system(command);

    free(file);
    pthread_exit(NULL);
}

// Function to count the number of each extension and print out the results
void countExtensions(char extensions[][256], int numExtensions) {
    DIR *dir = opendir("files");
    if (dir == NULL) {
        perror("opendir error");
        exit(EXIT_FAILURE);
    }

    int counts[numExtensions + 1]; // +1 for "other" category
    for (int i = 0; i <= numExtensions; i++) {
        counts[i] = 0;
    }

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char *filename = entry->d_name;
            int foundExtension = 0;
            for (int i = 0; i < numExtensions; i++) {
                if (hasExtension(filename, extensions[i])) {
                    foundExtension = 1;
                    counts[i]++;
                    break;
                }
            }
            if (!foundExtension) {
                counts[numExtensions]++;
            }
        }
    }

    closedir(dir);

    printf("File Extension Counts:\n");
    for (int i = 0; i < numExtensions; i++) {
        printf("%s: %d\n", extensions[i], counts[i]);
    }
    printf("Other: %d\n", counts[numExtensions]);
}



int main() {
    // Read the extensions from extensions.txt
    char extensions[256][256];
    readExtensions(extensions);

    int numExtensions = readExtensions(extensions);
    countExtensions(extensions, numExtensions);

    // Open the directory containing the unsorted files
    DIR *dir = opendir("files");
    if (dir == NULL) {
        perror("opendir error");
        exit(EXIT_FAILURE);
    }

    // Create the threads to sort the files
    pthread_t thread_id[1024];
    int thread_count = 0;
    //int counter = 0;

    struct dirent *entry;
    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char *filename = entry->d_name;
            for (int i = 0; i < 256; i++) {
                if (hasExtension(filename, extensions[i])) {
                    // Create a File struct to hold the file name and its extension
                    File *file = malloc(sizeof(File));
                    strcpy(file->filename, filename);
                    strcpy(file->extension, extensions[i]);
                    //counter++;

                    // Create a thread to sort the file
                    pthread_create(&thread_id[thread_count], NULL, sortFile, file);
                    thread_count++;

                    break;
                }
                //printf("%d\n", counter);
                //counter = 0;
            }
        }
    }

    // Wait for all threads to finish
    for (int i = 0; i < thread_count; i++) {
        pthread_join(thread_id[i], NULL);
    }
    // Close the directory
    closedir(dir);

    return 0;
}
