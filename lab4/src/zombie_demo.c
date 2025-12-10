
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>

void demonstrate_zombie() {
    printf("=== Демонстрация создания зомби-процесса ===\n");
    
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("fork failed");
        exit(1);
    }
    
    if (pid == 0) {
        // ДОЧЕРНИЙ ПРОЦЕСС
        printf("Дочерний процесс (PID: %d) запущен\n", getpid());
        printf("Дочерний процесс завершается через 3 секунды...\n");
        sleep(3);
        printf("Дочерний процесс завершился и станет зомби\n");
        exit(0);
        
    } else {
        // РОДИТЕЛЬСКИЙ ПРОЦЕСС
        printf("Родительский процесс (PID: %d) создал дочерний (PID: %d)\n", 
               getpid(), pid);
        printf("Родительский процесс НЕ вызывает wait(), а спит 10 секунд\n");
        printf("В это время дочерний процесс станет зомби\n");
        printf("Для проверки выполните в другом терминале:\n");
        printf("  ps aux | grep -E '(PID|%d)'\n", pid);
        
        // НЕ вызываем wait() - это создаст зомби!
        sleep(10);
        
        printf("\nРодительский процесс просыпается\n");
        printf("Зомби-процесс все еще существует (проверьте ps)\n");
        
        // Теперь подождем зомби
        printf("Теперь вызываем wait() для сбора зомби...\n");
        int status;
        waitpid(pid, &status, 0);
        printf("Зомби-процесс собран\n");
        
        printf("Проверьте еще раз (зомби должен исчезнуть):\n");
        printf("  ps aux | grep -E '(PID|%d)'\n", pid);
        sleep(3);
    }
}

void demonstrate_avoiding_zombie() {
    printf("\n\n=== Как избежать зомби-процессов ===\n");
    
    printf("Способ 1: Использование wait() в родительском процессе\n");
    pid_t pid1 = fork();
    
    if (pid1 == 0) {
        printf("  Дочерний процесс 1 завершается\n");
        exit(0);
    } else {
        int status;
        waitpid(pid1, &status, 0);  // Собираем дочерний процесс
        printf("  Родитель собрал дочерний процесс 1\n");
    }
    
    printf("\nСпособ 2: Игнорирование SIGCHLD\n");
    signal(SIGCHLD, SIG_IGN);  // Автоматически собирает дочерние процессы
    
    pid_t pid2 = fork();
    if (pid2 == 0) {
        printf("  Дочерний процесс 2 завершается\n");
        exit(0);
    } else {
        sleep(1);  // Даем время дочернему процессу завершиться
        printf("  Дочерний процесс 2 автоматически собран\n");
    }
    
    printf("\nСпособ 3: Двойной fork\n");
    pid_t pid3 = fork();
    
    if (pid3 == 0) {
        // Первый дочерний
        pid_t pid4 = fork();
        
        if (pid4 == 0) {
            // Второй дочерний (внук)
            printf("  Процесс-внук работает\n");
            sleep(2);
            printf("  Процесс-внук завершается\n");
            exit(0);
        } else {
            // Первый дочерний немедленно завершается
            printf("  Первый дочерний завершается, оставляя внука сиротой\n");
            exit(0);
        }
    } else {
        // Родитель собирает первого дочернего
        wait(NULL);
        printf("  Родитель собрал первого дочернего\n");
        printf("  Внук теперь сирота и будет собран init процессом\n");
        sleep(3);
    }
}

void demonstrate_zombie_apocalypse() {
    printf("\n\n=== Демонстрация 'зомби-апокалипсиса' ===\n");
    printf("Создаем много зомби-процессов...\n");
    
    int num_zombies = 10;
    pid_t pids[num_zombies];
    
    for (int i = 0; i < num_zombies; i++) {
        pid_t pid = fork();
        
        if (pid == 0) {
            // Дочерний процесс
            printf("  Зомби %d (PID: %d) создан\n", i+1, getpid());
            exit(0);
        } else {
            pids[i] = pid;
        }
    }
    
    printf("\nСоздано %d зомби-процессов\n", num_zombies);
    printf("Они занимают записи в таблице процессов\n");
    printf("Нажмите Enter для очистки зомби...\n");
    getchar();
    
    // Собираем всех зомби
    for (int i = 0; i < num_zombies; i++) {
        waitpid(pids[i], NULL, 0);
    }
    
    printf("Все зомби собраны\n");
}

void show_system_info() {
    printf("\n=== Информация о системе ===\n");
    
    // Проверка ограничений системы
    system("echo 'Максимальное количество процессов:'");
    system("cat /proc/sys/kernel/pid_max");
    
    printf("\nТекущие процессы (первые 10):\n");
    system("ps aux --sort=-pid | head -11");
}

int main(int argc, char **argv) {
    printf("=========================================\n");
    printf("   ДЕМОНСТРАЦИЯ ЗОМБИ-ПРОЦЕССОВ\n");
    printf("=========================================\n");
    
    if (argc > 1 && strcmp(argv[1], "--simple") == 0) {
        demonstrate_zombie();
    } else if (argc > 1 && strcmp(argv[1], "--apocalypse") == 0) {
        demonstrate_zombie_apocalypse();
    } else {
        demonstrate_zombie();
        demonstrate_avoiding_zombie();
        show_system_info();
    }
    
    printf("\n=========================================\n");
    printf("Демонстрация завершена\n");
    return 0;
}