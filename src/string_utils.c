#include "../include/string_utils.h"
#include "../include/mem_tracker.h"

int str_to_pol(const char* str, ULL modulo, Polynomial* pol)
{
    if (!str || !pol) return POL_NULL_PTR;
    if (modulo <= 1) return POL_INVALID_MODULO;

    if (pol->coeffs != NULL)
    {
        free(pol->coeffs, (pol->degree + 1) * sizeof(ULL));
    }
    set_pol_params(pol, 0, modulo);

    while (*str == ' ') str++;

    if (*str++ != '(') return POL_INVALID_ARG;

    // Считаем запятые
    size_t count = 1;
    const char* p = str;
    while (*p && *p != ')')
    {
        if (*p == ',') count++;
        p++;
    }
    if (*p != ')') return POL_INVALID_ARG;

    ULL* coeffs = malloc(count * sizeof(ULL));
    if (!coeffs) return POL_MEMORY_ERROR;

    p = str;
    for (size_t i = 0; i < count; i++)
    {
        // Пропускаем пробелы
        while (*p == ' ') p++;

        int negative = 0;
        if (*p == '-' || *p == '+')
        {
            negative = (*p == '-');
            p++;
        }

        if (*p < '0' || *p > '9')
        {
            free(coeffs, count * sizeof(ULL));
            return POL_INVALID_ARG;
        }

        ULL val = 0;
        while (*p >= '0' && *p <= '9')
            val = val * 10 + (*p++ - '0');

        if (negative)
        {
            coeffs[i] = (modulo - (val % modulo)) % modulo;
        }
        else
        {
            coeffs[i] = val % modulo;
        }

        while (*p == ' ') p++;

        if (i < count - 1)
        {
            if (*p != ',')
            {
                free(coeffs, count * sizeof(ULL));
                return POL_INVALID_ARG;
            }
            p++; // Пропускаем запятую
        }
    }

    // Пропускаем пробелы перед ')'
    while (*p == ' ') p++;
    if (*p != ')')
    {
        free(coeffs, count * sizeof(ULL));
        return POL_INVALID_ARG;
    }

    // Убираем ведущие нули
    size_t degree = count - 1;
    while (degree > 0 && coeffs[degree] == 0)
        degree--;

    // Выделяем память для результата
    ULL* result_coeffs = malloc((degree + 1) * sizeof(ULL));
    if (!result_coeffs)
    {
        free(coeffs, count * sizeof(ULL));
        return POL_MEMORY_ERROR;
    }

    for (size_t i = 0; i <= degree; i++)
        result_coeffs[i] = coeffs[i];

    free(coeffs, count * sizeof(ULL));

    set_pol_params(pol, degree, modulo);
    pol->coeffs = result_coeffs;

    return POL_SUCCESS;
}

int pol_to_str(const Polynomial* pol, char** str, size_t* out_size)
{
    if (pol == NULL || str == NULL || out_size == NULL)
        return POL_NULL_PTR;

    if (pol->coeffs == NULL)
        return POL_INVALID_ARG;

    size_t total_len = 2; // ( + )

    for (size_t i = 0; i <= pol->degree; i++)
    {
        ULL num = pol->coeffs[i];
        do {
            total_len++;
        } while (num /= 10);

        if (i < pol->degree) total_len++; // запятая
    }

    *str = malloc(total_len + 1);
    if (*str == NULL)
        return POL_MEMORY_ERROR;

    char* pos = *str;
    *pos++ = '(';

    for (size_t i = 0; i <= pol->degree; i++)
    {
        ULL num = pol->coeffs[i];
        char* num_start = pos;

        // в обратном порядке
        do {
            *pos++ = '0' + (num % 10); // ASCII NOLINT(*-narrowing-conversions)
        } while (num /= 10);

        // Разворачиваем цифры
        char* num_end = pos - 1;
        while (num_start < num_end)
        {
            char tmp = *num_start;
            *num_start = *num_end;
            *num_end = tmp;
            num_start++;
            num_end--;
        }

        if (i < pol->degree)
            *pos++ = ',';
    }

    *pos++ = ')';
    *pos = '\0';
    *out_size = total_len + 1; // + '\0'

    return POL_SUCCESS;
}
