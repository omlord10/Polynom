#ifndef LAB3_POLYNOMIAL_H
#define LAB3_POLYNOMIAL_H

#include <stddef.h>  // size_t
#include <stdlib.h>  // calloc
#include <limits.h>  // LLONG_MAX
#include <stdio.h>   // printf
#include <string.h> // strcspn
#include <time.h> // random generator


typedef unsigned long long ULL;

typedef struct Polynomial
{
    ULL *coeffs;    // массив коэффициентов: coeffs[i] соответствует x^i
    size_t degree;  // степень многочлена
    ULL modulo;     // характеристика кольца Z_modulo, modulo > 1
} Polynomial;

enum POL_Error
{
    POL_SUCCESS = 0,      /* успех */
    POL_NULL_PTR,         /* указатель на Polynomial или другой аргумент == NULL */
    POL_INVALID_MODULO,   /* неверный модуль (modulo <= 1) */
    POL_MEMORY_ERROR,     /* ошибка выделения памяти */
    POL_MODULO_MISMATCH,  /* несоответствие модулей у двух многочленов */
    POL_ZERO_DIV,         /* деление на нулевой многочлен */
    POL_NO_INVERSE,       /* нет мультипликативного обратного для старшего коэффициента */
    POL_BUFFER_SMALL,     /* буфер для строкового представления слишком мал */
    POL_SYNTAX_ERROR,     /* синтаксическая ошибка при парсинг строки */
    POL_INVALID_ARG       /* Некорректный аргумент */
};

/*----------------- ВСПОМОГАТЕЛЬНЫЕ ОПЕРАЦИИ -----------------*/

/*
 * Перевыделяет память под коэффициенты многочлена, если текущей ёмкости недостаточно.
 * Гарантирует, что в R->coeffs будет как минимум (required_degree + 1) элементов.
 * Если память уже выделена и достаточна, ничего не делает.
 *
 * [IN/OUT]  R               указатель на структуру Polynomial
 * [IN]      required_degree требуемая степень (массив размера required_degree + 1)
 *
 * [OUT]     R               R->coeffs имеет ёмкость не менее required_degree + 1
 *
 * [RETURN]  POL_SUCCESS        — успех или память уже достаточна
 *           POL_MEMORY_ERROR   — ошибка выделения памяти
 *
 * [WARNING] Функция не проверяет R на NULL; вызывающая сторона должна гарантировать
 *           валидность указателя.
 */
int realloc_coeffs(Polynomial* R, size_t required_degree);


/*
 * Устанавливает основные параметры многочлена (степень и модуль).
 * Не выделяет и не освобождает память под коэффициенты.
 *
 * [IN/OUT]  R       указатель на структуру Polynomial
 * [IN]      deg     новая степень многочлена
 * [IN]      modulo  характеристика кольца; должно быть > 1
 *
 * [OUT]     R       R->degree и R->modulo установлены в заданные значения
 *
 * [RETURN]  POL_SUCCESS        — успех
 *
 * [WARNING] Функция не проверяет R на NULL и не валидирует modulo;
 *           вызывающая сторона должна гарантировать корректность аргументов.
 *           Не изменяет массив коэффициентов R->coeffs.
 */
int set_pol_params(Polynomial *R, size_t deg, ULL modulo);


/*
 * Расширенный алгоритм Евклида для вычисления НОД(a, b) и коэффициентов Безу.
 * Находит такие целые x и y, что a*x + b*y = НОД(a, b).
 *
 * [IN]      a   первое число (беззнаковое)
 * [IN]      b   второе число (беззнаковое)
 * [OUT]     x   указатель для сохранения первого коэффициента Безу (знаковый)
 * [OUT]     y   указатель для сохранения второго коэффициента Безу (знаковый)
 *
 * [RETURN]  НОД(a, b) — наибольший общий делитель a и b
 *
 * [NOTE]    Для работы с беззнаковыми a, b коэффициенты x, y вычисляются в знаковом
 *           типе long long, что может приводить к предупреждениям компилятора
 *           при больших значениях аргументов.
 * [NOTE]    Если a и b равны 0, возвращает 0 и устанавливает x = 0, y = 0.
 */
ULL egcd(ULL a, ULL b, long long *x, long long *y);


/*
 * Вычисляет мультипликативный обратный элемент к a по модулю m.
 * Находит такое число inv, что (a * inv) ≡ 1 (mod m).
 *
 * [IN]      a   элемент, для которого ищется обратный
 * [IN]      m   модуль (должен быть > 1)
 * [OUT]     inv указатель для сохранения обратного элемента
 *
 * [RETURN]  POL_SUCCESS        — обратный элемент существует и вычислен
 *           POL_NO_INVERSE     — обратного элемента не существует (НОД(a, m) ≠ 1)
 *                                или модуль слишком велик для преобразования в long long
 *
 * [NOTE]    Обратный элемент существует только если НОД(a, m) = 1.
 * [NOTE]    Если m > LLONG_MAX, функция возвращает POL_NO_INVERSE, поскольку
 *           использует знаковую арифметику для приведения результата в диапазон [0, m-1].
 * [NOTE]    Реализация использует расширенный алгоритм Евклида (egcd).
 */
int modulo_inverse(ULL a, ULL m, ULL *inv);


/*
 * Выводит информацию о многочлене в удобочитаемом формате.
 * Формат вывода: имя: degree=<степень>, mod=<модуль>, coeffs=[коэф0,коэф1,...,коэфN]
 * где коэффициенты перечислены от младшей степени (свободный член) к старшей.
 *
 * [IN]      p       указатель на многочлен
 * [IN]      name    имя многочлена для префикса в выводе
 *
 * [OUT]     (на stdout) форматированная строка с информацией о многочлене
 *
 * [NOTE]    Функция предназначена для отладки и тестирования.
 */
void print_polynomial(const Polynomial* p, const char* name);

/*--------------------- БАЗОВЫЕ ОПЕРАЦИИ ---------------------*/

/*
 *
 * Инициализирует многочлен p заданной степени и модуля.
 * Память под массив коэффициентов выделяется внутри функции.
 *
 * [IN]      p       указатель на структуру Polynomial
 * [IN]      degree  требуемая степень (массив размера degree + 1)
 * [IN]      modulo     характеристика кольца; должно быть > 1
 *
 * [OUT]     p       p->coeffs выделены и заполнены нулями; p->degree и p->modulo установлены
 *
 * [RETURN]  POL_SUCCESS        — успех
 *           POL_NULL_PTR       — p == NULL
 *           POL_INVALID_MODULO — modulo <= 1
 *           POL_MEMORY_ERROR   — ошибка выделения памяти
 */
int new_pol(Polynomial *p, size_t degree, ULL modulo);


/*
 * Освобождает память, выделенную под многочлен p.
 *
 * [IN]      p       указатель на структуру Polynomial
 *
 * [OUT]     p       coeffs освобождены и сброшены в NULL; degree = 0; modulo = 0
 *
 * [RETURN]  POL_SUCCESS           — успех
 *           POL_NULL_PTR          — p == NULL
 */
void free_pol(Polynomial *p);


/*
 * Копирует многочлен src → dst.
 *
 * [IN]      src     исходный многочлен
 * [IN]      dst     указатель на принимающий многочлен
 *
 * [OUT]     dst     создаётся копия src
 *
 * [RETURN]  POL_SUCCESS           — успех
 *           POL_NULL_PTR          — src == NULL или dst == NULL
 *           POL_MODULO_MISMATCH      — несовместимые модули
 *           POL_MEMORY_ERROR      — ошибка выделения памяти
 */
int copy_pol(const Polynomial *src, Polynomial *dst);


/*
 * Нормализует многочлен: удаляет ведущие нули, корректирует степень,
 * приводит коэффициенты по модулю.
 *
 * [IN/OUT]  p       многочлен для нормализации
 *
 * [RETURN]  POL_SUCCESS           — успех
 *           POL_NULL_PTR          — p == NULL
 *           POL_INVALID_MODULO       — некорректный p->modulo (<= 1)
 */
int normalize_pol(Polynomial *p);


/*
 * Складывает многочлены: R = A + B.
 *
 * [IN]      A       первый многочлен
 * [IN]      B       второй многочлен
 * [IN/OUT]  R       результат
 *
 * [RETURN]  POL_SUCCESS           — успех
 *           POL_NULL_PTR          — A == NULL или B == NULL или R == NULL
 *           POL_MODULO_MISMATCH      — несовместимые модули
 *           POL_MEMORY_ERROR      — ошибка выделения памяти
 */
int sum_pol(const Polynomial* A, const Polynomial* B, Polynomial* R);


/*
 * Вычитает многочлены: R = A - B.
 *
 * [IN]      A       уменьшаемый
 * [IN]      B       вычитаемый
 * [IN/OUT]  R       результат
 *
 * [RETURN]  POL_SUCCESS           — успех
 *           POL_NULL_PTR          — A == NULL или B == NULL или R == NULL
 *           POL_MODULO_MISMATCH      — несовместимые модули
 *           POL_MEMORY_ERROR      — ошибка выделения памяти
 */
int sub_pol(const Polynomial* A, const Polynomial* B, Polynomial* R);


/*
 * Умножает многочлен A на скаляр k: R = k * A.
 *
 * [IN]      A       исходный многочлен
 * [IN]      k       целый скаляр
 * [IN/OUT]  R       результат умножения
 *
 * [RETURN]  POL_SUCCESS           — успех
 *           POL_NULL_PTR          — A == NULL или R == NULL
 *           POL_INVALID_MODULO       — неверный A->modulo
 *           POL_MEMORY_ERROR      — ошибка выделения памяти
 */
int scalar_mul_pol(const Polynomial* A, ULL k, Polynomial* R);


/*
 * Перемножает многочлены: R = A * B.
 *
 * [IN]      A       первый множитель
 * [IN]      B       второй множитель
 * [IN/OUT]  R       результат умножения
 *
 * [RETURN]  POL_SUCCESS           — успех
 *           POL_NULL_PTR          — A == NULL или B == NULL или R == NULL
 *           POL_MODULO_MISMATCH      — несовместимые модули
 *           POL_MEMORY_ERROR      — ошибка выделения памяти
 */
int pol_mul_pol(const Polynomial* A, const Polynomial* B, Polynomial* R);


/*
 * Скалярное произведение коэффициентов: sum(A[i] * B[i]).
 *
 * [IN]      A         первый многочлен
 * [IN]      B         второй многочлен
 * [OUT]     result    указатель, куда будет записан результат
 *
 * [RETURN]  POL_SUCCESS           — успех
 *           POL_NULL_PTR          — A == NULL или B == NULL или result == NULL
 *           POL_MODULO_MISMATCH      — несовместимые модули
 */
int dot_pol(const Polynomial* A, const Polynomial* B, ULL* Result);


/*
 * Остаток полиномиального деления: R = A modulo M.
 *
 * [IN]      A       делимое
 * [IN]      M       делитель (ненулевой)
 * [IN/OUT]  R       остаток
 *
 * [RETURN]  POL_SUCCESS           — успех
 *           POL_NULL_PTR          — A == NULL или M == NULL или R == NULL
 *           POL_ZERO_DIV         — M — нулевой многочлен
 *           POL_MODULO_MISMATCH      — несовместимые модули
 *           POL_MEMORY_ERROR      — ошибка выделения памяти
 *           POL_NO_INVERSE        — нет мультипликативного обратного для старшего коэффициента M
 */
int modulo_unit_pol(const Polynomial* A, const Polynomial* M, Polynomial* R);

/*
 * Вычисляет произведение многочленов A и B с последующим приведением результата
 * по модулю унитарного многочлена M:  R = (A * B) mod M.
 *
 * Многочлен M должен быть *унитарным*, то есть его старший коэффициент равен 1.
 * Это позволяет выполнять деление и нахождение остатка без вычисления обратного элемента.
 *
 * Алгоритм:
 *   1) Вычисляется временный многочлен T = A * B.
 *   2) Выполняется полиномиальное деление T на M (как на унитарный делитель).
 *      Так как M унитарный, каждый шаг деления корректен без поиска инверсии.
 *   3) В R записывается остаток T mod M.
 *
 * [IN]      A       первый множитель
 * [IN]      B       второй множитель
 * [IN]      M       унитарный модуль (старший коэффициент == 1)
 * [IN/OUT]  R       результирующий многочлен
 *
 * [OUT]     R       содержит остаток произведения A * B по модулю M
 *
 * [RETURN]  POL_SUCCESS         — успех
 *           POL_NULL_PTR        — один из аргументов == NULL
 *           POL_MODULO_MISMATCH — разные модули у A, B и M
 *           POL_ZERO_DIV        — M — нулевой многочлен
 *           POL_INVALID_ARG     — M не является унитарным (старший коэффициент != 1)
 *           POL_MEMORY_ERROR    — ошибка выделения памяти
 *
 * [NOTE]    Функция не изменяет входные A, B, M.
 * [NOTE]    Для корректной работы M должен быть нормализован заранее.
 */
int pol_mul_mod_unit(const Polynomial* A, const Polynomial* B,
                     const Polynomial* M, Polynomial* R);

#endif //LAB3_POLYNOMIAL_H