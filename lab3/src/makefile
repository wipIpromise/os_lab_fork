# CC=gcc                    # Компилятор C
# CFLAGS=-I.                # Флаги компиляции (поиск заголовков в текущей директории)
# TARGETS=sequential_min_max parallel_min_max  # Список целевых программ

# # Target по умолчанию (выполняется при запуске просто "make")
# all: $(TARGETS)

# # Target для создания последовательной версии программы
# sequential_min_max: utils.o find_min_max.o utils.h find_min_max.h sequential_min_max.c
# 	$(CC) -o $@ find_min_max.o utils.o sequential_min_max.c $(CFLAGS)
# # ↑ $@ - автоматическая переменная, заменяется на имя target'а (sequential_min_max)

# # Target для создания параллельной версии программы
# parallel_min_max: utils.o find_min_max.o utils.h find_min_max.h parallel_min_max.c
# 	$(CC) -o $@ utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

# # Создание object-файла utils.o
# utils.o: utils.c utils.h
# 	$(CC) -o $@ -c $< $(CFLAGS)
# # ↑ $@ - имя target'а (utils.o)
# # ↑ $< - первая зависимость (utils.c)

# # Создание object-файла find_min_max.o
# find_min_max.o: find_min_max.c find_min_max.h utils.h
# 	$(CC) -o $@ -c $< $(CFLAGS)

# # Target для очистки всех скомпилированных файлов
# clean:
# 	rm -f utils.o find_min_max.o $(TARGETS)
# # ↑ -f флаг - не выводить ошибку, если файлов нет

# # Target для пересборки всего (clean + all)
# rebuild: clean all

# # Target для отображения помощи
# help:
# 	@echo "Доступные команды:"
# 	@echo "  make all       - собрать все программы (по умолчанию)"
# 	@echo "  make sequential_min_max - собрать только последовательную версию"
# 	@echo "  make parallel_min_max   - собрать только параллельную версию"
# 	@echo "  make clean     - удалить все скомпилированные файлы"
# 	@echo "  make rebuild   - пересобрать все программы с нуля"
# 	@echo "  make help      - показать эту справку"

# # Указание, что эти targets не являются реальными файлами
# .PHONY: all clean rebuild help

CC=gcc
CFLAGS=-I.
TARGETS=sequential_min_max parallel_min_max exec_sequential  # Добавили exec_sequential

# Target по умолчанию
all: $(TARGETS)

# Последовательная версия
sequential_min_max: utils.o find_min_max.o utils.h find_min_max.h sequential_min_max.c
	$(CC) -o $@ find_min_max.o utils.o sequential_min_max.c $(CFLAGS)

# Параллельная версия
parallel_min_max: utils.o find_min_max.o utils.h find_min_max.h parallel_min_max.c
	$(CC) -o $@ utils.o find_min_max.o parallel_min_max.c $(CFLAGS)

# Программа для запуска через exec
exec_sequential: sequential_min_max exec_sequential.c
	$(CC) -o $@ exec_sequential.c $(CFLAGS)

# Object-файлы
utils.o: utils.c utils.h
	$(CC) -o $@ -c $< $(CFLAGS)

find_min_max.o: find_min_max.c find_min_max.h utils.h
	$(CC) -o $@ -c $< $(CFLAGS)

# Очистка
clean:
	rm -f utils.o find_min_max.o $(TARGETS) *.o

# Пересборка
rebuild: clean all

# Помощь
help:
	@echo "Доступные команды:"
	@echo "  make all             - собрать все программы"
	@echo "  make sequential_min_max - последовательная версия"
	@echo "  make parallel_min_max   - параллельная версия"
	@echo "  make exec_sequential    - программа для запуска через exec"
	@echo "  make clean           - удалить скомпилированные файлы"
	@echo "  make rebuild         - пересобрать всё"
	@echo "  make help            - эта справка"

# Пример запуска:
example:
	@echo "Пример использования:"
	@echo "  ./exec_sequential 42 100"

.PHONY: all clean rebuild help example