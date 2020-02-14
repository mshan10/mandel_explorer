#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <unistd.h>
#include <ctype.h>

void start(char **);
void waiting(int *);
void create_processes(int argc, const char *[]);

void create_processes(int argc, const char *argv[]) {
    if (argc != 2 || atoi(argv[1]) == 0) {
        printf("usage: ./mandelmovie <number of processes>\n"); 
        exit(1); 
    }

    int max_processes = atoi(argv[1]);
    int running_processes = 0;
    float a = .01;
    float b = .0001;
    int curr_iter = 0;
    while (curr_iter < 50) {
        if (running_processes < max_processes) {
            char *args[16] = {"./mandel", "-s", NULL, "-m", "1000", "-x", "-0.74529", "-y", "0.113075", "-H", "1200", "-W", "1200", "-o", NULL, NULL};
            
            // Create output file name
            char buffer[BUFSIZ];
            sprintf(buffer, "%d", curr_iter);
            char file[] = "/tmp/mandel";
            strcat(file, buffer);
            strcat(file, ".bmp");
            args[14] = file;

            // Create scale
            char scale[BUFSIZ];
            sprintf(scale, "%f", a); 
            args[2] = scale;

            // Start new process
            start(args);
            
            // Update scale
            a *= exp(log(b/a)/51);
            running_processes += 1;
            curr_iter += 1;
        } else {
            waiting(&running_processes);
        }
    }
}

void start(char **argv) {
    pid_t pid = fork();
    if (pid < 0) {
        printf("unable to fork: %s\n", strerror(errno));
    } else if (pid == 0) {
        // Child process
        execvp(argv[0], argv);
        printf("unable to exec: %s\n", strerror(errno));
    }
}

void waiting(int *running_processes) {
    int status, pid;
    pid = wait(&status);
    if (pid != -1 && WIFEXITED(status))
        *running_processes = *running_processes - 1;
    else
        printf("process %d exited abnormally with signal %d: %s\n", pid, WTERMSIG(status), strsignal(status));
}

int main(int argc, char const *argv[]) {
    create_processes(argc, argv);
    return 0;
}
