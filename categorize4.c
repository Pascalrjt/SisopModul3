#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

struct thread_data {
  char *source_folder;
  char *destination_folder;
  char *extension;
};

void *thread_func(void *arg) {
  struct thread_data *data = (struct thread_data *)arg;
  DIR *dir = opendir(data->source_folder);
  if (dir == NULL) {
    printf("Error opening source folder\n");
    return NULL;
  }

  struct dirent *ent;
  while ((ent = readdir(dir)) != NULL) {
    char *filename = ent->d_name;

    char *extension = strrchr(filename, '.');
    if (extension == NULL) {
      continue;
    }

    if (strcmp(extension, data->extension) == 0) {
      char folderPath[1024];
      snprintf(folderPath, sizeof(folderPath), "%s/%s", data->destination_folder, extension);
      if (!mkdir(folderPath, 0777)) {
        printf("Error creating folder %s\n", folderPath);
      }

      char command[1024];
      snprintf(command, sizeof(command), "mv %s %s", filename, folderPath);
      system(command);
    }
  }

  closedir(dir);
  return NULL;
}

int main() {
  // Define the source and destination folders
  char sourceFolder[] = "files";
  char destinationFolder[] = "organized";

  // Define the extensions file
  char extensions_file[] = "extensions.txt";

  // Open the extensions file
  FILE *fp = fopen(extensions_file, "r");
  if (fp == NULL) {
    printf("Error opening extensions file\n");
    return 1;
  }

  // Create a map of extensions to threads
  pthread_t threads[10];
  int num_threads = 0;
  char line[1024];
  while (fgets(line, sizeof(line), fp) != NULL) {
    char *extension = strtok(line, " ");
    if (extension == NULL) {
      continue;
    }

    // Create a thread for the extension
    struct thread_data data;
    data.source_folder = sourceFolder;
    data.destination_folder = destinationFolder;
    data.extension = extension;
    pthread_create(threads + num_threads, NULL, thread_func, &data);
    num_threads++;
  }

  // Create the "categorised" folder if it does not exist
  char categorisedFolderPath[1024];
  snprintf(categorisedFolderPath, sizeof(categorisedFolderPath), "%s/categorised", destinationFolder);
  if (!mkdir(categorisedFolderPath, 0777)) {
    printf("Error creating folder %s\n", categorisedFolderPath);
  }

  // Wait for all the threads to finish
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }

  // Close the extensions file
  fclose(fp);

  return 0;
}
