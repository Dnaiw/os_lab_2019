#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
    pid_t child_pid = fork();

    if (child_pid < 0) {
        perror("fork failed");
        return 1;
    }

    if (child_pid == 0) {
         printf("Child process with PID %d is exiting...\n", getpid());
        exit(0);
    } else {
         printf("Parent process with PID %d created child process with PID %d\n", getpid(), child_pid);

        printf("Parent process is sleeping for 10 seconds. Check for zombie process.\n");
        sleep(10);

        printf("Parent process is exiting without calling wait().\n");
    }

    return 0;
}