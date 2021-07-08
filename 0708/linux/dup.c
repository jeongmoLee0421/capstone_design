#include <stdio.h>
#include <unistd.h>

int main(int argc, char* argv[]){
    int cfd1, cfd2;
    char str1[] = "hi~\n";
    char str2[] = "nice day\n";

    // duplicate fd
    cfd1 = dup(1);
    cfd2 = dup2(cfd1, 7);

    printf("fd1 = %d, fd2 = %d\n", cfd1, cfd2);
    write(cfd1, str1, sizeof(str1));
    write(cfd2, str2, sizeof(str2));

    close(cfd1);
    close(cfd2);

    // the output is possible because fd remains
    write(1, str1, sizeof(str1));
    close(1);

    // the output is impossible because there is no fd left
    write(1, str2, sizeof(str2));
    return 0;
}
