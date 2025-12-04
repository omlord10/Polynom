#include "../include/polynomial.h"

/*--------------------- ВСПОМОГАТЕЛЬНЫЕ ОПЕРАЦИИ ---------------------*/

int realloc_coeffs(Polynomial* R, size_t required_degree)
{
    if (R->coeffs == NULL || R->degree < required_degree)
    {
        ULL *new_coeffs = calloc(required_degree + 1, sizeof(ULL));
        if (new_coeffs == NULL)
            return POL_MEMORY_ERROR;

        if (R->coeffs != NULL)
            free(R->coeffs);

        R->coeffs = new_coeffs;
    }
    return POL_SUCCESS;
}

int set_pol_params(Polynomial *R, size_t deg, ULL modulo)
{
    R->degree = deg;
    R->modulo = modulo;
    return POL_SUCCESS;
}

/* Extended GCD */
ULL egcd(ULL a, ULL b, long long *x, long long *y)
{
    long long x0 = 1, y0 = 0;
    long long x1 = 0, y1 = 1;

    while (b != 0)
    {
        ULL q = a / b;

        ULL tmp_a = a % b;
        a = b;
        b = tmp_a;

        long long tmp_x = x0 - (long long)q * x1;
        x0 = x1;
        x1 = tmp_x;

        long long tmp_y = y0 - (long long)q * y1;
        y0 = y1;
        y1 = tmp_y;
    }

    *x = x0;
    *y = y0;
    return a;   /* gcd */
}

/* Обратный по модулю М */
int modulo_inverse(ULL a, ULL m, ULL *inv)
{
    long long x, y;

    ULL g = egcd(a, m, &x, &y);

    if (g != 1)
        return POL_NO_INVERSE;

    if (m > LLONG_MAX)
        return POL_NO_INVERSE;  // Слишком большой модуль

    long long m_ll = (long long)m;
    long long res = x % m_ll;
    if (res < 0)
        res += m_ll;

    *inv = (ULL)res;
    return POL_SUCCESS;
}

void print_polynomial(const Polynomial* p, const char* name)
{
    if (p == NULL)
    {
        printf("%s: NULL\n", name);
        return;
    }

    printf("%s: degree=%zu, mod=%llu, coeffs=[", name, p->degree, p->modulo);
    for (size_t i = 0; i <= p->degree; i++)
    {
        printf("%llu", p->coeffs[i]);
        if (i < p->degree) printf(",");
    }
    printf("]\n");
}

/*--------------------- БАЗОВЫЕ ОПЕРАЦИИ ---------------------*/

int new_pol(Polynomial *p, const size_t degree, const ULL modulo)
{
    if (p == NULL)
    {
        return POL_NULL_PTR;
    }

    if (modulo <= 1)
    {
        return POL_INVALID_MODULO;
    }

    p->coeffs = calloc(degree + 1, sizeof(ULL));
    if (p->coeffs == NULL)
    {
        return POL_MEMORY_ERROR;
    }

    set_pol_params(p, degree, modulo);

    return POL_SUCCESS;
}

void free_pol(Polynomial *p)
{
    if (p == NULL)
    {
        return;
    }

    if (p->coeffs != NULL)
    {
        free(p->coeffs);
        p->coeffs = NULL;
    }

    set_pol_params(p, 0, 0);
}

int copy_pol(const Polynomial *src, Polynomial *dst)
{
    if (src == NULL || dst == NULL)
        return POL_NULL_PTR;

    realloc_coeffs(dst,src->degree);

    for (size_t i = 0; i <= src->degree; i++)
        dst->coeffs[i] = src->coeffs[i];

    set_pol_params(dst, src->degree, src->modulo);

    return POL_SUCCESS;
}

int normalize_pol(Polynomial *p)
{
    if (p == NULL)
    {
        return POL_NULL_PTR;
    }

    if (p->modulo <= 1)
    {
        return POL_INVALID_MODULO;
    }

    ULL m = p->modulo;

    for (size_t i = 0; i <= p->degree; i++)
    {
        p->coeffs[i] = p->coeffs[i] % m;
    }

    while (p->degree > 0 && p->coeffs[p->degree] == 0)
    {
        p->degree--;
    }

    return POL_SUCCESS;
}

int sum_pol(const Polynomial* A, const Polynomial* B, Polynomial* R)
{
    if (A == NULL || B == NULL || R == NULL)
    {
        return POL_NULL_PTR;
    }

    if (A->modulo != B->modulo)
    {
        return POL_MODULO_MISMATCH;
    }

    size_t max_deg = (A->degree > B->degree) ? A->degree : B->degree;

    realloc_coeffs(R, max_deg);
    set_pol_params(R, max_deg, A->modulo);

    ULL m = R->modulo;

    for (size_t i = 0; i <= max_deg; i++)
    {
        ULL a = (i <= A->degree) ? A->coeffs[i] : 0; // подгоняет до нужных размеров
        ULL b = (i <= B->degree) ? B->coeffs[i] : 0;

        ULL sum = (a % m + b % m) % m;

        R->coeffs[i] = sum;
    }

    normalize_pol(R);
    return POL_SUCCESS;
}

int sub_pol(const Polynomial* A, const Polynomial* B, Polynomial* R)
{
    if (A == NULL || B == NULL || R == NULL)
    {
        return POL_NULL_PTR;
    }

    if (A->modulo != B->modulo)
    {
        return POL_MODULO_MISMATCH;
    }

    size_t max_deg = (A->degree > B->degree) ? A->degree : B->degree;

    realloc_coeffs(R, max_deg);
    set_pol_params(R, max_deg, A->modulo);

    ULL m = R->modulo;
    ULL diff;

    for (size_t i = 0; i <= max_deg; i++)
    {
        ULL a = (i <= A->degree) ? A->coeffs[i] : 0;
        ULL b = (i <= B->degree) ? B->coeffs[i] : 0;

        if (a >= b)
        {
            diff = (a - b) % m;
        }
        else
        {
            diff = (m - ((b - a) % m)) % m;
        }

        R->coeffs[i] = diff;
    }

    normalize_pol(R);
    return POL_SUCCESS;
}

int scalar_mul_pol(const Polynomial* A, ULL k, Polynomial* R)
{
    if (A == NULL || R == NULL)
    {
        return POL_NULL_PTR;
    }

    if (A->modulo <= 1)
    {
        return POL_INVALID_MODULO;
    }

    realloc_coeffs(R, A->degree);

    // k == 0 -> 0 poly
    if (k == 0)
    {
        set_pol_params(R, 0, A->modulo);
        R->coeffs[0] = 0;
        return POL_SUCCESS;
    }

    set_pol_params(R, A->degree, A->modulo);

    ULL m = R->modulo;

    for (size_t i = 0; i <= A->degree; i++)
        R->coeffs[i] = (A->coeffs[i] * (k % m)) % m;

    normalize_pol(R);
    return POL_SUCCESS;
}

int dot_pol(const Polynomial* A, const Polynomial* B, ULL* result)
{
    if (A == NULL || B == NULL || result == NULL)
        return POL_NULL_PTR;

    if (A->modulo != B->modulo)
        return POL_MODULO_MISMATCH;

    ULL m = A->modulo;
    ULL sum = 0;

    size_t min_deg = (A->degree < B->degree) ? A->degree : B->degree;

    for (size_t i = 0; i <= min_deg; i++)
    {
        sum = (sum + (A->coeffs[i] * B->coeffs[i]) % m) % m;
    }

    *result = sum;
    return POL_SUCCESS;
}

int pol_mul_pol(const Polynomial* A, const Polynomial* B, Polynomial* R)
{
    if (A == NULL || B == NULL || R == NULL)
    {
        return POL_NULL_PTR;
    }

    if (A->modulo != B->modulo)
        return POL_MODULO_MISMATCH;

    int is_A_zero = (A->degree == 0 && A->coeffs[0] == 0);
    int is_B_zero = (B->degree == 0 && B->coeffs[0] == 0);

    if (is_A_zero || is_B_zero)
    {
        set_pol_params(R, 0, A->modulo);
        R->coeffs[0] = 0;
        return POL_SUCCESS;
    }

    size_t result_degree = A->degree + B->degree;

    ULL* temp_coeffs = calloc(result_degree + 1, sizeof(ULL));
    if (temp_coeffs == NULL) return POL_MEMORY_ERROR;

    ULL m = A->modulo;

    for (size_t i = 0; i <= A->degree; i++)
    {
        ULL a = A->coeffs[i];
        if (a == 0) continue;

        for (size_t j = 0; j <= B->degree; j++)
        {
            ULL b = B->coeffs[j];
            if (b == 0) continue;

            temp_coeffs[i + j] = (temp_coeffs[i + j] + (a * b) % m) % m;
        }
    }

    set_pol_params(R, result_degree, A->modulo);

    for (size_t i = 0; i <= result_degree; i++)
        R->coeffs[i] = temp_coeffs[i];

    free(temp_coeffs);
    normalize_pol(R);
    return POL_SUCCESS;
}

/* Остаток полиномиального деления: R = A modulo M */
int modulo_unit_pol(const Polynomial* A, const Polynomial* M, Polynomial* R)
{
    if (A == NULL || M == NULL || R == NULL)
    {
        return POL_NULL_PTR;
    }

    if (A->modulo != M->modulo)
    {
        return POL_MODULO_MISMATCH;
    }

    ULL m = A->modulo;

    if (M->degree == 0 && M->coeffs[0] == 0)
    {
        return POL_ZERO_DIV;
    }

    realloc_coeffs(R, A->degree);
    set_pol_params(R, A->degree, m);

    for (size_t i = 0; i <= A->degree; i++)
        R->coeffs[i] = A->coeffs[i] % m;

    normalize_pol(R);

    ULL lead = M->coeffs[M->degree] % m;
    ULL inv;
    int status = modulo_inverse(lead, m, &inv);
    if (status != POL_SUCCESS)
    {
        return POL_NO_INVERSE;
    }

    while (R->degree >= M->degree && !(R->degree == 0 && R->coeffs[0] == 0))
    {
        size_t shift = R->degree - M->degree;
        ULL coeff = (R->coeffs[R->degree] * inv) % m;

        for (size_t i = 0; i <= M->degree; i++)
        {
            size_t pos = i + shift;
            ULL sub = (M->coeffs[i] * coeff) % m;

            if (R->coeffs[pos] >= sub)
                R->coeffs[pos] = (R->coeffs[pos] - sub) % m;
            else
                R->coeffs[pos] = (m - ((sub - R->coeffs[pos]) % m)) % m;
        }

        normalize_pol(R);
    }

    return POL_SUCCESS;
}

int pol_mul_mod_unit(const Polynomial* A, const Polynomial* B,
                      const Polynomial* M, Polynomial* R)
{
    if (A == NULL || B == NULL || M == NULL || R == NULL)
        return POL_NULL_PTR;

    if (A->modulo != B->modulo || A->modulo != M->modulo)
        return POL_MODULO_MISMATCH;

    if (M->degree == 0 && M->coeffs[0] == 0)
        return POL_ZERO_DIV;

    if (M->coeffs[M->degree] != 1)
        return POL_INVALID_ARG;

    Polynomial T;

    new_pol(&T, A->degree + B->degree, A->modulo);

    int status = pol_mul_pol(A, B, &T);
    if (status != POL_SUCCESS)
    {
        free_pol(&T);
        return status;
    }
    status = modulo_unit_pol(&T, M, R);

    free_pol(&T);
    return status;
}