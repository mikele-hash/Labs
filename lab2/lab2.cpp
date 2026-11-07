#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <getopt.h>
#include <limits.h>

#define ARRAY_SIZE 100000000  

int *array = NULL;
int global_min = INT_MAX;
int global_max = INT_MIN;


double get_time_ms(struct timespec *start, struct timespec *end) {
    return (end->tv_sec - start->tv_sec) * 1000.0 +
           (end->tv_nsec - start->tv_nsec) / 1000000.0;
}

void generate_array() {
    array = (int *)malloc(ARRAY_SIZE * sizeof(int));
    if (array == NULL) {
        fprintf(stderr, "Ошибка выделения памяти!\n");
        exit(1);
    }
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        array[i] = (i * 7919 + 104729) % 2000000 - 1000000;
    }
}

void sequential_find(int *min_val, int *max_val) {
    *min_val = INT_MAX;
    *max_val = INT_MIN;
    
    for (int i = 0; i < ARRAY_SIZE; ++i) {
        if (array[i] < *min_val) *min_val = array[i];
        if (array[i] > *max_val) *max_val = array[i];
    }
}

typedef struct {
    int start_idx;
    int end_idx;
    int local_min;
    int local_max;
} ThreadArgs;

void *parallel_worker(void *_args) {
    ThreadArgs *args = (ThreadArgs *)_args;
    
    args->local_min = INT_MAX;
    args->local_max = INT_MIN;
    
    for (int i = args->start_idx; i < args->end_idx; ++i) {
        if (array[i] < args->local_min) args->local_min = array[i];
        if (array[i] > args->local_max) args->local_max = array[i];
    }
    
    return NULL;
}

void parallel_find(int n_threads, int *min_val, int *max_val) {
    pthread_t *threads = (pthread_t *)malloc(n_threads * sizeof(pthread_t));
    ThreadArgs *args = (ThreadArgs *)malloc(n_threads * sizeof(ThreadArgs));
    
    int chunk_size = ARRAY_SIZE / n_threads;
    
    for (int i = 0; i < n_threads; ++i) {
        args[i].start_idx = i * chunk_size;
        args[i].end_idx = (i == n_threads - 1) ? ARRAY_SIZE : (i + 1) * chunk_size;
        pthread_create(&threads[i], NULL, parallel_worker, &args[i]);
    }
    
    for (int i = 0; i < n_threads; ++i) {
        pthread_join(threads[i], NULL);
    }
    
    *min_val = args[0].local_min;
    *max_val = args[0].local_max;
    
    for (int i = 1; i < n_threads; ++i) {
        if (args[i].local_min < *min_val) *min_val = args[i].local_min;
        if (args[i].local_max > *max_val) *max_val = args[i].local_max;
    }
    
    free(threads);
    free(args);
}

int main(int argc, char **argv) {
    int n_threads = 1;
    int opt;
    
    while ((opt = getopt(argc, argv, "t:")) != -1) {
        if (opt == 't') {
            n_threads = atoi(optarg);
            if (n_threads < 1) n_threads = 1;
        }
    }
    printf("Размер массива: %d элементов\n", ARRAY_SIZE);
    printf("Количество потоков: %d\n\n", n_threads);
    
    generate_array();
    
    struct timespec t1, t2;
    int seq_min, seq_max, par_min, par_max;
    
    clock_gettime(CLOCK_MONOTONIC, &t1);
    sequential_find(&seq_min, &seq_max);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    double seq_time = get_time_ms(&t1, &t2);
    
    printf("--- Последовательная версия ---\n");
    printf("Минимум: %d\n", seq_min);
    printf("Максимум: %d\n", seq_max);
    printf("Время: %.2f мс\n\n", seq_time);
    
    clock_gettime(CLOCK_MONOTONIC, &t1);
    parallel_find(n_threads, &par_min, &par_max);
    clock_gettime(CLOCK_MONOTONIC, &t2);
    double par_time = get_time_ms(&t1, &t2);
    
    printf("--- Параллельная версия (%d потоков) ---\n", n_threads);
    printf("Минимум: %d\n", par_min);
    printf("Максимум: %d\n", par_max);
    printf("Время: %.2f мс\n\n", par_time);
    
    free(array);
    
    return 0;
}
