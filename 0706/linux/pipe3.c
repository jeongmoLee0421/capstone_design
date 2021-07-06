#include <stdio.h>
#include <unistd.h>
#define BUF_SIZE 30

int main(int argc, char* argv[]){
    int fds1[2], fds2[2];
    char str1[] = "who are you?";
    char str2[] = "thank you";
    char buf[BUF_SIZE];
    int str_len;
    pid_t pid;

    // child -> parent = fds1
    // parent -> child = fds2
    pipe(fds1), pipe(fds2);

    pid = fork();
    if (pid == 0){
        write(fds1[1], str1, sizeof(str1));
        str_len = read(fds2[0], buf, BUF_SIZE);
        buf[str_len] = 0;
        printf("child proc output: %s\n", buf);
}
    else{
        str_len = read(fds1[0], buf, BUF_SIZE);
        buf[str_len] = 0;
        printf("parent proc output: %s\n", buf);
        write(fds2[1], str2, sizeof(str2));
        sleep(3);
}

    return 0;
}
