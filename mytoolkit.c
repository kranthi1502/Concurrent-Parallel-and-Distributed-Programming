#include <signal.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>


#define MAX_LINE 80
#define EXITCMD "myexit"
#define MAX_PIPES 10
#define TIMEOUTCMD "mytimeout"

int redirection(char* input_file,char* output_file,char** args,int lengthc);
void pipe_handler(char **commands,int pipes_num);
void execute_command(char **args);
void execute_pwd(char* command){
    char cwd[1024];
    printf("%s\n",command);
    if(getcwd(cwd,sizeof(cwd))==NULL){
        perror("Error while retrieving the path");
        }
    else{
        printf("The current working directory path is:%s\n",cwd);
        }
        
}
void pipe_handler(char *exe_cmd[],int pipes_num){

    int pipes[MAX_PIPES][2]; 
    pid_t pid;
    int i,j;
        //creaating pipes
    for (i = 0; i < pipes_num - 1; i++) {
        if (pipe(pipes[i]) < 0) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
    }

    for ( i = 0; i < pipes_num; i++) {
        pid = fork();
        if (pid < 0) {
            perror("fork");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            if (i > 0) {
                if (dup2(pipes[i - 1][0], STDIN_FILENO) < 0) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            if (i < pipes_num - 1) {
                if (dup2(pipes[i][1], STDOUT_FILENO) < 0) {
                    perror("dup2");
                    exit(EXIT_FAILURE);
                }
            }

            for ( j = 0; j < pipes_num - 1; j++) {
                close(pipes[j][0]);
                close(pipes[j][1]);
            }

            char *args[MAX_LINE / 2 + 1];
            args[0] = strtok(exe_cmd[i], " ");
            int num_args = 0;
            while (args[num_args] != NULL) {
                num_args++;
                args[num_args] = strtok(NULL, " ");
            }
           execvp(args[0], args);
        } else {

            if (i > 0) {
                close(pipes[i - 1][0]);
                close(pipes[i - 1][1]);
            }
            if (i < pipes_num - 1) {
                close(pipes[i][1]);
            }
            int status;
            if (waitpid(pid,&status,0)==-1){
                perror("\nwaitpid\n");
            }
        }
    }

   
}
int redirection(char* input_file,char* output_file,char** args,int lengthc){
    int i;
    char *temp[80];
    int index=0;
    int flag_in=0;
    int flag_out=0;

    for (i = 0; i < lengthc; i++) {
        if (strcmp(args[i], "<") == 0) {
            if (i + 1 >= lengthc) {
                fprintf(stderr, "Missing input file\n");
                exit(EXIT_FAILURE);
            }
            input_file = args[i + 1];
            flag_in=1;
            i++;
        } else if (strcmp(args[i], ">") == 0) {
            
            if (i + 1 >= lengthc) {
                fprintf(stderr, "Missing output file\n");
                exit(EXIT_FAILURE);
            }
            
            output_file = args[i + 1];
            flag_out=1;
            i++;
        } else{
            temp[index] = args[i];
            index++;
        }
    }
    temp[index]=NULL;
    pid_t pid = fork();
    int status;
   
    if(pid==0){
        if(input_file!=NULL && flag_in==1){
            int fd_in = open(input_file, O_RDONLY);
            if (fd_in < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd_in, STDIN_FILENO);
        }
        if(output_file!=NULL && flag_out==1){
            int fd_out = open(output_file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd_out < 0) {
                perror("open");
                exit(EXIT_FAILURE);
            }
            dup2(fd_out, STDOUT_FILENO);
        }
        execvp(temp[0],temp);
        exit(1);
    }
    else if(pid==-1){
        perror("fork");
        exit(1);
    }
    else{
        waitpid(pid, &status, 0);
    }
    return 0;
}
void execute_command(char **args){
    pid_t pid;
    //printf("args\n",args[0]);
    pid=fork();
    if(pid==0){
       if(execvp(args[0],args)==-1){
        perror("Command not found");
        exit(EXIT_SUCCESS);
       }
    }
    else if(pid<0){
         perror("fork");
        exit(EXIT_FAILURE);
    }
    else{
        wait(NULL);
    }
}
int main(void)
{
    char* args[MAX_LINE];// array of arguments
    int untill_exit = 1; // flag to determine when to exit program
    char *fbuff;
	//shell creation

    while (untill_exit) {
            printf("$");
            fflush(stdout); // flush output buffer

            char input[MAX_PIPES * MAX_LINE]; // input buffer
            fbuff=fgets(input, MAX_LINE, stdin);
             //EOF--ctrl+D
            if(fbuff==NULL){
                putchar('\n');
                // break;
                exit(EXIT_SUCCESS);
            }
            if (strcmp(input,"\n")==0)
            continue;
            
            input[strcspn(input, "\n")] = '\0';
            char pipe_input[MAX_PIPES * MAX_LINE]="";
            strcat(pipe_input,input);

            // tokenize input into arguments
            char *token = strtok(input, " \n");
            int i = 0;
            int pipes_num=0,pipe_flag=0,redirect=0;;
            while (token != NULL) {
                args[i++] = token;
                args[i-1][strlen(args[i-1])] = '\0';
                if(strcmp(args[i-1],">")==0 || strcmp(args[i-1],"<")==0){
                    redirect=1;
                }
                if(strcmp(args[i-1],"|")==0){
                    pipe_flag=1;
                    pipes_num++;
                }
                token = strtok(NULL, " \n");
            }
            args[i] = NULL;
            char cwd[1024];
            //I/O REDIRECTION VARIABLES
            char* input_file = NULL;
            char* output_file = NULL;

        // printf("\ncount :%d",pipes_num);
        // printf("\nflag :%d",pipe_flag);
        // printf("\nredirect %d\n",redirect);

        //PWD - PATH
        if(strcmp(args[0],"mypwd")==0){
            execute_pwd(args[0]);
        }
        //exit
        else if (strcmp(args[0], "myexit") == 0) {
                untill_exit = 0;
                //break;
        }

        //change_directory
         else if(strcmp(args[0],"mycd")==0 || strcmp(args[0],"cd")==0 ){
            if(getcwd(cwd,sizeof(cwd))==NULL){
                perror("Cannot get the path");
            }
            else if(args[1]==NULL){
                chdir(getenv("HOME"));
               printf("The Home directory path is:%s\n",getcwd(cwd,sizeof(cwd))) ;

            }
            else{
                if(strcmp(args[1],"..")==0){
                    chdir("..");
                    printf("The next level directory path is:%s\n",getcwd(cwd,sizeof(cwd))) ;
                }
                else if(strcmp(args[1],"/")==0){
                    chdir("/");
                    printf("The Root directory is:%s\n",getcwd(cwd,sizeof(cwd)));
                }
                else if(strcmp(args[1],"~")==0){
                    chdir(getenv("HOME"));
                    printf("The Home directory is:%s\n",getcwd(cwd,sizeof(cwd)));
                }
                else{
                    if(chdir(args[1]) == 0) {
						printf("current directory:: %s\n", getcwd(cwd, sizeof(cwd)));
					} else {
						printf("unknown %s\n", args[1]);
					}
                }
            }
        }
        
    
    //i/o redirection
        else if(redirect==1){
            // printf("\nredirection\n");
            redirection(input_file,output_file,args,i);
        }

        //PIPES
        else if(pipe_flag==1){
            // printf("pipe input: %s\n",pipe_input);
            // printf("\npipe_execution\n");
            char *exe_cmd[10];
            int pipe_count = 0;
            pipe_count = 0;
            exe_cmd[pipe_count] = strtok(pipe_input, "|");
	        //printf("\ncommads:%s\n",commands[num_commands]);
            while (exe_cmd[pipe_count] != NULL) {
                pipe_count++;
                exe_cmd[pipe_count] = strtok(NULL, "|");
	            //printf("\ncommads:%s\n",commands[num_commands]);
                }
                pipe_handler(exe_cmd,pipe_count);
                }
            else{
                execute_command(args);
                }
        }
         // check if "exit" command was entered
         
           
       
        return 0;
    }

    
