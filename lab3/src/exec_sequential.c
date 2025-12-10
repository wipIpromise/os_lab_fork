#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s seed arraysize\n", argv[0]);
        return 1;
    }

    pid_t child_pid = fork();
    
    if (child_pid < 0) {
        // Ошибка при создании процесса
        perror("fork failed");
        return 1;
    }
    
    if (child_pid == 0) {
        // ДОЧЕРНИЙ ПРОЦЕСС
        // Используем exec для запуска sequential_min_max
        
        printf("Child process (PID: %d) is starting sequential_min_max...\n", getpid());
        
        // Создаем массив аргументов для exec
        char *exec_args[] = {
            "./sequential_min_max",  // имя программы (argv[0])
            argv[1],                  // seed
            argv[2],                  // array_size
            NULL                     // обязательный NULL в конце
        };
        
        // Вариант 1: execv - передаем полный путь и массив аргументов
        execv("./sequential_min_max", exec_args);
        
        // execv возвращает управление только в случае ошибки
        perror("execv failed");
        return 1;
        
    } else {
        // РОДИТЕЛЬСКИЙ ПРОЦЕСС
        printf("Parent process (PID: %d) created child process (PID: %d)\n", 
               getpid(), child_pid);
        printf("Waiting for child process to complete...\n");
        
        // Ждем завершения дочернего процесса
        int status;
        waitpid(child_pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("Child process exited with status: %d\n", WEXITSTATUS(status));
        } else if (WIFSIGNALED(status)) {
            printf("Child process terminated by signal: %d\n", WTERMSIG(status));
        }
        
        printf("Parent process completed.\n");
    }
    
    return 0;
}
