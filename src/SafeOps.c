/* SafeOps.c - Cross-platform implementation of safe operations */

#include "../include/SafeOps.h"
#include <errno.h>
#include <limits.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Platform-specific includes */
#ifdef _WIN32
#include <sys/stat.h>
#include <windows.h>
#define THREAD_LOCAL __declspec(thread)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#else
#include <pthread.h>
#define THREAD_LOCAL __thread
#include <sys/types.h>
    #include <sys/stat.h>
    #include <fcntl.h>
    #if defined(__unix__) || defined(__APPLE__)
        #include <unistd.h>
    #endif
#endif

/* Error handling macro */
#define SAFE_RETURN_VAL_IF_FAIL(cond, retval) \
    do { \
        if (!(cond)) { \
            errno = EINVAL; \
            return (retval); \
        } \
    } while(0)

#define SAFE_RETURN_IF_FAIL(cond) \
    do { \
        if (!(cond)) { \
            errno = EINVAL; \
            return; \
        } \
    } while(0)

static SafeOpsLogFunc g_logger = NULL;
static THREAD_LOCAL SafeOpsError g_lastError = SAFEOPS_OK;
/* ------------------------------------------------------
   1) Safe Memory Allocation / Free
   ------------------------------------------------------ */

static void SetError(SafeOpsError error, const char* message) {
    g_lastError = error;
    if (g_logger) {
        g_logger(error, message, __FILE__, __LINE__);
    }
}

void SafeOpsSetLogger(SafeOpsLogFunc logger) {
    g_logger = logger;
}

SafeOpsError SafeOpsGetLastError(void) {
    return g_lastError;
}

/* Memory allocation with performance considerations */
void* SafeMalloc(size_t size) {
    /* Check for zero size */
    if (size == 0) {
        SetError(SAFEOPS_ERR_INVALID_PARAM, "Zero size allocation requested");
        return NULL;
    }

    /* Overflow check that doesn't unnecessarily limit large allocations */
    if (size > (SIZE_MAX - sizeof(size_t))) {
        SetError(SAFEOPS_ERR_OVERFLOW, "Allocation size would overflow");
        return NULL;
    }

    void *ptr = calloc(1, size);
    if (!ptr) {
        SetError(SAFEOPS_ERR_ALLOCATION_FAILED, "Memory allocation failed");
        return NULL;
    }

    return ptr;
}

void* SafeMallocUninitialized(size_t size) {
    if (size == 0) {
        SetError(SAFEOPS_ERR_INVALID_PARAM, "Zero size allocation requested");
        return NULL;
    }

    if (size > (SIZE_MAX - sizeof(size_t))) {
        SetError(SAFEOPS_ERR_OVERFLOW, "Allocation size would overflow");
        return NULL;
    }

    void *ptr = malloc(size);
    if (!ptr) {
        SetError(SAFEOPS_ERR_ALLOCATION_FAILED, "Memory allocation failed");
        return NULL;
    }

    return ptr;
}

void SafeFree(void **ptrRef)
{
    if (!ptrRef || !*ptrRef) {
        return;
    }

    free(*ptrRef);
    *ptrRef = NULL;
}

bool SafeFreeTyped(void **ptrRef, size_t size)
{
    if (!ptrRef || !*ptrRef) {
        errno = EINVAL;
        return false;
    }

    /* Optional: Zero out memory before freeing for security */
    memset(*ptrRef, 0, size);

    free(*ptrRef);
    *ptrRef = NULL;
    return true;
}

/* ------------------------------------------------------
   2) Safe Copy / Move
   ------------------------------------------------------ */

bool SafeMemCopy(void *dest, size_t destSize, const void *src, size_t srcSize) {
    if (!dest || !src) {
        SetError(SAFEOPS_ERR_NULL_POINTER, "NULL pointer in SafeMemCopy");
        return false;
    }

    if (srcSize > destSize) {
        SetError(SAFEOPS_ERR_OUT_OF_BOUNDS, "Source size exceeds destination buffer");
        return false;
    }

    /* Overlap check is optional since we use memmove */
    if ((src < dest && (char*)src + srcSize > (char*)dest) ||
        (dest < src && (char*)dest + destSize > (char*)src)) {
        /* Log overlap but proceed since memmove handles it */
        if (g_logger) {
            g_logger(SAFEOPS_OK, "Memory regions overlap, using memmove", __FILE__, __LINE__);
        }
    }

    memmove(dest, src, srcSize);
    return true;
}

/* Enhanced string handling */
bool SafeStrLen(const char *str, size_t maxLen, size_t *outLen) {
    if (!str || !outLen) {
        SetError(SAFEOPS_ERR_NULL_POINTER, "NULL pointer in SafeStrLen");
        return false;
    }

    size_t len = 0;
    while (len < maxLen && str[len] != '\0') {
        len++;
    }

    if (len == maxLen && str[len] != '\0') {
        SetError(SAFEOPS_ERR_OUT_OF_BOUNDS, "String exceeds maximum length");
        return false;
    }

    *outLen = len;
    return true;
}

bool SafeWStrLen(const wchar_t *str, size_t maxLen, size_t *outLen)
{
    if (!str || !outLen) {
        SetError(SAFEOPS_ERR_NULL_POINTER, "NULL pointer in SafeWStrLen");
        return false;
    }

    size_t len = 0;
    while (len < maxLen && str[len] != L'\0') {
        len++;
    }

    if (len == maxLen && str[len] != L'\0') {
        SetError(SAFEOPS_ERR_OUT_OF_BOUNDS, "String exceeds maximum length");
        return false;
    }

    *outLen = len;
    return true;
}

bool SafeStrCat(char *dest, size_t destSize, const char *src) {
    if (!dest || !src) {
        SetError(SAFEOPS_ERR_NULL_POINTER, "NULL pointer in SafeStrCat");
        return false;
    }

    size_t destLen, srcLen;
    if (!SafeStrLen(dest, destSize, &destLen) ||
        !SafeStrLen(src, SIZE_MAX, &srcLen)) {
        return false;
    }

    if (destLen + srcLen >= destSize) {
        SetError(SAFEOPS_ERR_OUT_OF_BOUNDS, "Concatenation would overflow buffer");
        return false;
    }

    memcpy(dest + destLen, src, srcLen + 1);
    return true;
}

bool SafeStrCopy(char *dest, size_t destSize, const char *src)
{
    SAFE_RETURN_VAL_IF_FAIL(dest && src && destSize > 0, false);

    size_t srcLen = strlen(src);
    if (srcLen >= destSize) {
        errno = EOVERFLOW;
        return false;
    }

    strncpy(dest, src, destSize - 1);
    dest[destSize - 1] = '\0';
    return true;
}

bool SafeWStrNCopy(wchar_t *dest, size_t destSize, const wchar_t *src, size_t count) {
    if (!dest || !src) {
        SetError(SAFEOPS_ERR_NULL_POINTER, "NULL pointer in SafeWStrNCopy");
        return false;
    }

    if (destSize == 0) {
        SetError(SAFEOPS_ERR_INVALID_PARAM, "Destination size is 0");
        return false;
    }

    size_t srcLen;
    if (!SafeWStrLen(src, count, &srcLen)) {
        return false;
    }

    /* Ensure space for null terminator */
    size_t copyLen = (srcLen < count) ? srcLen : count;
    if (copyLen >= destSize) {
        SetError(SAFEOPS_ERR_OUT_OF_BOUNDS, "Insufficient destination buffer size");
        return false;
    }

    wcsncpy(dest, src, copyLen);
    dest[copyLen] = L'\0';
    return true;
}

bool SafeWStrNCat(wchar_t *dest, size_t destSize, const wchar_t *src, size_t count) {
    if (!dest || !src) {
        SetError(SAFEOPS_ERR_NULL_POINTER, "NULL pointer in SafeWStrNCat");
        return false;
    }

    size_t destLen, srcLen;
    if (!SafeWStrLen(dest, destSize, &destLen)) {
        return false;
    }

    if (!SafeWStrLen(src, count, &srcLen)) {
        return false;
    }

    size_t copyLen = (srcLen < count) ? srcLen : count;
    if (destLen + copyLen >= destSize) {
        SetError(SAFEOPS_ERR_OUT_OF_BOUNDS, "Concatenation would overflow buffer");
        return false;
    }

    wcsncat(dest, src, copyLen);
    dest[destLen + copyLen] = L'\0';
    return true;
}

bool SafeStrFind(const char *haystack, size_t haystackLen,
                 const char *needle, size_t *outPos) {
    if (!haystack || !needle || !outPos) {
        SetError(SAFEOPS_ERR_NULL_POINTER, "NULL pointer in SafeStrFind");
        return false;
    }

    size_t needleLen = strlen(needle);
    if (needleLen == 0 || needleLen > haystackLen) {
        SetError(SAFEOPS_ERR_INVALID_PARAM, "Invalid needle length");
        return false;
    }

    /* Search for the needle in the haystack */
    const char *found = strstr(haystack, needle);
    if (!found) {
        *outPos = haystackLen;  /* Convention: not found = length of haystack */
        return true;
    }

    *outPos = (size_t)(found - haystack);
    return true;
}

bool SafeStrReplace(char *str, size_t strSize,
                    const char *oldStr, const char *newStr,
                    size_t *outLen) {
    if (!str || !oldStr || !newStr || !outLen) {
        SetError(SAFEOPS_ERR_NULL_POINTER, "NULL pointer in SafeStrReplace");
        return false;
    }

    size_t strLen = strlen(str);
    size_t oldLen = strlen(oldStr);
    size_t newLen = strlen(newStr);

    if (oldLen == 0) {
        SetError(SAFEOPS_ERR_INVALID_PARAM, "Empty string to replace");
        return false;
    }

    /* First, count occurrences to calculate final size */
    size_t count = 0;
    char *pos = str;
    while ((pos = strstr(pos, oldStr)) != NULL) {
        count++;
        pos += oldLen;
    }

    /* Calculate new string length */
    size_t finalLen = strLen + count * (newLen - oldLen);
    if (finalLen >= strSize) {
        SetError(SAFEOPS_ERR_OUT_OF_BOUNDS, "Replacement would overflow buffer");
        return false;
    }

    /* If no replacements needed, return original length */
    if (count == 0) {
        *outLen = strLen;
        return true;
    }

    /* Allocate temporary buffer for the result */
    char *tempBuf = (char *)SafeMalloc(strSize);
    if (!tempBuf) {
        return false;
    }

    /* Perform replacements */
    const char *readPos = str;
    char *writePos = tempBuf;
    while ((pos = strstr(readPos, oldStr)) != NULL) {
        /* Copy chunk before match */
        size_t chunkLen = pos - readPos;
        memcpy(writePos, readPos, chunkLen);
        writePos += chunkLen;

        /* Copy replacement */
        memcpy(writePos, newStr, newLen);
        writePos += newLen;

        /* Move read position */
        readPos = pos + oldLen;
    }

    /* Copy remaining part */
    strcpy(writePos, readPos);

    /* Copy result back to original buffer */
    memcpy(str, tempBuf, finalLen + 1);
    SafeFree((void**)&tempBuf);

    *outLen = finalLen;
    return true;
}

/* ------------------------------------------------------
   3) Safe Indexed Read/Write
   ------------------------------------------------------ */

bool SafeWriteInt(int *array, size_t arraySize, size_t index, int value)
{
    SAFE_RETURN_VAL_IF_FAIL(array, false);
    SAFE_RETURN_VAL_IF_FAIL(index < arraySize, false);

    array[index] = value;
    return true;
}

bool SafeReadInt(const int *array, size_t arraySize, size_t index, int *outValue)
{
    SAFE_RETURN_VAL_IF_FAIL(array && outValue, false);
    SAFE_RETURN_VAL_IF_FAIL(index < arraySize, false);

    *outValue = array[index];
    return true;
}

/* ------------------------------------------------------
   4) Safe Pointer Offsets
   ------------------------------------------------------ */

void* SafePointerOffset(void *base, size_t baseSize, size_t offset)
{
    SAFE_RETURN_VAL_IF_FAIL(base, NULL);

    /* Check for overflow in pointer arithmetic */
    if (offset > baseSize ||
        (uintptr_t)base + offset < (uintptr_t)base) {
        errno = EOVERFLOW;
        return NULL;
    }

    return (char*)base + offset;
}

/* ------------------------------------------------------
   5) Safe Arithmetic
   ------------------------------------------------------ */

bool SafeAddInt(int a, int b, int *result)
{
    SAFE_RETURN_VAL_IF_FAIL(result, false);

    /* Use GCC/Clang builtin if available */
#if defined(__GNUC__) || defined(__clang__)
    if (__builtin_sadd_overflow(a, b, result)) {
            errno = EOVERFLOW;
            return false;
        }
#else
    if ((b > 0 && a > INT_MAX - b) ||
        (b < 0 && a < INT_MIN - b)) {
        errno = EOVERFLOW;
        return false;
    }
    *result = a + b;
#endif

    return true;
}

bool SafeCastLongLongToInt(long long val, int *out)
{
    SAFE_RETURN_VAL_IF_FAIL(out, false);

    if (val > INT_MAX || val < INT_MIN) {
        errno = EOVERFLOW;
        return false;
    }

    *out = (int)val;
    return true;
}

/* ------------------------------------------------------
   6) Safe Printf
   ------------------------------------------------------ */

int SafePrintf(const char *format, ...)
{
    SAFE_RETURN_VAL_IF_FAIL(format, -1);

    va_list args;
            va_start(args, format);
    int ret = vprintf(format, args);
            va_end(args);

    return ret;
}

/* ------------------------------------------------------
   7) TOCTOU & File Handling
   ------------------------------------------------------ */

FILE* SafeFOpen(const char *filePath, const char *mode, const SafeFileOpts *opts) {
    if (!filePath || !mode) {
        SetError(SAFEOPS_ERR_NULL_POINTER, "NULL pointer in SafeFOpen");
        return NULL;
    }

    /* Use default options if none provided */
    SafeFileOpts defaultOpts = {
            .followSymlinks = false,
            .requireRegularFile = true,
            .createMode = 0644,
            .secureDelete = false
    };
    if (!opts) opts = &defaultOpts;

    FILE *fp = NULL;
    struct stat st;

#ifdef _WIN32
    /* Windows implementation */
    if (fopen_s(&fp, filePath, mode) != 0) {
        SetError(SAFEOPS_ERR_FILE_ACCESS, "Failed to open file");
        return NULL;
    }

    int fd = _fileno(fp);
    if (_fstat(fd, &st) != 0) {
        fclose(fp);
        SetError(SAFEOPS_ERR_FILE_ACCESS, "Failed to stat file");
        return NULL;
    }
#else
    /* POSIX implementation with enhanced security */
    int flags = O_RDONLY;
    if (strchr(mode, 'w')) flags = O_WRONLY | O_CREAT | O_TRUNC;
    else if (strchr(mode, 'a')) flags = O_WRONLY | O_CREAT | O_APPEND;

    if (!opts->followSymlinks) {
        #ifdef O_NOFOLLOW
            flags |= O_NOFOLLOW;
        #endif
    }

    int fd = open(filePath, flags, opts->createMode);
    if (fd == -1) {
        SetError(SAFEOPS_ERR_FILE_ACCESS, "Failed to open file");
        return NULL;
    }

    /* Get file information using the file descriptor to avoid TOCTOU */
    if (fstat(fd, &st) != 0) {
        close(fd);
        SetError(SAFEOPS_ERR_FILE_ACCESS, "Failed to stat file");
        return NULL;
    }
#endif

    /* Verify file type if required */
    if (opts->requireRegularFile && !S_ISREG(st.st_mode)) {
#ifdef _WIN32
        fclose(fp);
#else
        close(fd);
#endif
        SetError(SAFEOPS_ERR_FILE_ACCESS, "Not a regular file");
        return NULL;
    }

#ifndef _WIN32
    fp = fdopen(fd, mode);
        if (!fp) {
            close(fd);
            SetError(SAFEOPS_ERR_FILE_ACCESS, "Failed to create FILE stream");
            return NULL;
        }
#endif

    return fp;
}

/* ------------------------------------------------------
   8) Additional Helpers
   ------------------------------------------------------ */

bool IsValidPointer(const void *ptr)
{
    return ptr != NULL;
}