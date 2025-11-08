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
