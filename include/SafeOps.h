#ifndef SAFE_OPS_H
#define SAFE_OPS_H

#include <stddef.h>  // For size_t
#include <stdbool.h> // For bool
#include <stdio.h>
#include <wchar.h>   // For wide string support

/* Error handling enhancements */
typedef enum {
    SAFEOPS_OK = 0,
    SAFEOPS_ERR_NULL_POINTER,
    SAFEOPS_ERR_OUT_OF_BOUNDS,
    SAFEOPS_ERR_OVERFLOW,
    SAFEOPS_ERR_INVALID_PARAM,
    SAFEOPS_ERR_ALLOCATION_FAILED,
    SAFEOPS_ERR_FILE_ACCESS,
    SAFEOPS_ERR_OVERLAP,
    SAFEOPS_ERR_UNKNOWN
} SafeOpsError;

/* Error logging callback type */
typedef void (*SafeOpsLogFunc)(SafeOpsError error, const char* message, const char* file, int line);

/* Configure error handling */
void SafeOpsSetLogger(SafeOpsLogFunc logger);
SafeOpsError SafeOpsGetLastError(void);

/* Memory allocation with different initialization strategies */
void* SafeMalloc(size_t size);                    /* Zero-initialized allocation */
void* SafeMallocUninitialized(size_t size);       /* Non-initialized for performance */
void SafeFree(void **ptrRef);
bool SafeFreeTyped(void **ptrRef, size_t size);   /* With secure clearing */

/* Generic version of SafeFree for typed pointers */
#define SAFE_FREE(type, ptr) SafeFreeTyped((void**)ptr, sizeof(type))

/* Enhanced string operations - NULL terminated strings */
bool SafeStrCopy(char *dest, size_t destSize, const char *src);
bool SafeStrCat(char *dest, size_t destSize, const char *src);
bool SafeStrLen(const char *str, size_t maxLen, size_t *outLen);
bool SafeStrNCopy(char *dest, size_t destSize, const char *src, size_t count);
bool SafeStrNCat(char *dest, size_t destSize, const char *src, size_t count);

/* New string search and replace operations */
bool SafeStrFind(const char *haystack, size_t haystackLen,
                 const char *needle, size_t *outPos);
bool SafeStrReplace(char *str, size_t strSize,
                    const char *oldStr, const char *newStr,
                    size_t *outLen);

/* Wide string operations (wchar_t) */
bool SafeWStrCopy(wchar_t *dest, size_t destSize, const wchar_t *src);
bool SafeWStrCat(wchar_t *dest, size_t destSize, const wchar_t *src);
bool SafeWStrLen(const wchar_t *str, size_t maxLen, size_t *outLen);
bool SafeWStrNCopy(wchar_t *dest, size_t destSize, const wchar_t *src, size_t count);
bool SafeWStrNCat(wchar_t *dest, size_t destSize, const wchar_t *src, size_t count);


/* Memory operations */
bool SafeMemCopy(void *dest, size_t destSize, const void *src, size_t srcSize);
bool SafeMemMove(void *dest, size_t destSize, const void *src, size_t srcSize);
#define SAFE_MEMZERO(ptr, size) do { \
    volatile unsigned char *volatile p = (volatile unsigned char *)(ptr); \
    size_t sz = (size); \
    while (sz--) *p++ = 0; \
} while(0)

/* Array operations */
bool SafeWriteInt(int *array, size_t arraySize, size_t index, int value);
bool SafeReadInt(const int *array, size_t arraySize, size_t index, int *outValue);

/* Pointer arithmetic */
void* SafePointerOffset(void *base, size_t baseSize, size_t offset);

/* Arithmetic operations */
bool SafeAddInt(int a, int b, int *result);
bool SafeSubInt(int a, int b, int *result);
bool SafeMulInt(int a, int b, int *result);
bool SafeDivInt(int a, int b, int *result);
bool SafeCastLongLongToInt(long long val, int *out);

/* Enhanced printf with format validation */
int SafePrintf(const char *format, ...);
int SafeSnprintf(char *str, size_t size, const char *format, ...);

/* File operations with enhanced security notes */
typedef struct {
    bool followSymlinks;      /* Whether to follow symbolic links */
    bool requireRegularFile;  /* Require target to be a regular file */
    unsigned int createMode;  /* File creation mode (POSIX: 0644, Windows: ignored) */
    bool secureDelete;       /* Overwrite file contents on delete */
} SafeFileOpts;

FILE* SafeFOpen(const char *filePath, const char *mode, const SafeFileOpts *opts);
bool SafeFClose(FILE **fp);  /* Secure close with NULL assignment */

/* Validation helpers */
bool IsValidPointer(const void *ptr);
bool IsAligned(const void *ptr, size_t alignment);

#endif // SAFE_OPS_H