#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <dirent.h>

#include <pthread.h>

#include <sys/stat.h>

#include <unistd.h>

#include <errno.h>

#include <fcntl.h>

#include <time.h>

#define MAX_PATH 1024
#define MAX_EXT 100
#define MAX_THREAD 10
#define MAX_FILENAME 255

struct file_info {
  char src_path[MAX_PATH];
  char dst_path[MAX_PATH];
};

struct ext_info {
  char ext[MAX_EXT];
  int count;
  int max;
};

struct file_info files[MAX_PATH];
struct ext_info exts[MAX_EXT];

int num_files = 0;
int num_exts = 0;
int num_other = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void * move_file(void * arg);
void create_directory(char * dir_path);
void get_extension(char * filename, char * ext);
int read_max(char * filename, char * ext);
void parse_extensions(char * filename);
void parse_directory(char * dir_path);
void log_event(char * event_type, char * path);

int main(int argc, char * argv[]) {
  if (argc != 3) {
    printf("Usage: %s <directory_path> <extensions_file_path>\n", argv[0]);
    return 1;
  }

  // Parse extensions file
  parse_extensions(argv[2]);

  // Parse directory
  parse_directory(argv[1]);

  // Print file counts
  printf("%s", exts[0].ext);
  for (int i = 1; i < num_exts; i++) {
    printf(" %s", exts[i].ext);
  }
  printf(" other\n");

  printf("%d", exts[0].count);
  for (int i = 1; i < num_exts; i++) {
    printf(" %d", exts[i].count);
  }
  printf(" %d\n", num_other);

  return 0;
}

void * move_file(void * arg) {
  struct file_info * info = (struct file_info * ) arg;

  // Get extension of file
  char ext[MAX_EXT];
  get_extension(info -> src_path, ext);

  // Check if extension is in list of extensions
  int ext_index = -1;
  for (int i = 0; i < num_exts; i++) {
    if (strcmp(ext, exts[i].ext) == 0) {
      ext_index = i;
      break;
    }
  }

  if (ext_index != -1) {
    // Extension found
    pthread_mutex_lock( & mutex);

    // Check if folder is full
    if (exts[ext_index].count >= exts[ext_index].max) {
      char new_dir_path[MAX_PATH];
      int i = 2;
      while (1) {
        sprintf(new_dir_path, "categorized/%s (%d)", exts[ext_index].ext, i);
        struct stat st;
        if (stat(new_dir_path, & st) == -1) {
          // Directory does not exist, create it
          create_directory(new_dir_path);
          exts[ext_index].max = read_max("max.txt", exts[ext_index].ext);
          break;
        }
        i++;
      }
    }

    // Move file
    char * file_name = strrchr(info -> src_path, '/') + 1;
    sprintf(info -> dst_path, "categorized/%s/%s", exts[ext_index].ext, file_name);
  }

  void * moveFile(void * arg) {
    struct ThreadData * data = (struct ThreadData * ) arg;
    char * fileSrc = (char * ) malloc(sizeof(char) * MAX_FILENAME_LENGTH);
    char * fileDst = (char * ) malloc(sizeof(char) * MAX_FILENAME_LENGTH);
    char * folderDst = (char * ) malloc(sizeof(char) * MAX_FILENAME_LENGTH);
    char * extension = (char * ) malloc(sizeof(char) * MAX_EXTENSION_LENGTH);

    // Copy thread data to local variables
    strcpy(fileSrc, data -> fileSrc);
    strcpy(fileDst, data -> fileDst);
    strcpy(folderDst, data -> folderDst);
    strcpy(extension, data -> extension);

    // Free thread data memory
    free(data -> fileSrc);
    free(data -> fileDst);
    free(data -> folderDst);
    free(data -> extension);
    free(data);

    // Access folder
    accessFolder(folderDst);

    // Move file to destination folder
    if (rename(fileSrc, fileDst) != 0) {
      printf("Error moving file: %s\n", fileSrc);
      return NULL;
    }

    // Log file move
    char timestamp[20];
    getTimestamp(timestamp);
    char logMessage[LOG_MESSAGE_LENGTH];
    snprintf(logMessage, LOG_MESSAGE_LENGTH, "%s MOVED %s file : %s > %s\n", timestamp, extension, fileSrc, fileDst);
    writeToLog(logMessage);

    return NULL;

  }

  void createFolder(char * folderName) {
    char folderPath[MAX_FILENAME_LENGTH];
    snprintf(folderPath, MAX_FILENAME_LENGTH, "%s/%s", CATEGORY_FOLDER, folderName);

    // Create folder
    mkdir(folderPath, 0777);

    // Log folder creation
    char timestamp[20];
    getTimestamp(timestamp);
    char logMessage[LOG_MESSAGE_LENGTH];
    snprintf(logMessage, LOG_MESSAGE_LENGTH, "%s MADE %s\n", timestamp, folderName);
    writeToLog(logMessage);
  }

  void categorizeFiles() {
    // Initialize variables for reading extensions and max.txt
    FILE * extensionsFile = fopen("extensions.txt", "r");
    FILE * maxFile = fopen("max.txt", "r");
    char extension[MAX_EXTENSION_LENGTH];
    int maxCount;
  }

  // Initialize arrays for storing folder names and file counts
  char folderNames[MAX_EXTENSIONS][MAX_EXTENSION_LENGTH];
  int fileCounts[MAX_EXTENSIONS];
  int folderCount = 0;

  // Read extensions and max.txt
  while (fscanf(extensionsFile, "%s", extension) != EOF) {
    // Make folder name lowercase
    toLowercase(extension);

    // Create folder for extension
    createFolder(extension);
    strcpy(folderNames[folderCount], extension);

    // Read max count from max.txt
    if (fscanf(maxFile, "%d", & maxCount) == EOF) {
      maxCount = DEFAULT_MAX_COUNT;
    }

    // Count files and move them to folder
    int count = countFiles(extension);
    fileCounts[folderCount] = count;
    if (count > maxCount) {
      int folderIndex = 2;
      char folderName[MAX_EXTENSION_LENGTH];
      while (1) {
        snprintf(folderName, MAX_EXTENSION_LENGTH, "%s (%d)", extension, folderIndex);
        createFolder(folderName);
        folderIndex++;
        count = countFiles(folderName);
        fileCounts[folderCount + folderIndex - 3] = count;
        if (count < maxCount) {
          break;
        }
      }
      moveFilesToFolder(extension, folderName);
    } else {
      moveFilesToFolder(extension, extension);
    }

    folderCount++;
  }

  // Count files

}
closedir(d);
}
}

int main() {
  char * folder_name = "categorized";
  char * ext_file_name = "extensions.txt";
  char * max_file_name = "max.txt";
  char * src_folder_name = "files";

  // create categorized folder if not exist
  mkdir(folder_name, 0777);

  // read extensions from file
  char ** extensions = read_extensions(ext_file_name);

  // read max values from file
  int * max_values = read_max_values(max_file_name, extensions);

  // count number of extensions
  int num_extensions = 0;
  while (extensions[num_extensions] != NULL) {
    num_extensions++;
  }

  // create subfolders for each extension
  create_subfolders(folder_name, extensions, num_extensions);

  // process files and move/copy to respective extension folder
  categorize_files(src_folder_name, folder_name, extensions, max_values, num_extensions);

  // print number of files for each extension and for other
  print_num_files(folder_name, extensions, num_extensions);

  // free memory
  for (int i = 0; i < num_extensions; i++) {
    free(extensions[i]);
  }
  free(extensions);
  free(max_values);

  return 0;
}
