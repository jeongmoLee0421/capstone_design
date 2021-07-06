#include <stdio.h>
#include <unistd.h>
#define BUF_SIZE 30

int main(int argc, char* argv[]){
    int fds1[2], fds2[2];
    char* c_str[] = {"child1", "child2", "child3"};
    char* p_str[] = {"parent1", "parent2", "parent3"};
    char buf[BUF_SIZE];
    int str_len, i;
    pid_t pid;

    // child -> parent = fds1
    // parent -> child = fds2
    pipe(fds1), pipe(fds2);

    pid = fork();
    if (pid == 0){
        for (i=0; i<3; i++){
            write(fds1[1], c_str[i], sizeof(c_str[i]));
            str_len = read(fds2[0], buf, BUF_SIZE);
            buf[str_len] = 0;
            printf("child proc output %d: %s\n", i+1, buf);
}
}
    else{
        for (i=0; i<3; i++){
            str_len = read(fds1[0], buf, BUF_SIZE);
            buf[str_len] = 0;
            printf("parent proc output %d: %s\n", i+1, buf);
            write(fds2[1], p_str[i], sizeof(p_str[i]));
            sleep(1);
}
}

    return 0;
}
