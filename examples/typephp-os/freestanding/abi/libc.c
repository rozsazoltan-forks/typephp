/*
   +----------------------------------------------------------------------+
   | TypePHP OS                                                          |
   +----------------------------------------------------------------------+
   | Small freestanding C/POSIX compatibility layer for Zend bootstrap.  |
   | SPDX-License-Identifier: BSD-3-Clause                               |
   +----------------------------------------------------------------------+

   This is not a replacement for zend_alloc. It only supplies the host calls
   used by zend_alloc to acquire its aligned 2 MiB chunks, plus the C memory
   primitives emitted by C/C++ compilers. User allocations continue through
   emalloc/efree and therefore retain the original Zend allocator semantics.
*/

#include "typephp_os_abi.h"

#include <stdarg.h>
#include <locale.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <inttypes.h>

typedef struct typephp_os_block {
    size_t size;
} typephp_os_block;

static uintptr_t arena_cursor;
static uintptr_t arena_end;
static int typephp_os_errno;

/* TypePHP OS has no hosted stdio object. These opaque values only satisfy
 * upstream error paths; bytes are forwarded through the host write hook. */
void *stdin = (void *) 0;
void *stdout = (void *) 1;
void *stderr = (void *) 2;

size_t strlen(const char *string);
void *malloc(size_t size);
int posix_memalign(void **result, size_t alignment, size_t size);
int vsnprintf(char *buffer, size_t size, const char *format, va_list args);

int *__errno_location(void)
{
    return &typephp_os_errno;
}

void __assert_fail(
    const char *assertion,
    const char *file,
    unsigned int line,
    const char *function)
{
    (void) file;
    (void) line;
    (void) function;
    typephp_os_panic(assertion);
}

static uintptr_t align_up(uintptr_t value, size_t alignment)
{
    return (value + alignment - 1u) & ~((uintptr_t) alignment - 1u);
}

static int is_power_of_two(size_t value)
{
    return value != 0 && (value & (value - 1u)) == 0;
}

void typephp_os_memory_init(void *address, size_t size)
{
    const uintptr_t begin = (uintptr_t) address;
    if (size > UINTPTR_MAX - begin) {
        arena_cursor = 0;
        arena_end = 0;
        return;
    }
    arena_cursor = begin;
    arena_end = begin + size;
}

size_t typephp_os_memory_available(void)
{
    return arena_end >= arena_cursor ? (size_t) (arena_end - arena_cursor) : 0;
}

__attribute__((weak)) void typephp_os_write(const char *data, size_t size)
{
    (void) data;
    (void) size;
}

__attribute__((weak, noreturn)) void typephp_os_panic(const char *message)
{
    typephp_os_write(message, strlen(message));
    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}

void phpx_no_exception_abort(const char *message)
{
    typephp_os_panic(message);
}

int php_nano_host_random_bytes(void *bytes, size_t size)
{
    static uint64_t state = UINT64_C(0x9e3779b97f4a7c15);
    unsigned char *output = (unsigned char *) bytes;
    while (size-- != 0) {
        state ^= state << 13;
        state ^= state >> 7;
        state ^= state << 17;
        *output++ = (unsigned char) state;
    }
    return 0;
}

uint64_t php_nano_host_random_seed(void)
{
    uint64_t seed = 0;
    (void) php_nano_host_random_bytes(&seed, sizeof(seed));
    return seed;
}

void *memset(void *destination, int value, size_t size)
{
    unsigned char *output = (unsigned char *) destination;
    while (size-- != 0) {
        *output++ = (unsigned char) value;
    }
    return destination;
}

void *memcpy(void *destination, const void *source, size_t size)
{
    unsigned char *output = (unsigned char *) destination;
    const unsigned char *input = (const unsigned char *) source;
    while (size-- != 0) {
        *output++ = *input++;
    }
    return destination;
}

void *memmove(void *destination, const void *source, size_t size)
{
    unsigned char *output = (unsigned char *) destination;
    const unsigned char *input = (const unsigned char *) source;
    if (output <= input || output >= input + size) {
        return memcpy(destination, source, size);
    }
    output += size;
    input += size;
    while (size-- != 0) {
        *--output = *--input;
    }
    return destination;
}

int memcmp(const void *left, const void *right, size_t size)
{
    const unsigned char *a = (const unsigned char *) left;
    const unsigned char *b = (const unsigned char *) right;
    while (size-- != 0) {
        if (*a != *b) {
            return *a < *b ? -1 : 1;
        }
        ++a;
        ++b;
    }
    return 0;
}

size_t strlen(const char *string)
{
    const char *end = string;
    while (*end != '\0') {
        ++end;
    }
    return (size_t) (end - string);
}

char *strdup(const char *string)
{
    const size_t size = strlen(string) + 1;
    char *copy = (char *) malloc(size);
    return copy ? (char *) memcpy(copy, string, size) : 0;
}

int strcmp(const char *left, const char *right)
{
    while (*left != '\0' && *left == *right) {
        ++left;
        ++right;
    }
    return (unsigned char) *left - (unsigned char) *right;
}

int strncmp(const char *left, const char *right, size_t size)
{
    while (size-- != 0) {
        const unsigned char a = (unsigned char) *left++;
        const unsigned char b = (unsigned char) *right++;
        if (a != b) {
            return (int) a - (int) b;
        }
        if (a == 0) {
            return 0;
        }
    }
    return 0;
}

void *memchr(const void *memory, int character, size_t size)
{
    const unsigned char *cursor = (const unsigned char *) memory;
    const unsigned char value = (unsigned char) character;
    while (size-- != 0) {
        if (*cursor == value) {
            return (void *) cursor;
        }
        ++cursor;
    }
    return 0;
}

char *strchr(const char *string, int character)
{
    do {
        if (*string == (char) character) {
            return (char *) string;
        }
    } while (*string++ != '\0');
    return 0;
}

char *strrchr(const char *string, int character)
{
    const char *match = 0;
    do {
        if (*string == (char) character) {
            match = string;
        }
    } while (*string++ != '\0');
    return (char *) match;
}

char *strstr(const char *haystack, const char *needle)
{
    const size_t needle_size = strlen(needle);
    if (needle_size == 0) {
        return (char *) haystack;
    }
    while (*haystack != '\0') {
        if (*haystack == *needle && strncmp(haystack, needle, needle_size) == 0) {
            return (char *) haystack;
        }
        ++haystack;
    }
    return 0;
}

static unsigned char ascii_lower(unsigned char character)
{
    return character >= 'A' && character <= 'Z'
        ? (unsigned char) (character + ('a' - 'A'))
        : character;
}

int strcasecmp(const char *left, const char *right)
{
    while (*left != '\0' && ascii_lower((unsigned char) *left) == ascii_lower((unsigned char) *right)) {
        ++left;
        ++right;
    }
    return (int) ascii_lower((unsigned char) *left) - (int) ascii_lower((unsigned char) *right);
}

int strncasecmp(const char *left, const char *right, size_t size)
{
    while (size-- != 0) {
        const unsigned char a = ascii_lower((unsigned char) *left++);
        const unsigned char b = ascii_lower((unsigned char) *right++);
        if (a != b) {
            return (int) a - (int) b;
        }
        if (a == 0) {
            return 0;
        }
    }
    return 0;
}

int isascii(int character)
{
    return (character & ~0x7f) == 0;
}

int isdigit(int character)
{
    return character >= '0' && character <= '9';
}

int islower(int character)
{
    return character >= 'a' && character <= 'z';
}

int isupper(int character)
{
    return character >= 'A' && character <= 'Z';
}

int isalpha(int character)
{
    return islower(character) || isupper(character);
}

int isalnum(int character)
{
    return isalpha(character) || isdigit(character);
}

int isspace(int character)
{
    return character == ' ' || (character >= '\t' && character <= '\r');
}

int isblank(int character)
{
    return character == ' ' || character == '\t';
}

int iscntrl(int character)
{
    return (character >= 0 && character < 0x20) || character == 0x7f;
}

int isgraph(int character)
{
    return character > 0x20 && character < 0x7f;
}

int isprint(int character)
{
    return character >= 0x20 && character < 0x7f;
}

int ispunct(int character)
{
    return isgraph(character) && !isalnum(character);
}

size_t __ctype_get_mb_cur_max(void)
{
    return 1;
}

int isxdigit(int character)
{
    return isdigit(character)
        || (character >= 'a' && character <= 'f')
        || (character >= 'A' && character <= 'F');
}

int tolower(int character)
{
    return isupper(character) ? character + ('a' - 'A') : character;
}

int toupper(int character)
{
    return islower(character) ? character - ('a' - 'A') : character;
}

struct lconv *localeconv(void)
{
    static struct lconv value = {
        .decimal_point = ".",
        .thousands_sep = "",
        .grouping = "",
    };
    return &value;
}

char *setlocale(int category, const char *locale)
{
    static char c_locale[] = "C";
    (void) category;
    if (locale == 0 || strcmp(locale, "C") == 0 || strcmp(locale, "POSIX") == 0) {
        return c_locale;
    }
    return 0;
}

char *getcwd(char *buffer, size_t size)
{
    if (buffer == 0 || size < 2) {
        typephp_os_errno = 34; /* ERANGE */
        return 0;
    }
    buffer[0] = '/';
    buffer[1] = '\0';
    return buffer;
}

char *getenv(const char *name)
{
    /* The kernel process starts with an intentionally empty environment. */
    (void) name;
    return 0;
}

int strcoll(const char *left, const char *right)
{
    return strcmp(left, right);
}

char *strcat(char *destination, const char *source)
{
    char *result = destination;
    destination += strlen(destination);
    while ((*destination++ = *source++) != '\0') {
    }
    return result;
}

char *strpbrk(const char *string, const char *characters)
{
    for (; *string != '\0'; ++string) {
        if (strchr(characters, *string) != 0) {
            return (char *) string;
        }
    }
    return 0;
}

static int digit_value(int character)
{
    if (character >= '0' && character <= '9') {
        return character - '0';
    }
    if (character >= 'a' && character <= 'z') {
        return character - 'a' + 10;
    }
    if (character >= 'A' && character <= 'Z') {
        return character - 'A' + 10;
    }
    return -1;
}

unsigned long long strtoull(const char *string, char **end, int base)
{
    while (isspace((unsigned char) *string)) {
        ++string;
    }
    if (*string == '+') {
        ++string;
    }
    if ((base == 0 || base == 16) && string[0] == '0'
        && (string[1] == 'x' || string[1] == 'X')) {
        base = 16;
        string += 2;
    } else if (base == 0) {
        base = string[0] == '0' ? 8 : 10;
    }
    const char *cursor = string;
    unsigned long long value = 0;
    int digit;
    while ((digit = digit_value((unsigned char) *cursor)) >= 0 && digit < base) {
        value = value * (unsigned int) base + (unsigned int) digit;
        ++cursor;
    }
    if (end != 0) {
        *end = (char *) cursor;
    }
    return value;
}

long long strtoll(const char *string, char **end, int base)
{
    while (isspace((unsigned char) *string)) {
        ++string;
    }
    const int negative = *string == '-';
    if (*string == '-' || *string == '+') {
        ++string;
    }
    char *parsed_end;
    const unsigned long long value = strtoull(string, &parsed_end, base);
    if (end != 0) {
        *end = parsed_end;
    }
    return negative ? -(long long) value : (long long) value;
}

long strtol(const char *string, char **end, int base)
{
    return (long) strtoll(string, end, base);
}

int atoi(const char *string)
{
    return (int) strtol(string, 0, 10);
}

long long atoll(const char *string)
{
    return strtoll(string, 0, 10);
}

intmax_t imaxabs(intmax_t value)
{
    return value < 0 ? -value : value;
}

double strtod(const char *string, char **end)
{
    while (isspace((unsigned char) *string)) {
        ++string;
    }
    const int negative = *string == '-';
    if (*string == '-' || *string == '+') {
        ++string;
    }
    double value = 0.0;
    while (isdigit((unsigned char) *string)) {
        value = value * 10.0 + (double) (*string++ - '0');
    }
    if (*string == '.') {
        double place = 0.1;
        ++string;
        while (isdigit((unsigned char) *string)) {
            value += (double) (*string++ - '0') * place;
            place *= 0.1;
        }
    }
    if (end != 0) {
        *end = (char *) string;
    }
    return negative ? -value : value;
}

void qsort(void *base, size_t count, size_t width, int (*compare)(const void *, const void *))
{
    unsigned char *bytes = (unsigned char *) base;
    for (size_t index = 1; index < count; ++index) {
        size_t current = index;
        while (current != 0
            && compare(bytes + (current - 1) * width, bytes + current * width) > 0) {
            for (size_t byte = 0; byte < width; ++byte) {
                unsigned char value = bytes[(current - 1) * width + byte];
                bytes[(current - 1) * width + byte] = bytes[current * width + byte];
                bytes[current * width + byte] = value;
            }
            --current;
        }
    }
}

int fflush(void *stream)
{
    (void) stream;
    return 0;
}

int setvbuf(void *stream, char *buffer, int mode, size_t size)
{
    (void) stream;
    (void) buffer;
    (void) mode;
    (void) size;
    return 0;
}

int vprintf(const char *format, va_list args)
{
    char buffer[1024];
    const int written = vsnprintf(buffer, sizeof(buffer), format, args);
    if (written > 0) {
        typephp_os_write(buffer, (size_t) written < sizeof(buffer)
            ? (size_t) written : sizeof(buffer) - 1);
    }
    return written;
}

int printf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    const int written = vprintf(format, args);
    va_end(args);
    return written;
}

int _setjmp(jmp_buf environment)
{
    (void) environment;
    return 0;
}

double pow(double base, double exponent)
{
    long power = (long) exponent;
    if ((double) power != exponent) {
        return 0.0;
    }
    double result = 1.0;
    unsigned long magnitude = power < 0 ? (unsigned long) (-power) : (unsigned long) power;
    while (magnitude != 0) {
        if ((magnitude & 1u) != 0) {
            result *= base;
        }
        base *= base;
        magnitude >>= 1u;
    }
    return power < 0 ? 1.0 / result : result;
}

double fmod(double value, double divisor)
{
    if (divisor == 0.0) {
        return 0.0 / 0.0;
    }
    double quotient = value / divisor;
    if (quotient >= 0.0) {
        quotient = (double) (unsigned long) quotient;
    } else {
        quotient = (double) (long) quotient;
    }
    return value - quotient * divisor;
}

double ceil(double value)
{
    long integral = (long) value;
    return value > (double) integral ? (double) integral + 1.0 : (double) integral;
}

double floor(double value)
{
    long integral = (long) value;
    return value < (double) integral ? (double) integral - 1.0 : (double) integral;
}

double trunc(double value)
{
    return (double) (long) value;
}

double round(double value)
{
    return value < 0.0 ? ceil(value - 0.5) : floor(value + 0.5);
}

double fabs(double value)
{
    return value < 0.0 ? -value : value;
}

int abs(int value)
{
    return value < 0 ? -value : value;
}

long long llabs(long long value)
{
    return value < 0 ? -value : value;
}

int ap_php_vsnprintf(char *buffer, size_t size, const char *format, va_list args);

int vsnprintf(char *buffer, size_t size, const char *format, va_list args)
{
    return ap_php_vsnprintf(buffer, size, format, args);
}

int snprintf(char *buffer, size_t size, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    const int result = ap_php_vsnprintf(buffer, size, format, args);
    va_end(args);
    return result;
}

int __snprintf_chk(char *buffer, size_t size, int flag, size_t buffer_size, const char *format, ...)
{
    (void) flag;
    (void) buffer_size;
    va_list args;
    va_start(args, format);
    const int result = ap_php_vsnprintf(buffer, size, format, args);
    va_end(args);
    return result;
}

__attribute__((noreturn)) void __longjmp_chk(void *environment, int value)
{
    (void) environment;
    (void) value;
    typephp_os_panic("zend_bailout");
}

__attribute__((noreturn)) void longjmp(jmp_buf environment, int value)
{
    (void) environment;
    (void) value;
    typephp_os_panic("zend_bailout");
}

int fputs(const char *string, void *stream)
{
    (void) stream;
    typephp_os_write(string, strlen(string));
    return 0;
}

int fputc(int character, void *stream)
{
    (void) stream;
    const char byte = (char) character;
    typephp_os_write(&byte, 1);
    return (unsigned char) byte;
}

size_t fwrite(const void *data, size_t size, size_t count, void *stream)
{
    (void) stream;
    if (size != 0 && count > SIZE_MAX / size) {
        return 0;
    }
    typephp_os_write((const char *) data, size * count);
    return count;
}

int fprintf(void *stream, const char *format, ...)
{
    (void) stream;
    /* Zend's allocator only uses this on fatal paths. Keep the implementation
     * allocation-free; the complete formatter belongs to the standard layer. */
    typephp_os_write(format, strlen(format));
    return (int) strlen(format);
}

int __fprintf_chk(void *stream, int flag, const char *format, ...)
{
    (void) flag;
    return fprintf(stream, format);
}

void *malloc(size_t size)
{
    void *result = 0;
    if (posix_memalign(&result, 16, size) != 0) {
        return 0;
    }
    return result;
}

void free(void *pointer)
{
    /* The bootstrap arena is monotonic. Zend's own allocations are reclaimed
     * by zend_mm; backing chunks remain reserved until the kernel exits. */
    (void) pointer;
}

void *calloc(size_t count, size_t size)
{
    if (count != 0 && size > SIZE_MAX / count) {
        return 0;
    }
    const size_t bytes = count * size;
    void *result = malloc(bytes);
    if (result != 0) {
        memset(result, 0, bytes);
    }
    return result;
}

void *realloc(void *pointer, size_t size)
{
    if (pointer == 0) {
        return malloc(size);
    }
    if (size == 0) {
        return 0;
    }
    typephp_os_block *old = (typephp_os_block *) pointer - 1;
    void *replacement = malloc(size);
    if (replacement != 0) {
        memcpy(replacement, pointer, old->size < size ? old->size : size);
    }
    return replacement;
}

int posix_memalign(void **result, size_t alignment, size_t size)
{
    if (result == 0 || !is_power_of_two(alignment) || alignment < sizeof(void *)) {
        return 22; /* EINVAL */
    }
    if (arena_cursor == 0 || size > SIZE_MAX - sizeof(typephp_os_block)) {
        *result = 0;
        return 12; /* ENOMEM */
    }

    const uintptr_t payload = align_up(
        arena_cursor + sizeof(typephp_os_block), alignment);
    if (payload > arena_end || size > arena_end - payload) {
        *result = 0;
        return 12;
    }
    typephp_os_block *block = (typephp_os_block *) payload - 1;
    block->size = size;
    arena_cursor = payload + size;
    *result = (void *) payload;
    return 0;
}

__attribute__((noreturn)) void abort(void)
{
    typephp_os_panic("abort");
}

__attribute__((noreturn)) void exit(int status)
{
    (void) status;
    typephp_os_panic("exit");
}

__attribute__((noreturn)) void _Exit(int status)
{
    exit(status);
}
