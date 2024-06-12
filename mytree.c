#define _DEFAULT_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
void dfs(char *path,int height){

 DIR *directory; 
    struct dirent *entry;
    directory=opendir(path);
    if(!directory){
        printf("Error while opening the file/directory");
        return;
    }
    char newpath[1000];
    while((entry=readdir(directory))!=NULL){
        if(entry->d_name[0]=='.') continue;
        int i;
        for(i=0;i<=height;i++){
            if(i%4==0){
                printf("|");
            }else{
                printf(" ");
            }
        }
        if(entry->d_type==DT_REG){
            printf("---%s\n",entry->d_name);
        }
        if(entry->d_type==DT_DIR){
            strcpy(newpath,path);
            strcat(newpath,"/");
            strcat(newpath,entry->d_name);
            DIR *temp_d=opendir(newpath);
            if (!temp_d){
                printf("---%s%c [%s]\n",entry->d_name,'/',"Error opening the directory");
            }else{
                closedir(temp_d);
                printf("---%s%c\n",entry->d_name,'/');
                dfs(newpath,height+4);

            }
        }
    }
    closedir(directory);
    return;
}
int main(int argc, char *argv[]){
    char *path=".";
    if(argc>1){
        path=argv[1];
    }
    printf("%s\n",path);
    dfs(path,0);
    return 0;
}