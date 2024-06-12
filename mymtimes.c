#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <string.h>


void dfs(char *path, int *modified_files) {
  DIR *directory = opendir(path);
  if(directory==NULL){
        return;
    }

  time_t current_time = time(NULL);
  struct dirent *entry;
  while ((entry = readdir(directory)) != NULL) {
    if(strcmp(entry->d_name,"..")==0 || strcmp(entry->d_name,".")==0) continue;
    if (entry->d_type == DT_REG) {
      char path_n[4096];
      strcpy(path_n,path);
      strcat(path_n,"/");
      strcat(path_n,entry->d_name);
      struct stat st;
      if (stat(path_n, &st) == -1) {
        perror("stat");
        exit(1);
      }

      if (st.st_mtime >= current_time - 3600 * 24) {
        int hours = (current_time - st.st_mtime) / 3600;
        modified_files[hours]++;
      }
    } else if (entry->d_type == DT_DIR) {
      char path_n[4096];
      strcpy(path_n,path);
      strcat(path_n,"/");
      strcat(path_n,entry->d_name);
      dfs(path_n, modified_files);
    }
  }

  closedir(directory);
}

int main(int argc, char *argv[]) {
  char *path = ".";
  if (argc == 2) {
    path = argv[1];
  }
  time_t now = time(NULL);
  time_t temp_now=now;
  int modified_files[24] = {0};
  dfs(path, modified_files);
  int i;
  for (i = 0; i < 24; i++) {
    temp_now=now-(3600*(24-i));
    printf("%.24s: %d\n", ctime(&temp_now), modified_files[24-i-1]);
  }

  return 0;
}
