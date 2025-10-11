#!/bin/bash

echo "=== Building Dynamic Library ==="
# Создаем динамическую библиотеку
gcc -c -fPIC revert_string.c -o revert_string.o
gcc -shared -o librevert_string.so revert_string.o

echo "=== Building Main Program ==="
# Компилируем основную программу с динамической библиотекой
gcc main.c -L. -lrevert_string -o revert_string_dynamic

echo "=== Building Test Program ==="
# Компилируем тестовую программу с той же динамической библиотекой
# Важно: указываем путь к заголовочным файлам и библиотекам
gcc tests.c -I. -L. -lrevert_string -lcunit -o test_revert

echo "=== Setting Library Path ==="
export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
echo "LD_LIBRARY_PATH set to: $LD_LIBRARY_PATH"

echo "=== Build Complete ==="
echo "Main program: ./revert_string_dynamic"
echo "Test program: ./test_revert"
echo ""
echo "To run tests: ./test_revert"
echo "To run main: ./revert_string_dynamic 'Your string here'"