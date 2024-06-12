#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

int main(int argc, char *args[]) {
    if (argc < 3) {
        fprintf(stderr, "%s\n", args[0]);
        exit(EXIT_FAILURE);
    }

    int stoptime = atoi(args[1]);

    pid_t pid = fork();
    if (pid == -1) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    } else if (pid == 0) {
        // Child process
        execvp(args[2], &args[2]);
        perror("execvp");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        sleep(stoptime);
        if (kill(pid, SIGTERM) == -1) {
            perror("kill");
            exit(EXIT_FAILURE);
        }
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid");
            exit(EXIT_FAILURE);
        } else {
            if (WIFEXITED(status)) {
                //  if Child process exited normally
                exit(WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                //if child process was terminated by a signal
                exit(WTERMSIG(status));
            } else {
                // if any error occurr
                exit(EXIT_FAILURE);
            }
        }
    }

    return 0;
}
