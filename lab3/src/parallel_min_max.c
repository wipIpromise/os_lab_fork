#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

int main(int argc, char **argv) {
  int seed = -1;
  int array_size = -1;
  int pnum = -1;
  bool with_files = false;

  // Разбор аргументов командной строки
  while (true) {
    int current_optind = optind ? optind : 1;

    static struct option options[] = {{"seed", required_argument, 0, 0},
                                      {"array_size", required_argument, 0, 0},
                                      {"pnum", required_argument, 0, 0},
                                      {"by_files", no_argument, 0, 'f'},
                                      {0, 0, 0, 0}};

    int option_index = 0;
    int c = getopt_long(argc, argv, "f", options, &option_index);

    if (c == -1) break;

    switch (c) {
      case 0:
        switch (option_index) {
          case 0:
            seed = atoi(optarg);
            // Проверка корректности seed
            if (seed <= 0) {
                printf("seed must be a positive number\n");
                return 1;
            }
            break;
          case 1:
            array_size = atoi(optarg);
            // Проверка корректности array_size
            if (array_size <= 0) {
                printf("array_size must be a positive number\n");
                return 1;
            }
            break;
          case 2:
            pnum = atoi(optarg);
            // Проверка корректности pnum
            if (pnum <= 0) {
                printf("pnum must be a positive number\n");
                return 1;
            }
            break;
          case 3:
            with_files = true;
            break;

          default:  // Исправлено: было defalut
            printf("Index %d is out of options\n", option_index);
        }
        break;
      case 'f':
        with_files = true;
        break;

      case '?':
        break;

      default:
        printf("getopt returned character code 0%o?\n", c);
    }
  }

  if (optind < argc) {
    printf("Has at least one no option argument\n");
    return 1;
  }

  // Проверка наличия всех необходимых аргументов
  if (seed == -1 || array_size == -1 || pnum == -1) {
    printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--by_files]\n",
           argv[0]);
    return 1;
  }

  // Генерация массива
  int *array = malloc(sizeof(int) * array_size);
  GenerateArray(array, array_size, seed);
  int active_child_processes = 0;

  // Массивы для pipe или имен файлов
  int pipes[2 * pnum]; // Для pipe: [read1, write1, read2, write2, ...]
  char filenames[pnum][50]; // Для имен файлов

  // Подготовка коммуникационных каналов
  if (!with_files) {
    // Создание pipe для каждого дочернего процесса
    for (int i = 0; i < pnum; i++) {
      if (pipe(pipes + i * 2) < 0) {
        printf("Pipe creation failed!\n");
        free(array);
        return 1;
      }
    }
  } else {
    // Подготовка имен файлов
    for (int i = 0; i < pnum; i++) {
      sprintf(filenames[i], "min_max_%d.txt", i);
    }
  }

  // Начало отсчета времени
  struct timeval start_time;
  gettimeofday(&start_time, NULL);

  // Создание дочерних процессов
  for (int i = 0; i < pnum; i++) {
    pid_t child_pid = fork();
    
    if (child_pid >= 0) {
      // Успешный fork
      active_child_processes += 1;
      
      if (child_pid == 0) {
        // КОД ДОЧЕРНЕГО ПРОЦЕССА
        
        // Вычисление границ части массива для этого процесса
        int chunk_size = array_size / pnum;
        int start = i * chunk_size;
        int end = (i == pnum - 1) ? array_size : (i + 1) * chunk_size;
        
        // Поиск минимума и максимума в своей части
        struct MinMax local_min_max = GetMinMax(array, start, end);
        
        if (with_files) {
          // Использование файлов для передачи результатов
          FILE *file = fopen(filenames[i], "w");
          if (file != NULL) {
            fprintf(file, "%d %d", local_min_max.min, local_min_max.max);
            fclose(file);
          } else {
            printf("Failed to open file %s\n", filenames[i]);
          }
        } else {
          // Использование pipe для передачи результатов
          close(pipes[i * 2]); // Закрываем read end в дочернем процессе
          
          // Запись результатов в pipe
          write(pipes[i * 2 + 1], &local_min_max.min, sizeof(int));
          write(pipes[i * 2 + 1], &local_min_max.max, sizeof(int));
          
          close(pipes[i * 2 + 1]); // Закрываем write end после записи
        }
        
        free(array);
        exit(0); // Завершение дочернего процесса
      }
      
    } else {
      printf("Fork failed!\n");
      free(array);
      return 1;
    }
  }

  // КОД РОДИТЕЛЬСКОГО ПРОЦЕССА
  
  // Ожидание завершения всех дочерних процессов
  while (active_child_processes > 0) {
    wait(NULL); // Ожидание завершения любого дочернего процесса
    active_child_processes -= 1;
  }

  // Агрегация результатов от всех процессов
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  for (int i = 0; i < pnum; i++) {
    int min = INT_MAX;
    int max = INT_MIN;

    if (with_files) {
      // Чтение результатов из файлов
      FILE *file = fopen(filenames[i], "r");
      if (file != NULL) {
        fscanf(file, "%d %d", &min, &max);
        fclose(file);
        // Удаление временного файла
        remove(filenames[i]);
      } else {
        printf("Failed to read from file %s\n", filenames[i]);
      }
    } else {
      // Чтение результатов из pipe
      close(pipes[i * 2 + 1]); // Закрываем write end в родительском процессе
      
      read(pipes[i * 2], &min, sizeof(int));
      read(pipes[i * 2], &max, sizeof(int));
      
      close(pipes[i * 2]); // Закрываем read end после чтения
    }

    // Обновление общего минимума и максимума
    if (min < min_max.min) min_max.min = min;
    if (max > min_max.max) min_max.max = max;
  }

  // Конец отсчета времени
  struct timeval finish_time;
  gettimeofday(&finish_time, NULL);

  // Вычисление затраченного времени в миллисекундах
  double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
  elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

  // Освобождение памяти
  free(array);

  // Вывод результатов
  printf("Min: %d\n", min_max.min);
  printf("Max: %d\n", min_max.max);
  printf("Elapsed time: %fms\n", elapsed_time);
  printf("Number of processes: %d\n", pnum);
  
  fflush(NULL);
  return 0;
}
