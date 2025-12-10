#include "../include/test.h"
#include "../include/mem_tracker.h"

#define MAX_INPUT_LEN 1024

size_t maxMemory = 0;
size_t currentMemory = 0;

ULL rand64()
{
    return ((ULL)(rand() & 0xFFFF) << 48) |
           ((ULL)(rand() & 0xFFFF) << 32) |
           ((ULL)(rand() & 0xFFFF) << 16) |
           ((ULL)(rand() & 0xFFFF));
}

int get_rand_pol(Polynomial* P, size_t degree, ULL modulo)
{
    if (P == NULL)
        return POL_NULL_PTR;

    normalize_pol(P);
    return POL_SUCCESS;
}

size_t pol_bytes(const Polynomial* P)
{
    if (!P || !P->coeffs) return 0;
    return (P->degree + 1) * sizeof(ULL);
}

int manual_test()
{
    printf("=== Тестирование pol_mul_mod_unit ===\n\n");

    int test_count = 0;
    int passed_count = 0;
    Polynomial A, B, M, R;
    new_pol(&R, 0, 2);
    // ----- ТЕСТ 1: обычное корректное вычисление -----
    {
        test_count++;
        printf("[TEST 1] Обычное корректное вычисление\n");


        new_pol(&A, 2, 7);  // A = x^2 + 2x + 3 в Z7
        A.coeffs[0] = 3; A.coeffs[1] = 2; A.coeffs[2] = 1;

        new_pol(&B, 1, 7);  // B = x + 4 в Z7
        B.coeffs[0] = 4; B.coeffs[1] = 1;

        new_pol(&M, 2, 7);  // M = x^2 + 1 (унитарный)
        M.coeffs[0] = 1; M.coeffs[1] = 0; M.coeffs[2] = 1;

        // Ожидаем: (x^2+2x+3)(x+4) mod (x^2+1) = 3x + 5 в Z7
        Polynomial expected;
        new_pol(&expected, 1, 7);
        expected.coeffs[0] = 6; expected.coeffs[1] = 3;

        int result = pol_mul_mod_unit(&A, &B, &M, &R);

        printf("  Вход: A="); print_polynomial(&A, "");
        printf("        B="); print_polynomial(&B, "");
        printf("        M="); print_polynomial(&M, "");
        printf("  Ожидаем: "); print_polynomial(&expected, "");
        printf("  Получили: "); print_polynomial(&R, "");
        printf("  Статус: %d", result);

        // Простая проверка результата
        int ok = (result == POL_SUCCESS && R.degree == expected.degree &&
                 R.coeffs[0] == expected.coeffs[0] && R.coeffs[1] == expected.coeffs[1]);
        if (ok)
        {
            printf(" -> ПРОЙДЕН\n");
            passed_count++;
        }
        else
        {
            printf(" -> ПРОВАЛ\n");
        }

        free_pol(&A); free_pol(&B); free_pol(&M); free_pol(&expected);
    }

    printf("\n");

    // ----- ТЕСТ 2: произведение с нулевым многочленом -----
    {
        test_count++;
        printf("[TEST 2] Произведение с нулевым многочленом\n");

        new_pol(&A, 0, 5);  // A = 0
        A.coeffs[0] = 0;

        new_pol(&B, 2, 5);  // B = x^2 + 1
        B.coeffs[0] = 1; B.coeffs[1] = 0; B.coeffs[2] = 1;

        new_pol(&M, 2, 5);  // M = x^2 + x + 1 (унитарный)
        M.coeffs[0] = 1; M.coeffs[1] = 1; M.coeffs[2] = 1;

        int result = pol_mul_mod_unit(&A, &B, &M, &R);

        printf("  Вход: A=0, B=x^2+1, M=x^2+x+1 mod 5\n");
        printf("  Ожидаем: 0\n");
        printf("  Получили: "); print_polynomial(&R, "");
        printf("  Статус: %d", result);

        int ok = (result == POL_SUCCESS && R.degree == 0 && R.coeffs[0] == 0);
        if (ok)
        {
            printf(" -> ПРОЙДЕН\n");
            passed_count++;
        }
        else
        {
            printf(" -> ПРОВАЛ\n");
        }

        free_pol(&A); free_pol(&B); free_pol(&M);
    }

    printf("\n");

    // ----- ТЕСТ 3: модуль степени 0 (M = 1) -----
    {
        test_count++;
        printf("[TEST 3] Модуль степени 0 (M = 1)\n");

        new_pol(&A, 1, 11);  // A = x + 2
        A.coeffs[0] = 2; A.coeffs[1] = 1;

        new_pol(&B, 1, 11);  // B = x + 3
        B.coeffs[0] = 3; B.coeffs[1] = 1;

        new_pol(&M, 0, 11);  // M = 1 (унитарный)
        M.coeffs[0] = 1;

        int result = pol_mul_mod_unit(&A, &B, &M, &R);

        printf("  Вход: A=x+2, B=x+3, M=1 mod 11\n");
        printf("  Ожидаем: 0 (любое число mod 1 = 0)\n");
        printf("  Получили: "); print_polynomial(&R, "");
        printf("  Статус: %d", result);

        int ok = (result == POL_SUCCESS && R.degree == 0 && R.coeffs[0] == 0);
        if (ok)
        {
            printf(" -> ПРОЙДЕН\n");
            passed_count++;
        }
        else
        {
            printf(" -> ПРОВАЛ\n");
        }

        free_pol(&A); free_pol(&B); free_pol(&M);
    }

    printf("\n");

    // ----- ТЕСТ 4: M не унитарный -----
    {
        test_count++;
        printf("[TEST 4] M не унитарный (старший коэффициент != 1)\n");

        new_pol(&A, 0, 7);  // A = 1
        A.coeffs[0] = 1;

        new_pol(&B, 0, 7);  // B = 1
        B.coeffs[0] = 1;

        new_pol(&M, 1, 7);  // M = 2x + 1 (не унитарный, старший коэффициент 2)
        M.coeffs[0] = 1; M.coeffs[1] = 2;

        int result = pol_mul_mod_unit(&A, &B, &M, &R);

        printf("  Вход: A=1, B=1, M=2x+1 mod 7\n");
        printf("  Ожидаем: POL_INVALID_ARG (9, M не унитарный)\n");
        printf("  Статус: %d", result);

        if (result == POL_INVALID_ARG) {
            printf(" -> ПРОЙДЕН\n");
            passed_count++;
        } else {
            printf(" -> ПРОВАЛ\n");
        }

        free_pol(&A); free_pol(&B); free_pol(&M);
    }

    printf("\n");

    // ----- ТЕСТ 5: деление на нулевой многочлен -----
    {
        test_count++;
        printf("[TEST 5] Деление на нулевой многочлен\n");

        new_pol(&A, 0, 13);  // A = 1
        A.coeffs[0] = 1;

        new_pol(&B, 0, 13);  // B = 1
        B.coeffs[0] = 1;

        new_pol(&M, 0, 13);  // M = 0
        M.coeffs[0] = 0;

        int result = pol_mul_mod_unit(&A, &B, &M, &R);

        printf("  Вход: A=1, B=1, M=0 mod 13\n");
        printf("  Ожидаем: POL_ZERO_DIV (5, деление на нулевой многочлен)\n");
        printf("  Статус: %d", result);

        if (result == POL_ZERO_DIV)
        {
            printf(" -> ПРОЙДЕН\n");
            passed_count++;
        }
        else
        {
            printf(" -> ПРОВАЛ\n");
        }

        free_pol(&A); free_pol(&B); free_pol(&M);
    }
    free_pol(&R);

    printf("\n=== ИТОГО: %d/%d тестов пройдено ===\n", passed_count, test_count);

    if (passed_count == test_count)
    {
        return TEST_SUCCESS;
    }
    else
    {
        return TEST_UNKNOWN_ERROR;
    }
}

int input_test()
{
    char buffer[MAX_INPUT_LEN];
    ULL modulo;

    printf("Enter modulo (integer > 1):\n");
    if (scanf("%llu", &modulo) != 1 || modulo <= 1)
    {
        return TEST_INVALID_MODULO;
    }

    while (getchar() != '\n')
    {;}

    Polynomial A, B, M, R;
    new_pol(&A, 0, modulo);
    new_pol(&B, 0, modulo);
    new_pol(&M, 0, modulo);
    new_pol(&R, 0, modulo);

    printf("Enter polynomial A in format (a0,a1,...,an):\n");
    if (!fgets(buffer, sizeof(buffer), stdin))
    {
        return TEST_INVALID_ARG;
    }
    buffer[strcspn(buffer, "\n")] = '\0';

    int status = str_to_pol(buffer, modulo, &A);
    if (status != POL_SUCCESS)
    {
        return TEST_INVALID_ARG;
    }

    printf("Enter polynomial B in format (b0,b1,...,bn):\n");
    if (!fgets(buffer, sizeof(buffer), stdin))
    {
        free_pol(&A);
        return TEST_INVALID_ARG;
    }
    buffer[strcspn(buffer, "\n")] = '\0';

    status = str_to_pol(buffer, modulo, &B);
    if (status != POL_SUCCESS)
    {
        free_pol(&A);
        return TEST_INVALID_ARG;
    }

    printf("Enter unitary modulus M in format (m0,m1,...,1):\n");
    if (!fgets(buffer, sizeof(buffer), stdin))
    {
        free_pol(&A);
        free_pol(&B);
        return TEST_INVALID_ARG;
    }
    buffer[strcspn(buffer, "\n")] = '\0';

    status = str_to_pol(buffer, modulo, &M);
    if (status != POL_SUCCESS)
    {
        free_pol(&A);
        free_pol(&B);
        return TEST_INVALID_ARG;
    }

    if (M.degree == 0 && M.coeffs[0] == 0)
    {
        free_pol(&A);
        free_pol(&B);
        free_pol(&M);
        return TEST_ZERO_DIV;
    }

    if (M.coeffs[M.degree] != 1)
    {
        free_pol(&A);
        free_pol(&B);
        free_pol(&M);
        return TEST_INVALID_ARG;
    }

    status = pol_mul_mod_unit(&A, &B, &M, &R);

    free_pol(&A);
    free_pol(&B);
    free_pol(&M);

    if (status != POL_SUCCESS)
        return TEST_UNKNOWN_ERROR;

    char* str_result = NULL;
    size_t str_size = 0;
    status = pol_to_str(&R, &str_result, &str_size);
    if (status != POL_SUCCESS)
    {
        free_pol(&R);
        return TEST_MEMORY_ERROR;
    }

    printf("Result: %s\n", str_result);

    free(str_result, str_size);
    free_pol(&R);

    return TEST_SUCCESS;
}

int auto_test()
{
    // srand((unsigned)time(NULL));
    srand(1);

    int test_variant;
    printf("Choose test variant (1 = fix C vs M, 2 = fix degree of divisor M): ");
    if (scanf("%d", &test_variant) != 1 || (test_variant != 1 && test_variant != 2))
        return TEST_INVALID_ARG;
    while (getchar() != '\n')
    {;}

    size_t deg_A, deg_B, deg_M;
    ULL modulo;

    printf("Enter modulo (>1):\n");
    if (scanf("%llu", &modulo) != 1 || modulo <= 1)
        return TEST_INVALID_MODULO;

    if (test_variant == 1)
    {
        printf("Enter degree for A:\n");
        if (scanf("%zu", &deg_A) != 1)
            return TEST_INVALID_ARG;
        while (getchar() != '\n')
        {;}

        printf("Enter degree for B:\n");
        if (scanf("%zu", &deg_B) != 1)
            return TEST_INVALID_ARG;
        while (getchar() != '\n')
        {;}

        printf("Enter degree for M:\n");
        if (scanf("%zu", &deg_M) != 1)
            return TEST_INVALID_ARG;
        while (getchar() != '\n')
        {;}
    }
    else if (test_variant == 2)
    {
        printf("Enter degree for divisor M:\n");
        if (scanf("%zu", &deg_M) != 1)
            return TEST_INVALID_ARG;
        while (getchar() != '\n')
        {;}

        printf("Enter degree for A:\n");
        if (scanf("%zu", &deg_A) != 1)
            return TEST_INVALID_ARG;
        while (getchar() != '\n')
        {;}

        printf("Enter degree for B (less than or equal M): ");
        if (scanf("%zu", &deg_B) != 1 || deg_B > deg_M) deg_B = deg_M;
        while (getchar() != '\n')
        {;}
    }
    else
    {
        printf("Invalid test variant.\n");
        return TEST_INVALID_ARG;
    }

    Polynomial A, B, M, R;
    new_pol(&A, deg_A, modulo);
    new_pol(&B, deg_B, modulo);
    new_pol(&M, deg_M, modulo);
    new_pol(&R, deg_A + deg_B, modulo);

    get_rand_pol(&A, deg_A, modulo);
    get_rand_pol(&B, deg_B, modulo);
    get_rand_pol(&M, deg_M, modulo);

    // Сохраняем начальные значения памяти
    size_t mem_before = g_current_allocated;

    int status = pol_mul_mod_unit(&A, &B, &M, &R);
    if (status != POL_SUCCESS)
    {
        printf("Error in pol_mul_mod_unit: %d\n", status);
        free_pol(&A); free_pol(&B); free_pol(&M); free_pol(&R);
        return status;
    }

    size_t mem_after = g_current_allocated;

    size_t bytes_A = pol_bytes(&A);
    size_t bytes_B = pol_bytes(&B);
    size_t bytes_M = pol_bytes(&M);
    size_t bytes_R = pol_bytes(&R);

    // Консольный вывод
    printf("degA=%zu, degB=%zu, degM=%zu, degR=%zu, bytesA=%zu, bytesB=%zu, bytesM=%zu, bytesR=%zu, mem_delta=%zu\n",
           A.degree, B.degree, M.degree, R.degree,
           bytes_A, bytes_B, bytes_M, bytes_R,
           mem_after - mem_before);

    // CSV вывод
    FILE* f = fopen("result.csv", "a");
    if (f)
    {
        fprintf(f, "%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu,%zu\n",
                A.degree, B.degree, M.degree, R.degree,
                bytes_A, bytes_B, bytes_M, bytes_R,
                mem_after - mem_before);
        fclose(f);
    }

    free_pol(&A); free_pol(&B); free_pol(&M); free_pol(&R);
    return 0;
}