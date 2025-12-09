#include "find_min_max.h"

#include <limits.h>

struct MinMax GetMinMax(int *array, unsigned int begin, unsigned int end){
  struct MinMax min_max;
  min_max.min = INT_MAX;
  min_max.max = INT_MIN;

  // your code here

  // Проходим по всем элементам в промежутке [begin, end)
  for (unsigned int i = begin; i < end; i++) {
    if (array[i] < min_max.min) {
      min_max.min = array[i]; // обновляем минимум
    }
    if (array[i] > min_max.max) {
      min_max.max = array[i]; // обновляем максимум
    }
  }

  return min_max;
}

// #include <stdio.h>
// #include "find_min_max.h"

// void print_array(int *array, unsigned int size) {
//     printf("Массив: [");
//     for (unsigned int i = 0; i < size; i++) {
//         printf("%d", array[i]);
//         if (i < size - 1) printf(", ");
//     }
//     printf("]\n");
// }

// int main() {
//     int arr[] = {1, 2, 8, 5, 4, 3, 7, 9, 6};
//     unsigned int size = sizeof(arr) / sizeof(arr[0]);
    
//     print_array(arr, size);
    
//     // Тест 1: Поиск в промежутке [2, 7)
//     struct MinMax result = GetMinMax(arr, 2, 7);
//     printf("Промежуток [2, 7): min = %d, max = %d\n", result.min, result.max);
    
//     // Тест 2: Поиск во всем массиве [0, size)
//     result = GetMinMax(arr, 0, size);
//     printf("Весь массив [0, %d): min = %d, max = %d\n", size, result.min, result.max);
    
//     // Тест 3: Поиск в маленьком промежутке [3, 5)
//     result = GetMinMax(arr, 3, 5);
//     printf("Промежуток [3, 5): min = %d, max = %d\n", result.min, result.max);
    
//     return 0;
// }