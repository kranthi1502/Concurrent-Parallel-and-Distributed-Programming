#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("wrong input command\n");
    exit(1);
  }

  struct timeval begin, end;
  struct rusage usage;

  gettimeofday(&begin, NULL);
  getrusage(RUSAGE_SELF, &usage);
  pid_t pid = fork();
  if (pid == 0) {
    execvp(argv[1], &argv[1]);
    printf("Invalid command used\n");
    exit(1);
  } else if (pid > 0) {
    wait(NULL);
    getrusage(RUSAGE_CHILDREN, &usage);
    gettimeofday(&end, NULL);
    double user_cpu_time = usage.ru_utime.tv_sec + usage.ru_utime.tv_usec * 0.000001;
    double system_cpu_time = usage.ru_stime.tv_sec + usage.ru_stime.tv_usec * 0.000001;
    double elapsed_wallclock_time = (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec) * 0.000001;
    printf("User CPU time: %.6f seconds\n", user_cpu_time);
    printf("System CPU time: %.6f seconds\n", system_cpu_time);
    printf("Elapsed wall-clock time: %.6f seconds\n", elapsed_wallclock_time);
  } else {
    perror("Failed to create child process");
    exit(1);
  }



  return 0;
}

