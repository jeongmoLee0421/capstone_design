#include <stdio.h>
#include <stdlib.h>
#define BUF_SIZE 100
void error_handling(char* message);

int main(void){
    FILE* rstream;
    FILE* wstream;
    char buf[BUF_SIZE];
    int numRead, numWrite;

    if ((rstream = fopen("data.txt", "r")) == NULL)
        error_handling("fopen() error");
    if ((wstream = fopen("copy.txt", "w")) == NULL)
        error_handling("fopen() error");

    numRead = fread(buf, sizeof(char), sizeof(buf), rstream);
    /*printf("%d\n", numRead);
    if (numRead != sizeof(buf))
        error_handling("fread() error");*/

    numWrite = fwrite(buf, sizeof(char), sizeof(buf), wstream);
    /*if (numWrite != sizeof(buf))
        error_handling("fwrite() wrror");*/

    fclose(rstream); fclose(wstream);
    return 0;
}

void error_handling(char* message){
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
