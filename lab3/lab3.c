//gcc lab3.c -lpthread
//2.	Отсортировать массив целых чисел при помощи параллельного алгоритма быстрой сортировки
// Программе передается количество потоков, которые будут использованы. Далее происходит считывание: 
// количество элементов в векторе, а затем каждый элемент.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>


pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int max_threads;

int check() {
        int result;
        if (max_threads <= 0){
                return 0;
        }
        pthread_mutex_lock(&mutex);

        if (max_threads > 0){
                result = 1;
        }
        else{
                result = 0;
        }
        max_threads--;

        pthread_mutex_unlock(&mutex);
        return result;
}


typedef struct vector{
        int size;
        int *data;
}vector;


typedef struct args{
        int* array;
        int left;
        int right;
}args;


void *quick_sort(void *args){
        int *array = ((struct args *)args)->array;
        int left = ((struct args *)args)->left;
        int right = ((struct args *)args)->right;
        int i = left, j = right, tmp;
        int t1 = 0, t2 = 0;
        int pivot = array[(left + right) / 2];
        while(i <= j) {
                for (; array[i] < pivot; i++);
                for (; array[j] > pivot; j--);
                if (i <= j) {
                        tmp = array[i];
                        array[i++] = array[j];
                        array[j--] = tmp;
                }
        }

        pthread_t thread1 = 0;
        pthread_t thread2 = 0;

        if (left < j){
                if(!check(args)){
                        struct args *args1 = malloc(sizeof(args));
                        args1->array = array;
                        args1->left = left;
                        args1->right = j--;
                        quick_sort(args1);
                }
                else{
                        t1++;
                        struct args *args1 = malloc(sizeof(args));
                        args1->array = array;
                        args1->left = left;
                        args1->right = j--;
                        int status = pthread_create(&thread1, NULL, quick_sort, args1);
                        if (status != 0){
                                printf("CAN'T CREATE THREAD");
                                return NULL;
                        }
                }
        }

        if (i < right){
                if(!check(args)){
                        struct args *args1 = malloc(sizeof(args));
                        args1->array = array;
                        args1->left = i++;
                        args1->right = right;
                        quick_sort(args1);
                }
                else{
                        t2++;
                        struct args *args1 = malloc(sizeof(args));
                        args1->array = array;
                        args1->left = i++;
                        args1->right = right;
                        int status = pthread_create(&thread2, NULL, quick_sort, args1);
                        if (status != 0){
                                printf("CAN'T CREATE THREAD");
                                return NULL;
                        }
                }
        }


        if (t1 != 0){
                int status1 = pthread_join(thread1, 0);
                if (status1 != 0){
                        printf("JOIN ERROR");
                        return NULL;
                }
        }
        if (t2 != 0){
                int status2 = pthread_join(thread2, 0);
                if (status2 != 0){
                        printf("JOIN ERROR");
                        return NULL;
                }
        }

        free(args);

        return NULL;
}


int main(int argc, char* argv[]){
        if (argc != 2 || atoi(argv[1]) < 1){
                printf("WRONG INPUT\n");
                return 1;
        }

        max_threads = atoi(argv[1]) - 1;

        vector *vector = malloc(sizeof(vector));
        int s;
        scanf("%d", &s);
        vector->data = (int *)malloc(sizeof(int) * s);
        int x;
        for (int i = 0; i < s; i++){
                scanf("%d", &x);
                vector->data[i] = x;
        }
        printf("--Entered vector:--\n");
        for (int i = 0; i < s; i++) {
                printf("%d\n", vector->data[i]);
        }
        vector->size = s;

        args *args1 = malloc(sizeof(args));
        args1->array = vector->data;
        args1->left = 0;
        args1->right = vector->size - 1;
        quick_sort(args1);

        printf("--Result:--\n");
        for (int i = 0; i < vector->size; i++) {
                printf("%d\n", vector->data[i]);
        }

        free(vector);
        pthread_mutex_destroy(&mutex);
        return 0;
}