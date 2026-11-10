/*
 * Saturating arithmetic lab: Windows x64 + Microsoft C compiler.
 * Run build_msvc.cmd in the x64 Native Tools Command Prompt.
 *
 * The scalar functions use defined C arithmetic. No signed wraparound is
 * assumed. SSE2 wraparound is demonstrated with _mm_add_epi16 explicitly.
 * __declspec(noinline) keeps the operations visible in the disassembly.
 * The deliberately wrong LONG_MAX/LONG_MIN example is clearly identified.
 */

#include <emmintrin.h>
#include <inttypes.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#if defined(_MSC_VER)
#if !defined(_M_X64)
#error Compile this lab using the x64 MSVC toolchain.
#endif
#define NOINLINE __declspec(noinline)
_Static_assert(sizeof(long) == 4, "This lab expects Windows LLP64.");
_Static_assert(sizeof(void *) == 8, "This lab expects x64 pointers.");
#else
/* Allows an independent check of the C logic with another compiler. */
#define NOINLINE __attribute__((noinline))
#endif

_Static_assert(sizeof(int16_t) == 2, "16-bit samples are required.");
_Static_assert(sizeof(int32_t) == 4, "32-bit integers are required.");
_Static_assert(sizeof(int64_t) == 8, "64-bit integers are required.");

NOINLINE int16_t sat_add_i16(int16_t a, int16_t b)
{
    int32_t sum = (int32_t)a + (int32_t)b;
    if (sum > INT16_MAX) return INT16_MAX;
    if (sum < INT16_MIN) return INT16_MIN;
    return (int16_t)sum;
}

NOINLINE int32_t sat_add_i32(int32_t a, int32_t b)
{
    int64_t sum = (int64_t)a + (int64_t)b;
    if (sum > INT32_MAX) return INT32_MAX;
    if (sum < INT32_MIN) return INT32_MIN;
    return (int32_t)sum;
}

NOINLINE int64_t sat_add_i64(int64_t a, int64_t b)
{
    /* The bounds are checked BEFORE a + b; no __int128 is needed. */
    if (b > 0 && a > INT64_MAX - b) return INT64_MAX;
    if (b < 0 && a < INT64_MIN - b) return INT64_MIN;
    return a + b;
}

NOINLINE uint32_t sat_add_u32(uint32_t a, uint32_t b)
{
    uint32_t sum = a + b;
    uint32_t mask = UINT32_C(0) - (uint32_t)(sum < a);
    return sum | mask;
}

/* Precondition: b != 0. Saturation does not define division by zero. */
NOINLINE int32_t sat_div_i32(int32_t a, int32_t b)
{
    if (a == INT32_MIN && b == -1) return INT32_MAX;
    return a / b;
}

NOINLINE __m128i wrap_add_i16x8(__m128i a, __m128i b)
{
    return _mm_add_epi16(a, b);
}

NOINLINE __m128i sat_add_i16x8(__m128i a, __m128i b)
{
    return _mm_adds_epi16(a, b);
}

NOINLINE __m128i sat_pack_i16_to_i8(__m128i a, __m128i b)
{
    return _mm_packs_epi16(a, b);
}

/*
 * DELIBERATELY WRONG: uses limits of long for an int64_t result.
 * This is a minimal illustration of that portability error, not a copy
 * of a published implementation. Its C arithmetic remains defined.
 */
NOINLINE int64_t wrong_long_limits(int64_t a, int64_t b)
{
    if (b > 0 && a > INT64_MAX - b) return (int64_t)LONG_MAX;
    if (b < 0 && a < INT64_MIN - b) return (int64_t)LONG_MIN;
    return a + b;
}

static void bits16(uint16_t value)
{
    for (unsigned i = 16; i > 0; --i) {
        putchar((((unsigned)value >> (i - 1u)) & 1u) ? '1' : '0');
        if (i > 1u && (i - 1u) % 4u == 0u) putchar(' ');
    }
}

static void show_i16(const char *label, int16_t value)
{
    printf("%-7s = %6d | 0x%04X | ", label, (int)value,
           (unsigned)(uint16_t)value);
    bits16((uint16_t)value);
    putchar('\n');
}

static void demonstrate(void)
{
    const int16_t a[8] = {30000, -30000, 32767, -32768, 100, -100, 20000, -20000};
    const int16_t b[8] = {10000, -10000,     1,     -1, 200, -200, 20000, -20000};
    const int16_t input[16] = {
        100, 127, 128, 200, -100, -128, -129, -200,
        32767, -32768, 0, 1, -1, 126, -127, 42
    };
    int16_t wrap[8], sat[8];
    int8_t packed[16];
    __m128i va, vb, result;

    memcpy(&va, a, sizeof va);
    memcpy(&vb, b, sizeof vb);
    result = wrap_add_i16x8(va, vb);
    memcpy(wrap, &result, sizeof wrap);
    result = sat_add_i16x8(va, vb);
    memcpy(sat, &result, sizeof sat);

    puts("\nSOMA DE OITO PARES DE 16 BITS COM SINAL");
    puts("lane |      A |      B | soma exata |  PADDW | PADDSW");
    for (unsigned i = 0; i < 8; ++i) {
        int32_t exact = (int32_t)a[i] + (int32_t)b[i];
        printf("%4u | %6d | %6d | %10d | %6d | %6d\n", i,
               (int)a[i], (int)b[i], (int)exact, (int)wrap[i], (int)sat[i]);
    }
    puts("\nDetalhe binario da lane 0:");
    show_i16("A", a[0]);
    show_i16("B", b[0]);
    show_i16("PADDW", wrap[0]);
    show_i16("PADDSW", sat[0]);

    memcpy(&va, input, sizeof va);
    memcpy(&vb, input + 8, sizeof vb);
    result = sat_pack_i16_to_i8(va, vb);
    memcpy(packed, &result, sizeof packed);
    puts("\nPACKSSWB: 16 BITS COM SINAL -> 8 BITS COM SINAL");
    for (unsigned i = 0; i < 16; ++i)
        printf("%6d -> %4d\n", (int)input[i], (int)packed[i]);

    puts("\nDEMONSTRACAO: LIMITES DE long USADOS INCORRETAMENTE EM int64_t");
    printf("INT64_MAX + 1: correto=%" PRId64 "; limite errado=%" PRId64 "\n",
           sat_add_i64(INT64_MAX, 1), wrong_long_limits(INT64_MAX, 1));
    printf("INT64_MIN - 1: correto=%" PRId64 "; limite errado=%" PRId64 "\n",
           sat_add_i64(INT64_MIN, -1), wrong_long_limits(INT64_MIN, -1));
    if (sizeof(long) != 4)
        puts("Este host nao e LLP64: a diferenca acima deve ser observada no Windows.");
}

static unsigned test_scalar(void)
{
    const struct { int32_t a, b, expected; } add32[] = {
        {INT32_MAX, 1, INT32_MAX}, {INT32_MIN, -1, INT32_MIN},
        {INT32_MAX, INT32_MAX, INT32_MAX}, {INT32_MIN, INT32_MIN, INT32_MIN},
        {INT32_MAX, INT32_MIN, -1}, {0, INT32_MIN, INT32_MIN},
        {32767, 1, 32768}, {-32768, -1, -32769}, {42, -1, 41}
    };
    const struct { int64_t a, b, expected; } add64[] = {
        {INT64_MAX, 1, INT64_MAX}, {INT64_MIN, -1, INT64_MIN},
        {INT64_MAX, INT64_MAX, INT64_MAX}, {INT64_MIN, INT64_MIN, INT64_MIN},
        {INT64_MAX, INT64_MIN, -1}, {INT64_MIN, INT64_MAX, -1},
        {0, INT64_MIN, INT64_MIN}, {INT64_MIN, 1, INT64_MIN + 1},
        {INT64_MAX, -1, INT64_MAX - 1}, {42, -1, 41}
    };
    const struct { uint32_t a, b, expected; } addu32[] = {
        {UINT32_MAX, 1, UINT32_MAX}, {UINT32_MAX, UINT32_MAX, UINT32_MAX},
        {UINT32_MAX, 0, UINT32_MAX}, {1, 2, 3}, {0, 0, 0}
    };
    const struct { int32_t a, b, expected; } div32[] = {
        {INT32_MIN, -1, INT32_MAX}, {1, INT32_MAX, 0},
        {INT32_MIN, INT32_MAX, -1}, {INT32_MIN, 1, INT32_MIN},
        {INT32_MAX, -1, -INT32_MAX}, {10, 3, 3}, {-10, 3, -3}
    };
    unsigned failures = 0;

    for (size_t i = 0; i < sizeof add32 / sizeof add32[0]; ++i)
        if (sat_add_i32(add32[i].a, add32[i].b) != add32[i].expected) ++failures;
    for (size_t i = 0; i < sizeof add64 / sizeof add64[0]; ++i)
        if (sat_add_i64(add64[i].a, add64[i].b) != add64[i].expected) ++failures;
    for (size_t i = 0; i < sizeof addu32 / sizeof addu32[0]; ++i)
        if (sat_add_u32(addu32[i].a, addu32[i].b) != addu32[i].expected) ++failures;
    for (size_t i = 0; i < sizeof div32 / sizeof div32[0]; ++i)
        if (sat_div_i32(div32[i].a, div32[i].b) != div32[i].expected) ++failures;

    return failures;
}

static unsigned test_simd(void)
{
    const int16_t probe[] = {-32768, -32767, -129, -1, 0, 1, 127, 32766, 32767};
    const int16_t pack_input[16] = {
        100, 127, 128, 200, -100, -128, -129, -200,
        32767, -32768, 0, 1, -1, 126, -127, 42
    };
    const int8_t pack_expected[16] = {
        100, 127, 127, 127, -100, -128, -128, -128,
        127, -128, 0, 1, -1, 126, -127, 42
    };
    unsigned failures = 0;
    __m128i va, vb, result;
    int8_t pack_output[16];

    /* Every int16_t value is checked against nine boundary probes.
     * The scalar result is compared against the hardware SIMD operation.
     */
    for (size_t p = 0; p < sizeof probe / sizeof probe[0]; ++p) {
        int16_t b[8];
        for (unsigned i = 0; i < 8; ++i) b[i] = probe[p];
        memcpy(&vb, b, sizeof vb);
        for (int32_t base = INT16_MIN; base <= INT16_MAX - 7; base += 8) {
            int16_t a[8], sat[8], wrap[8];
            for (unsigned i = 0; i < 8; ++i) a[i] = (int16_t)(base + (int32_t)i);
            memcpy(&va, a, sizeof va);
            result = sat_add_i16x8(va, vb);
            memcpy(sat, &result, sizeof sat);
            result = wrap_add_i16x8(va, vb);
            memcpy(wrap, &result, sizeof wrap);
            for (unsigned i = 0; i < 8; ++i) {
                int32_t exact = (int32_t)a[i] + (int32_t)probe[p];
                if (sat[i] != sat_add_i16(a[i], probe[p])) ++failures;
                if ((uint16_t)wrap[i] != (uint16_t)exact) ++failures;
            }
        }
    }

    memcpy(&va, pack_input, sizeof va);
    memcpy(&vb, pack_input + 8, sizeof vb);
    result = sat_pack_i16_to_i8(va, vb);
    memcpy(pack_output, &result, sizeof pack_output);
    for (unsigned i = 0; i < 16; ++i)
        if (pack_output[i] != pack_expected[i]) ++failures;

    return failures;
}

int main(void)
{
    unsigned failures;

    puts("SATURATION LAB: Windows x64 / MSVC");
#if defined(_MSC_VER)
    printf("_MSC_VER=%d; _MSC_FULL_VER=%d; target=_M_X64\n",
           _MSC_VER, _MSC_FULL_VER);
#else
    puts("LOGICA EXECUTADA EM OUTRO COMPILADOR; ISTO NAO VALIDA O MSVC.");
#endif
    printf("sizeof(short)=%zu; sizeof(int)=%zu; sizeof(long)=%zu; "
           "sizeof(long long)=%zu; sizeof(void*)=%zu\n",
           sizeof(short), sizeof(int), sizeof(long), sizeof(long long), sizeof(void *));
    printf("LONG_MAX=%ld; INT64_MAX=%" PRId64 "\n", LONG_MAX, INT64_MAX);

    demonstrate();
    failures = test_scalar() + test_simd();
    printf("\nTESTES: %s; falhas=%u\n", failures == 0 ? "PASS" : "FAIL", failures);
    return failures == 0 ? 0 : 1;
}
