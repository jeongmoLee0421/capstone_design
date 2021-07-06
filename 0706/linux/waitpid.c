#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){
    int status;
    pid_t pid = fork();

    if (pid == 0){
        sleep(15);
        return 244;
}
    else{
        while(!waitpid(-1, &status, WNOHANG)){ // return 0, if no child process have been terminated
            sleep(3);
            puts("sleep 3sec");
}
        if (WIFEXITED(status))
            printf("child send: %d\n", WEXITSTATUS(status));
}
    return 0;
}
