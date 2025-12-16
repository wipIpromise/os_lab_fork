# # Makefile для process_memory.c
# # Компилятор и флаги
# CC = gcc
# CFLAGS = -Wall -Wextra -Wno-implicit-int -Wno-return-type -Wno-implicit-function-declaration

# # Имя исполняемого файла
# TARGET = process_memory

# # Цель по умолчанию
# all: $(TARGET)

# # Основная цель - компиляция программы
# $(TARGET): process_memory.c
# 	$(CC) $(CFLAGS) -o $(TARGET) process_memory.c

# # Флаги для отладки
# debug: CFLAGS += -g -O0
# debug: $(TARGET)

# # Флаги для релиза
# release: CFLAGS += -O2 -DNDEBUG
# release: $(TARGET)

# # Цель для запуска программы
# run: $(TARGET)
# 	@echo "Запуск программы $(TARGET)..."
# 	@echo "========================================"
# 	./$(TARGET)
# 	@echo "========================================"

# # Цель для просмотра символов в исполняемом файле
# symbols: $(TARGET)
# 	@echo "Символы etext, edata, end в исполняемом файле:"
# 	@nm $(TARGET) | grep -E "\s(etext|edata|end)$$" || echo "Символы не найдены"
# 	@echo ""
# 	@echo "Разделы (sections) исполняемого файла:"
# 	@readelf -S $(TARGET) | grep -E "(Name|\.text|\.data|\.bss)"

# # Цель для просмотра карты памяти процесса
# memmap: $(TARGET)
# 	@echo "Запуск программы и просмотр карты памяти..."
# 	@./$(TARGET) &
# 	@sleep 1
# 	@PID=$$(pidof $(TARGET)); \
# 	if [ -n "$$PID" ]; then \
# 		echo "PID процесса: $$PID"; \
# 		echo "Карта памяти процесса:"; \
# 		cat /proc/$$PID/maps | head -20; \
# 		kill $$PID 2>/dev/null || true; \
# 	else \
# 		echo "Процесс не найден"; \
# 	fi

# # Цель для тестирования
# test: $(TARGET)
# 	@echo "Тест 1: Запуск программы"
# 	./$(TARGET)
# 	@echo ""
# 	@echo "Тест 2: Проверка возвращаемого значения"
# 	./$(TARGET) > /dev/null && echo "Программа завершилась успешно" || echo "Ошибка выполнения"

# # Цель для очистки
# clean:
# 	rm -f $(TARGET) $(TARGET).o core *.o

# # Цель для справки
# help:
# 	@echo "Доступные цели:"
# 	@echo "  make all        - собрать программу (по умолчанию)"
# 	@echo "  make debug      - собрать с отладочной информацией"
# 	@echo "  make release    - собрать с оптимизацией"
# 	@echo "  make run        - собрать и запустить программу"
# 	@echo "  make symbols    - показать символы etext, edata, end"
# 	@echo "  make memmap     - показать карту памяти процесса"
# 	@echo "  make test       - запустить тесты"
# 	@echo "  make clean      - удалить скомпилированные файлы"
# 	@echo "  make help       - показать эту справку"

# # Объявляем фиктивные цели
# .PHONY: all debug release run symbols memmap test clean help

# Makefile for parallel_sum project
CC = gcc
CFLAGS = -Wall -Wextra -pthread -I.
TARGET = parallel_sum

# Source files
SRCS = parallel_sum.c sum.c utils.c
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Build executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

# Compile each .c file to .o
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(TARGET) $(OBJS)

# Run tests
test_small: $(TARGET)
	@echo "=== Small test (1000 elements, 4 threads) ==="
	./$(TARGET) --threads_num 4 --seed 42 --array_size 1000

test_medium: $(TARGET)
	@echo "=== Medium test (100000 elements, 8 threads) ==="
	./$(TARGET) --threads_num 8 --seed 123 --array_size 100000

test_large: $(TARGET)
	@echo "=== Large test (1000000 elements, 16 threads) ==="
	./$(TARGET) --threads_num 16 --seed 456 --array_size 1000000

test_all: test_small test_medium test_large

# Comparison with sequential version
seq_test: $(TARGET)
	@echo "=== Sequential vs Parallel (1 thread) ==="
	./$(TARGET) --threads_num 1 --seed 42 --array_size 1000000
	@echo ""
	@echo "=== Parallel (4 threads) ==="
	./$(TARGET) --threads_num 4 --seed 42 --array_size 1000000

# Help
help:
	@echo "Available targets:"
	@echo "  make all         - build program"
	@echo "  make clean       - remove compiled files"
	@echo "  make test_small  - run small test"
	@echo "  make test_medium - run medium test"
	@echo "  make test_large  - run large test"
	@echo "  make test_all    - run all tests"
	@echo "  make seq_test    - compare sequential vs parallel"
	@echo "  make help        - show this help"

.PHONY: all clean test_small test_medium test_large test_all seq_test help