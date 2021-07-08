#include <stdio.h>
#include <fcntl.h>

int main(void){
    FILE* fp;
    int fd = open("data.dat", O_WRONLY|O_CREAT|O_TRUNC); // write mode
    if (fd == -1){
        fputs("file open error", stdout);
        return -1;
}

    // if the modes of the file descriptor and the file pointer do not match, a segmentation fault occurs.
    fp = fdopen(fd, "w"); // convert fd to fp
    fputs("network c programming\n", fp);
    fclose(fp); // Closing a file using the file pointer completely closes the file.
    //write(fd, "123", 3);
    //fclose(fp);
    return 0;
}
