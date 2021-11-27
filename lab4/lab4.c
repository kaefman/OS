/* Родительский процесс создает дочерний процесс. Первой строкой пользователь в консоль
 * родительского процесса вводит имя файла, которое будет использовано для открытия File 
 * с таким именем на запись. Родительский процесс принимает от пользователя строки произвольной 
 * длины и пересылает их в pipe1. Процесс child проверяет строки на валидность правилу. 
 * Если строка соответствует правилу, то она выводится в стандартный поток вывода дочернего 
 * процесса, иначе в pipe2 выводится информация об ошибке. Родительский процесс полученные от 
 * child ошибки выводит в стандартный поток вывода.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>

int readString(char** str) {    // считывает строку (до '\n') и возвращает ее размер
    int size = 0;
    int capacity = 10;
    char a = '\0';

    if (!((*str) = malloc(sizeof(char) * capacity))) {
        write(STDERR_FILENO, "MEM ERR\n", sizeof(char) * 8);
        exit(-1);
    }

    while (read(0, &a, sizeof(char)) == 1) {
        if (a == '\n') {
            break;
        }

        if (size == capacity) {
            capacity = capacity * 3 / 2;
            if (!((*str) = realloc((*str), sizeof(char) * capacity))) {
                write(STDERR_FILENO, "MEM ERR\n", sizeof(char) * 8);
                exit(-1);
            }
        }

        (*str)[size] = a;
        ++size;
    }
    if (size == 0) {
        free((*str));
        (*str) = NULL;
        return -1;
    }
    if (!((*str) = realloc((*str), sizeof(char) * (size + 1)))) {
        write(STDERR_FILENO, "MEM ERR\n", sizeof(char) * 8);
        exit(-1);
    }
    (*str)[size] = '\0';
    return size;
}

void parent(char* data, int fd) {
    while (1) {
        char* str;
        int size = readString(&str);
        data[0] = size;
        data[1] = -1;
        if (size == 0) {
            data[2] = '\n';
            break;
        }
        memcpy(data + 2, str, size + 1);
        
        dup2(fd, 1);
        while (data[1] != 1 && data[1] != 0) {}
        if (data[1] == 1) {
            printf("%s\n", str);
        } 
        else {
            printf("%s\n", "ERROR");            
        }
    }
    wait(NULL);
    close(fd);
}

void child(char* data) {
    while (1) {
        while (data[2] == '\0') {}

        char* str = data + 2;
        int size = data[0];
        if (size == 0) {
            break;
        }

        if(str[0] < 'A' || str[0] > 'Z'){
            data[1] = 0;
        } 
        else {
            data[1] = 1;
        }
        data[2] = '\0';
    }
}

int main () {
    char* mfile = malloc(sizeof(char) * 100);
    char* file = malloc(sizeof(char) * 100);
    scanf("%s", mfile);
    scanf("%s", file);

    int outFD, mFD;
    if ((outFD = open(file, O_CREAT | O_WRONLY, S_IWUSR | S_IRUSR)) == -1) {
        fprintf(stderr, "Open file error\n");
        return -1;
    }
    if ((mFD = open(mfile, O_CREAT | O_RDWR, S_IWUSR | S_IRUSR)) == -1) {
        fprintf(stderr, "Open memory file error\n");
        return -1;
    }
    int pid;

    if (ftruncate(mFD, 100)) {
        perror("ftruncate");
        return -1;
    }

    char* ptr;
    if ((ptr = (char*) mmap(NULL, 100, PROT_READ | PROT_WRITE, MAP_SHARED, mFD, 0)) == MAP_FAILED) {
        perror("Map");
        return -1;
    }
    ptr[2] = '\0';
    close(mFD);

    pid = fork();
    if (pid == -1) {
        perror("FORK");
    } 
    else if (pid == 0) {
        child(ptr);
    } 
    else {
        parent(ptr, outFD);
    }

    munmap(ptr, 100);
    close(outFD);
    return 0;
}
