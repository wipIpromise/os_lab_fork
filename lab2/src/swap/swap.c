#include "swap.h"

void Swap(char *a, char *b)
{
	char temp = *a;  // Сохраняем значение по адресу a
    *a = *b;         // Записываем значение b в a
    *b = temp;       // Восстанавливаем сохраненное значение в b
}
