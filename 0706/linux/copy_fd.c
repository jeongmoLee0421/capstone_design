#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <signal.h>
#include <sys/wait.h>

void error_handling(char* message);
void read_childproc(int sig);

int main(int argc, char* argv[]){
    int sock_fd;
    pid_t pid;
    struct sigaction act;

    sock_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (sock_fd == -1)
        error_handling("socket() error");
    
    act.sa_handler = read_childproc;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    sigaction(SIGCHLD, &act, 0);

    pid = fork();

    if (pid == -1)
        exit(1);

    // same socket file descriptor
    if (pid == 0){
        printf("child fd: %d\n", sock_fd);
        return 12;
}
    else{
        printf("parent fd: %d\n", sock_fd);
}

    return 0;
}

void read_childproc(int sig){
    int status;
    pid_t id = waitpid(-1, &status, WNOHANG);

    if (WIFEXITED(status)){
        printf("remove proc id: %d\n", id);
        printf("child send: %d\n", WEXITSTATUS(status));
}
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
