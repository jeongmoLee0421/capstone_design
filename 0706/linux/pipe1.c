#include <stdio.h>
#include <unistd.h>
#define BUF_SIZE 30

int main(int argc, char* argv[]){
    int fds[2];
    char str[] = "who are you?";
    char buf[BUF_SIZE];
    int str_len;
    pid_t pid;

    pipe(fds); // contains two file descriptors

    pid = fork(); // copy pipe's file descriptors
    if (pid == 0)
        write(fds[1], str, sizeof(str));
    else{
        int str_len = read(fds[0], buf, BUF_SIZE);
        buf[str_len] = 0;
        puts(buf);
}

    return 0;
}
