#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <getopt.h>
#include <pthread.h>

#include "utils.h"
#include "sum.h"

int main(int argc, char **argv) {
    // Параметры по умолчанию
    uint32_t threads_num = 0;
    uint32_t array_size = 0;
    uint32_t seed = 0;
    
    // Парсинг аргументов командной строки
    while (1) {
        static struct option options[] = {
            {"threads_num", required_argument, 0, 0},
            {"array_size", required_argument, 0, 1},
            {"seed", required_argument, 0, 2},
            {0, 0, 0, 0}
        };
        
        int option_index = 0;
        int c = getopt_long(argc, argv, "", options, &option_index);
        
        if (c == -1) break;
        
        switch (c) {
            case 0:
                threads_num = atoi(optarg);
                if (threads_num <= 0) {
                    fprintf(stderr, "threads_num must be positive\n");
                    return 1;
                }
                break;
            case 1:
                array_size = atoi(optarg);
                if (array_size <= 0) {
                    fprintf(stderr, "array_size must be positive\n");
                    return 1;
                }
                break;
            case 2:
                seed = atoi(optarg);
                if (seed <= 0) {
                    fprintf(stderr, "seed must be positive\n");
                    return 1;
                }
                break;
            default:
                fprintf(stderr, "Usage: %s --threads_num \"num\" --seed \"num\" --array_size \"num\"\n", argv[0]);
                return 1;
        }
    }
    
    // Проверка наличия всех параметров
    if (threads_num == 0 || array_size == 0 || seed == 0) {
        fprintf(stderr, "Usage: %s --threads_num \"num\" --seed \"num\" --array_size \"num\"\n", argv[0]);
        return 1;
    }
    
    // Выделение памяти для массива
    int *array = malloc(sizeof(int) * array_size);
    if (array == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Генерация массива
    GenerateArray(array, array_size, seed);
    
    // Подготовка аргументов для потоков
    struct SumArgs args[threads_num];
    pthread_t threads[threads_num];
    
    // Разделение массива между потоками
    int chunk_size = array_size / threads_num;
    for (uint32_t i = 0; i < threads_num; i++) {
        args[i].array = array;
        args[i].begin = i * chunk_size;
        args[i].end = (i == threads_num - 1) ? array_size : (i + 1) * chunk_size;
    }
    
    // Начало отсчета времени
    struct timeval start_time;
    gettimeofday(&start_time, NULL);
    
    // Создание потоков
    for (uint32_t i = 0; i < threads_num; i++) {
        if (pthread_create(&threads[i], NULL, ThreadSum, (void *)&args[i]) != 0) {
            fprintf(stderr, "Error: pthread_create failed!\n");
            free(array);
            return 1;
        }
    }
    
    // Ожидание завершения потоков и сбор результатов
    int total_sum = 0;
    for (uint32_t i = 0; i < threads_num; i++) {
        int *thread_sum = NULL;
        if (pthread_join(threads[i], (void **)&thread_sum) != 0) {
            fprintf(stderr, "Error: pthread_join failed!\n");
            free(array);
            return 1;
        }
        total_sum += *thread_sum;
        free(thread_sum);
    }
    
    // Конец отсчета времени
    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);
    
    // Вычисление времени выполнения
    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;
    
    // Проверка результата (последовательный подсчет для верификации)
    int sequential_sum = 0;
    for (uint32_t i = 0; i < array_size; i++) {
        sequential_sum += array[i];
    }
    
    // Вывод результатов
    printf("\n=== Parallel Sum Results ===\n");
    printf("Array size: %u\n", array_size);
    printf("Threads: %u\n", threads_num);
    printf("Seed: %u\n", seed);
    printf("Parallel sum: %d\n", total_sum);
    printf("Sequential sum: %d\n", sequential_sum);
    printf("Elapsed time: %.2f ms\n", elapsed_time);
    
    if (total_sum == sequential_sum) {
        printf("✓ Results match!\n");
    } else {
        printf("✗ Results DO NOT match!\n");
    }
    
    // Освобождение памяти
    free(array);
    
    return 0;
}