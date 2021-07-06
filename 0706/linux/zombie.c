#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    pid_t pid = fork();

    if (pid == 0){
        puts("hi i am a child proc");
}
    else{
        printf("child process id: %d\n", pid);
        sleep(30);
}

    if (pid == 0)
        puts("end child process");
    else
        puts("end parent process");

    return 0;
}
