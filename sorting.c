#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

void *arrange_animal(void *arg) {
    char *dir_path = (char*) arg;
    DIR *dir;
    struct dirent *ent;
    char path[100] = "home/doscuments/praktikum/modul3"; //"/home/riskiilyas/Desktop/SISOP/" 
    char buffer[256];
    char *token;
    char *ext;
    const char *delimiter = "_";
    char ext_file_path[100];
    sprintf(ext_file_path, "%s/extension.txt", dir_path);
    FILE *ext_file = fopen(ext_file_path, "r");
    if (ext_file == NULL) {
        printf("Error: Failed to open format file for %s.\n", dir_path);
        pthread_exit(NULL);
    }
    mkdir(dir_path, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    dir = opendir(path);
    if (dir == NULL) {
        printf("Error: Failed to open directory.\n");
        pthread_exit(NULL);
    }

    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        strcpy(buffer, ent->d_name);
        token = strtok(buffer, ".");
        ext = strtok(NULL, ".");
        if (ext == NULL) {
            continue;
        }
        fseek(ext_file, 0, SEEK_SET);
        int found_ext = 0;
        char line[100];
        while (fgets(line, 100, ext_file) != NULL) {
            if (line[strlen(line) - 1] == '\n') {
                line[strlen(line) - 1] = '\0';
            }
            if (strcmp(line, ext) == 0) {
                found_ext = 1;
                break;
            }
        }
        if (found_ext) {
            char file_path[100];
            strcpy(file_path, path);
            strcat(file_path, ent->d_name);
            printf("%s", file_path);
            char new_path[100];
            strcpy(new_path, dir_path);
            strcat(new_path, "/");
            strcat(new_path, ent->d_name);
            if (rename(file_path, new_path) != 0) {
                printf("Error: Failed to move file %s\n", file_path);
            }
        }
    }

    fclose(ext_file);
    closedir(dir);
    pthread_exit(NULL);
}

void classify(){
    pthread_t tid[3];
    const char* POSISI[] = {"petshop/cat", "petshop/dog", "petshop/other"};
    for(int i=0 ;i<3; i++) {
        if (pthread_create(&tid[i], NULL, arrange_animal, (void*) POSISI[i]) != 0) {
            perror("Failed to create thread!!");
            exit(EXIT_FAILURE);
        }
    }

    for(int i=0;i<3;i++) {
        pthread_join(tid[i], NULL);
    }
}

int main() {
    classify();
    return 0;
}
