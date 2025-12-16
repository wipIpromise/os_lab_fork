#include <ctype.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <getopt.h>

#include "find_min_max.h"
#include "utils.h"

// Глобальная переменная для хранения PID дочерних процессов
pid_t *child_pids = NULL;
int pnum = 0;
bool timeout_expired = false;

// Обработчик сигнала SIGALRM (таймаут)
void timeout_handler(int sig) {
    printf("\nTimeout expired! Sending SIGKILL to all child processes...\n");
    timeout_expired = true;
    
    if (child_pids != NULL) {
        for (int i = 0; i < pnum; i++) {
            if (child_pids[i] > 0) {
                kill(child_pids[i], SIGKILL);
            }
        }
    }
}

// Функция для ожидания завершения дочерних процессов с таймаутом
void wait_for_children_with_timeout(int timeout) {
    int active_children = pnum;
    
    if (timeout > 0) {
        // Устанавливаем обработчик для SIGALRM
        signal(SIGALRM, timeout_handler);
        
        // Устанавливаем таймер
        alarm(timeout);
    }
    
    while (active_children > 0) {
        int status;
        pid_t finished_pid = waitpid(-1, &status, WNOHANG);
        
        if (finished_pid > 0) {
            // Один из дочерних процессов завершился
            active_children--;
            
            // Находим и обнуляем PID завершенного процесса
            for (int i = 0; i < pnum; i++) {
                if (child_pids[i] == finished_pid) {
                    child_pids[i] = 0;
                    break;
                }
            }
            
            if (WIFEXITED(status)) {
                printf("Child process %d exited with status %d\n", 
                       finished_pid, WEXITSTATUS(status));
            } else if (WIFSIGNALED(status)) {
                printf("Child process %d terminated by signal %d\n", 
                       finished_pid, WTERMSIG(status));
            }
            
        } else if (finished_pid == 0) {
            // Дочерние процессы еще работают
            if (timeout_expired) {
                // Таймаут истек, процессы были убиты
                // Ждем их окончательного завершения
                sleep(1);
            } else {
                // Даем процессорам время для работы
                usleep(1000); // 1 миллисекунда
            }
        } else {
            // Ошибка waitpid
            if (errno == ECHILD) {
                // Нет дочерних процессов
                break;
            }
            perror("waitpid error");
            break;
        }
    }
    
    // Отключаем таймер, если он еще не сработал
    if (timeout > 0) {
        alarm(0);
    }
}

int main(int argc, char **argv) {
    int seed = -1;
    int array_size = -1;
    int timeout = 0;  // 0 означает "нет таймаута"
    bool with_files = false;

    // Разбор аргументов командной строки
    while (true) {
        int current_optind = optind ? optind : 1;

        static struct option options[] = {
            {"seed", required_argument, 0, 0},
            {"array_size", required_argument, 0, 0},
            {"pnum", required_argument, 0, 0},
            {"timeout", required_argument, 0, 0},
            {"by_files", no_argument, 0, 'f'},
            {0, 0, 0, 0}
        };

        int option_index = 0;
        int c = getopt_long(argc, argv, "f", options, &option_index);

        if (c == -1) break;

        switch (c) {
            case 0:
                switch (option_index) {
                    case 0:
                        seed = atoi(optarg);
                        if (seed <= 0) {
                            printf("seed must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 1:
                        array_size = atoi(optarg);
                        if (array_size <= 0) {
                            printf("array_size must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 2:
                        pnum = atoi(optarg);
                        if (pnum <= 0) {
                            printf("pnum must be a positive number\n");
                            return 1;
                        }
                        break;
                    case 3:
                        timeout = atoi(optarg);
                        if (timeout <= 0) {
                            printf("timeout must be a positive number (seconds)\n");
                            return 1;
                        }
                        printf("Timeout set to %d seconds\n", timeout);
                        break;
                    case 4:
                        with_files = true;
                        break;
                    default:
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

    // Проверка обязательных аргументов
    if (seed == -1 || array_size == -1 || pnum == -1) {
        printf("Usage: %s --seed \"num\" --array_size \"num\" --pnum \"num\" [--timeout \"num\"] [--by_files]\n",
               argv[0]);
        return 1;
    }

    // Выделение памяти для хранения PID дочерних процессов
    child_pids = malloc(pnum * sizeof(pid_t));
    if (child_pids == NULL) {
        printf("Memory allocation failed for child PIDs\n");
        return 1;
    }

    // Генерация массива
    int *array = malloc(sizeof(int) * array_size);
    GenerateArray(array, array_size, seed);

    // Массивы для pipe или имен файлов
    int pipes[2 * pnum];
    char filenames[pnum][50];

    // Подготовка коммуникационных каналов
    if (!with_files) {
        for (int i = 0; i < pnum; i++) {
            if (pipe(pipes + i * 2) < 0) {
                printf("Pipe creation failed!\n");
                free(array);
                free(child_pids);
                return 1;
            }
        }
    }

    // Начало отсчета времени
    struct timeval start_time;
    gettimeofday(&start_time, NULL);

    // Создание дочерних процессов
    for (int i = 0; i < pnum; i++) {
        pid_t pid = fork();
        
        if (pid >= 0) {
            if (pid == 0) {
                // ДОЧЕРНИЙ ПРОЦЕСС
                
                // Вычисление границ части массива
                int chunk_size = array_size / pnum;
                int start = i * chunk_size;
                int end = (i == pnum - 1) ? array_size : (i + 1) * chunk_size;
                
                // Поиск минимума и максимума в своей части
                struct MinMax local_min_max = GetMinMax(array, start, end);
                
                if (with_files) {
                    // Использование файлов
                    sprintf(filenames[i], "min_max_%d.txt", i);
                    FILE *file = fopen(filenames[i], "w");
                    if (file != NULL) {
                        fprintf(file, "%d %d", local_min_max.min, local_min_max.max);
                        fclose(file);
                    }
                } else {
                    // Использование pipe
                    close(pipes[i * 2]);
                    write(pipes[i * 2 + 1], &local_min_max.min, sizeof(int));
                    write(pipes[i * 2 + 1], &local_min_max.max, sizeof(int));
                    close(pipes[i * 2 + 1]);
                }
                
                free(array);
                exit(0);
                
            } else {
                // РОДИТЕЛЬСКИЙ ПРОЦЕСС
                child_pids[i] = pid;
            }
            
        } else {
            printf("Fork failed!\n");
            free(array);
            free(child_pids);
            return 1;
        }
    }

    // Ожидание завершения дочерних процессов с возможным таймаутом
    printf("Parent process waiting for %d child processes", pnum);
    if (timeout > 0) {
        printf(" (timeout: %d seconds)", timeout);
    }
    printf("...\n");
    
    wait_for_children_with_timeout(timeout);

    // Сбор результатов
    struct MinMax min_max;
    min_max.min = INT_MAX;
    min_max.max = INT_MIN;

    int results_received = 0;
    for (int i = 0; i < pnum; i++) {
        int min = INT_MAX;
        int max = INT_MIN;

        if (with_files) {
            sprintf(filenames[i], "min_max_%d.txt", i);
            FILE *file = fopen(filenames[i], "r");
            if (file != NULL) {
                if (fscanf(file, "%d %d", &min, &max) == 2) {
                    results_received++;
                }
                fclose(file);
                remove(filenames[i]);
            }
        } else {
            close(pipes[i * 2 + 1]);
            if (read(pipes[i * 2], &min, sizeof(int)) > 0 &&
                read(pipes[i * 2], &max, sizeof(int)) > 0) {
                results_received++;
            }
            close(pipes[i * 2]);
        }

        if (min < min_max.min) min_max.min = min;
        if (max > min_max.max) min_max.max = max;
    }

    // Конец отсчета времени
    struct timeval finish_time;
    gettimeofday(&finish_time, NULL);

    double elapsed_time = (finish_time.tv_sec - start_time.tv_sec) * 1000.0;
    elapsed_time += (finish_time.tv_usec - start_time.tv_usec) / 1000.0;

    // Вывод результатов
    printf("\n=== Results ===\n");
    printf("Results received from %d out of %d processes\n", results_received, pnum);
    
    if (timeout_expired) {
        printf("WARNING: Timeout expired! Some processes were terminated.\n");
    }
    
    if (results_received > 0) {
        printf("Min: %d\n", min_max.min);
        printf("Max: %d\n", min_max.max);
    } else {
        printf("No results received (all processes may have been terminated)\n");
        min_max.min = 0;
        min_max.max = 0;
    }
    
    printf("Elapsed time: %.2fms\n", elapsed_time);

    // Освобождение памяти
    free(array);
    free(child_pids);

    return 0;
}