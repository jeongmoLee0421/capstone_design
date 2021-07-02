#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#define BUF_SIZE 100
void error_handling(char* message);

int main(void){
    int rfd, wfd;
    char buf[BUF_SIZE];

    rfd = open("data.txt", O_RDONLY);
    wfd = open("copy.txt", O_WRONLY|O_CREAT|O_TRUNC);

    if (rfd == -1 || wfd == -1)
        error_handling("open() error");

    if (read(rfd, buf, sizeof(buf)) == -1)
        error_handling("read() error");
    if (write(wfd, buf, sizeof(buf)) == -1)
        error_handling("write() error");

    close(rfd); close(wfd);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
