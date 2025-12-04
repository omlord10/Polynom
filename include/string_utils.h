#ifndef LAB3_STRING_UTILS_H
#define LAB3_STRING_UTILS_H

#include "../include/polynomial.h"

/*
 * Преобразует строку в многочлен.
 * Формат строки: "(c0,c1,...,cn)"
 * Пример: "(1,2,3)" соответствует многочлену 3x^2 + 2x + 1.
 *
 * [IN]      str     строка с коэффициентами
 * [IN]      modulo     модуль (modulo > 1)
 * [OUT]     pol     результирующий многочлен (coeffs выделяются заново)
 *
 * [RETURN]  POL_SUCCESS        — успех
 *           POL_NULL_PTR       — str == NULL или pol == NULL
 *           POL_INVALID_MODULO — modulo <= 1
 *           POL_INVALID_ARG    — некорректный формат строки
 *           POL_MEMORY_ERROR   — ошибка выделения памяти
 */
int str_to_pol(const char* str, ULL modulo, Polynomial* pol);

/*
 * Формирует строку вида "(c0,c1,...,cn)" по многочлену.
 * Память под строку выделяется внутри функции.
 *
 * [IN]      pol     исходный многочлен
 * [OUT]     str     указатель на результирующую строку (нужно освободить)
 *
 * [RETURN]  POL_SUCCESS        — успех
 *           POL_NULL_PTR       — pol == NULL или str == NULL
 *           POL_INVALID_ARG    — pol->coeffs == NULL
 *           POL_MEMORY_ERROR   — ошибка выделения памяти
 */
int pol_to_str(const Polynomial* pol, char** str);


#endif //LAB3_STRING_UTILS_H