#include<stdio.h>
#include<stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include<sys/stat.h>
#include <dirent.h>
#define URL "https://drive.google.com/uc?export=download&id=1oDgj5kSiDO0tlyS7-20uz7t20X3atwrq"
#define ZIP_FILENAME "file.zip"
char *POSISI[] = {"HewanAir", "HewanAmphibi", "HewanDarat"};
char *FILE_CATEGORY[] = {"air.jpg", "amphibi.jpg", "darat.jpg"};


void download() {
    pid_t pid;
    int status;

    pid = fork();
    if (pid == 0) {
        execl("/usr/bin/wget", "wget", "-q", "-O", ZIP_FILENAME, URL, NULL);
        printf("Error: Failed to execute wget. errno=%d\n", errno);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
    } else {
        printf("Error: Failed to fork.\n");
        exit(EXIT_FAILURE);
    }
}

void extract() {
    pid_t pid;
    int status;
    pid = fork();
    if (pid == 0) {
        execl("/usr/bin/unzip", "unzip", ZIP_FILENAME, NULL);
        printf("Error: Failed to execute unzip. errno=%d\n", errno);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
    } else {
        printf("Error: Failed to fork.\n");
        exit(EXIT_FAILURE);
    }

}

void remove_zip() {
    pid_t pid;
    int status;
    pid = fork();
    if (pid == 0) {
        execl("/bin/rm", "rm", ZIP_FILENAME, NULL);
        printf("Error: Failed to execute rm. errno=%d\n", errno);
        exit(EXIT_FAILURE);
    } else if (pid > 0) {
        waitpid(pid, &status, 0);
    } else {
        printf("Error: Failed to fork.\n");
        exit(EXIT_FAILURE);
    }
}

void arrange_animal(int i) {
    DIR *dir;
    struct dirent *ent;
    char path[100] = "/home/riskiilyas/Desktop/SISOP/";
    char dir_path[100] = "/home/riskiilyas/Desktop/SISOP/";
    strcat(dir_path, POSISI[i]);
    char buffer[256];
    char *token;
    const char *delimiter = "_";
    mkdir(POSISI[i], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    dir = opendir(path);
    if (dir == NULL) {
        printf("Error: Failed to open directory.\n");
        exit(EXIT_FAILURE);
    }

    while ((ent = readdir(dir)) != NULL) {
        if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) {
            continue;
        }
        strcpy(buffer, ent->d_name);
        token = strtok(buffer, delimiter);
        while (token != NULL) {
            if (strcmp(token, FILE_CATEGORY[i]) == 0) {
                char file_path[100];
                strcpy(file_path, path);
                strcat(file_path, ent->d_name);
                printf("%s", file_path);
                int pid = fork();
                if(pid == 0) {
                    char *argv2[] = {"cp", file_path, dir_path, NULL};
                    execv("/bin/cp", argv2);
                } else {
                    wait(NULL);
                }
                remove(file_path);
                break;
            }
            token = strtok(NULL, delimiter);
        }
    }

    closedir(dir);
}

void classify(){
    pid_t pid[3];
    for(int i=0 ;i<3; i++) {
        pid[i] = fork();
        if(pid[i]==-1) {
            perror("Failed to Create Proccess!!");
            exit(EXIT_FAILURE);
        } else if(pid[i]==0) {
            arrange_animal(i);
            exit(EXIT_SUCCESS);
        }
    }

    for(int i=0;i<3;i++) {
        waitpid(pid[i], NULL, 0);
    }
}

void zip(){
    char zipFileName[] = "grapekun.zip";
    char* cmd[] = {"zip", "-r", zipFileName, POSISI[0], POSISI[1], POSISI[2], NULL};
    execvp("zip", cmd);
}

int main(){
    download();
    extract();
    remove_zip();
    classify();
    zip();
    return 0;
}
