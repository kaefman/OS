#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>

//родительский процесс
void parent(int *pipe1, int *pipe2, int fd){
    close(pipe1[0]);
    close(pipe2[1]);
    char* errorMess = "ERROR\n";
    while(1){
        char str[100] = {'\0'};
        fgets(str, 100, stdin);
        int size = strlen(str);
        if (size == 0) {
            write(pipe1[1], &size, sizeof(int));
            break;
        }
        if (size == 1) {
            if (str[0] == '\n') {
                continue;
            }
        }
        str[size - 1] = '\0';
        write(pipe1[1], &size, sizeof(int));
        write(pipe1[1], str, sizeof(char) * size);
        bool result;
        if (read(pipe2[0], &result, sizeof(bool)) != sizeof(bool)) {
            fprintf(stderr, "read error in parent process\n");
            close(fd);
            close(pipe1[1]);
            close(pipe2[0]);
            exit(-1);
        }
        if(!result){
            write(fd, errorMess, sizeof(char) * 6);
        }
    }
    wait(NULL);
    close(fd);
    close(pipe1[1]);
    close(pipe2[0]);
}

//дочерний процесс
void child(int *pipe1, int *pipe2, int fd){
    close(pipe1[1]);
    close(pipe2[0]);
    while (1) {
        int size;
        bool result;
        char str[100];
        if (read(pipe1[0], &size, sizeof(int)) != sizeof(int)) {
            fprintf(stderr, "read error in child process\n");
            close(fd);
            close(pipe1[0]);
            close(pipe2[1]);
            exit(-1);
        }
        if (size == 0) {
            break;
        }
        if (read(pipe1[0], str, sizeof(char) * size) != sizeof(char) * size) {
            fprintf(stderr, "read error in child process\n");
            close(fd);
            close(pipe1[0]);
            close(pipe2[1]);
            exit(-2);
        }
        
        if(str[0] < 'A' || str[0] > 'Z' || size == 0){
            result = false;
            write(pipe2[1], &result, sizeof(bool));
        } else {
            result = true;
            write(pipe2[1], &result, sizeof(bool));
            write(fd, str, sizeof(char) * size);
            write(fd, "\n", sizeof(char));
        }
    }
    close(fd);
    close(pipe1[0]);
    close(pipe2[1]);
}

int main(){
    char filename[30] = {'\0'};
    fgets(filename, 30, stdin);
    size_t size = strlen(filename);
    if (size == 0 || (size == 1 && filename[0] == '\n')) {
        fprintf(stderr, "incorrect file name\n");
        exit(-1);
    }
    filename[size - 1] = '\0';
    int fd = open(filename, O_CREAT | O_APPEND | O_WRONLY, S_IWUSR | S_IRUSR);
    int fd1[2], fd2[2];
    int pid;
    if(pipe(fd1) == -1){
        perror("pipe1 error");
    }
    if(pipe(fd2) == -1){
        perror("pipe2 error");
    }
    pid = fork();
    if(pid == -1){
        perror("FORK");
    } else if(pid == 0){
        child(fd1, fd2, fd);
    } else {
        parent(fd1, fd2, fd);
    }
    return 0;
}