#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <ctype.h>
#include <pthread.h>

#define MAX_EXTENSION_LENGTH 10
#define MAX_EXTENSIONS 1000

int numExtensions = 0;
char extensions[MAX_EXTENSIONS][MAX_EXTENSION_LENGTH];

void create_directory(const char *path) {
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0755);
#endif
}

void process_file(const char *filename, const char *extension, const char *subdirectory) {
    // Convert extension to lowercase
    char lowercase_extension[MAX_EXTENSION_LENGTH];
    for (int i = 0; i < strlen(extension); i++) {
        lowercase_extension[i] = tolower(extension[i]);
    }
    lowercase_extension[MAX_EXTENSION_LENGTH - 1] = '\0';


    // Check if the extension is in the list
    int found = 0;
    for (int i = 0; i < numExtensions; i++) {
        if (strcmp(lowercase_extension, extensions[i]) == 0) {
            found = 1;
            break;
        }
    }

    // Move the file to the appropriate directory
    char sourcePath[512];
    sprintf(sourcePath, "files/%s/%s", subdirectory, filename);

    char destPath[512];
    if (found) {
        sprintf(destPath, "categorized/%s/%s", lowercase_extension, filename);
    } else {
        sprintf(destPath, "categorized/other/%s", filename);
    }

    if (rename(sourcePath, destPath) != 0) {
        printf("Failed to move file: %s\n", filename);
    }
}

void *process_directory_thread(void *arg) {
    const char *dirPath = (char *)arg;

    DIR *dir;
    struct dirent *entry;
    dir = opendir(dirPath);
    if (dir == NULL) {
        printf("Failed to open directory: %s\n", dirPath);
        return NULL;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char *filename = entry->d_name;

            char extension[MAX_EXTENSION_LENGTH];
            char *dot = strrchr(filename, '.');
            if (dot) {
                strcpy(extension, dot + 1);
                process_file(filename, extension, strrchr(dirPath, '/') + 1);
            }
        } else if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char subdirPath[512];
                sprintf(subdirPath, "%s/%s", dirPath, entry->d_name);
                process_directory_thread(subdirPath);
            }
        }
    }

    closedir(dir);
    return NULL;
}

void process_directory(const char *dirPath) {
    DIR *dir;
    struct dirent *entry;
    dir = opendir(dirPath);
    if (dir == NULL) {
        printf("Failed to open directory: %s\n", dirPath);
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            char *filename = entry->d_name;

            char extension[MAX_EXTENSION_LENGTH];
            char *dot = strrchr(filename, '.');
            if (dot) {
                strcpy(extension, dot + 1);
                process_file(filename, extension, strrchr(dirPath, '/') + 1);
            }
        } else if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") != 0 && strcmp(entry->d_name, "..") != 0) {
                char subdirPath[512];
                sprintf(subdirPath, "%s/%s", dirPath, entry->d_name);
                process_directory(subdirPath);
            }
        }
    }

    closedir(dir);
}

int main() {
    // Read extensions from file
    FILE *file = fopen("extensions.txt", "r");
    if (file == NULL) {
        printf("Failed to open extensions.txt file.\n");
        return 1;
    }

    // Read extensions and validate the maximum number of extensions
    char extension[MAX_EXTENSION_LENGTH];
    while (fscanf(file, "%s", extension) == 1) {
        if (numExtensions >= MAX_EXTENSIONS) {
            printf("Maximum number of extensions reached.\n");
            break;
        }
        strcpy(extensions[numExtensions], extension);
        numExtensions++;
    }

    fclose(file);

    // Create 'categorized' directory
    create_directory("categorized");

    // Create directories for each extension
    for (int i = 0; i < numExtensions; i++) {
        // Convert extension to lowercase
        for (int j = 0; j < strlen(extensions[i]); j++) {
            extensions[i][j] = tolower(extensions[i][j]);
        }

        char path[MAX_EXTENSION_LENGTH + 12];
        sprintf(path, "categorized/%s", extensions[i]);
        create_directory(path);
    }

    // Create 'other' directory
    create_directory("categorized/other");

    // Create threads
    pthread_t threads[numExtensions + 1];
    for (int i = 0; i < numExtensions; i++) {
        pthread_create(&threads[i], NULL, process_directory_thread, extensions[i]);
    }

    // Join threads
    for (int i = 0; i < numExtensions; i++) {
        pthread_join(threads[i], NULL);
    }

    // Process files in the 'files' directory and its subdirectories
    process_directory("files");

    return 0;
}
