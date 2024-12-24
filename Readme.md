# SafeOperations Library

A comprehensive C99 library providing safe operations for common programming tasks. This library is designed to prevent common security vulnerabilities and programming errors by providing bounds-checked, null-checked, and overflow-checked alternatives to standard C functions.

## Features

- Memory allocation and deallocation safety
- String manipulation with bounds checking
- Array access protection
- Pointer arithmetic safety
- Integer arithmetic with overflow protection
- File operations with TOCTOU mitigations
- Cross-platform compatibility (Windows, Linux, macOS)
- Both static and dynamic library versions

## Building the Library

### Prerequisites
- CMake 3.10 or higher
- C99 compliant compiler (GCC, Clang, or MSVC)

### Build Instructions
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

### Build Outputs
- Windows: libSafeOperations.lib (static), libSafeOperations.dll
- Linux: libSafeOperations.a (static), libSafeOperations.so
- macOS: libSafeOperations.a (static), libSafeOperations.dylib

## API Reference

### Memory Management

#### `void* SafeMalloc(size_t size)`
Safely allocates memory with overflow checks and zero initialization.
- Returns NULL if size is 0 or if allocation fails
- Automatically zeros allocated memory
- Checks for size_t overflow

#### `void SafeFree(void **ptrRef)`
Safely frees memory and nulls the pointer.
- Prevents double-free vulnerabilities
- Nulls the pointer after freeing
- Handles NULL pointers safely

#### `bool SafeFreeTyped(void **ptrRef, size_t size)`
Type-safe version of SafeFree with secure memory clearing.
- Clears memory before freeing
- Returns success/failure status
- Size parameter enables secure clearing

### String Operations

#### `bool SafeStrCopy(char *dest, size_t destSize, const char *src)`
Safe string copy with bounds checking.
- Ensures null termination
- Prevents buffer overflow
- Returns false if copy would overflow

#### `bool SafeStrCat(char *dest, size_t destSize, const char *src)`
Safe string concatenation.
- Checks destination buffer size
- Ensures null termination
- Prevents buffer overflow

#### `bool SafeStrFind(const char *haystack, size_t haystackLen, const char *needle, size_t *outPos)`
Safe string search operation.
- Bounds-checked search
- Returns position through outPos parameter
- Returns false on invalid parameters

#### `bool SafeStrReplace(char *str, size_t strSize, const char *oldStr, const char *newStr, size_t *outLen)`
Safe string replacement.
- Handles multiple replacements
- Checks buffer size
- Returns new length through outLen

### Wide String Operations

#### `bool SafeWStrNCopy(wchar_t *dest, size_t destSize, const wchar_t *src, size_t count)`
Safe wide string copy with count limit.
- Unicode-aware
- Bounds-checked
- Null-termination guaranteed

#### `bool SafeWStrNCat(wchar_t *dest, size_t destSize, const wchar_t *src, size_t count)`
Safe wide string concatenation with count limit.
- Unicode-aware
- Prevents buffer overflow
- Maintains null termination

### Array Operations

#### `bool SafeWriteInt(int *array, size_t arraySize, size_t index, int value)`
Safe array element writing.
- Bounds checking
- NULL pointer protection
- Returns success/failure status

#### `bool SafeReadInt(const int *array, size_t arraySize, size_t index, int *outValue)`
Safe array element reading.
- Bounds checking
- NULL pointer protection
- Returns value through outValue parameter

### Arithmetic Operations

#### `bool SafeAddInt(int a, int b, int *result)`
Safe integer addition with overflow check.
- Detects overflow/underflow
- Returns false on overflow
- Uses compiler intrinsics when available

#### `bool SafeCastLongLongToInt(long long val, int *out)`
Safe integer type conversion.
- Checks for truncation
- Returns false if value out of range
- Preserves value integrity

### File Operations

#### `FILE* SafeFOpen(const char *filePath, const char *mode, const SafeFileOpts *opts)`
Safe file opening with security options.
```c
SafeFileOpts opts = {
    .followSymlinks = false,
    .requireRegularFile = true,
    .createMode = 0644,
    .secureDelete = false
};
```
- TOCTOU mitigation
- Symlink attack protection
- Platform-specific security features

## Error Handling

All functions that return bool indicate success/failure status. Functions set errno or use the library's error reporting system:

```c
SafeOpsError error = SafeOpsGetLastError();
```

## Thread Safety

The library is thread-safe with the following considerations:
- Memory functions are thread-safe
- String operations are thread-safe for different buffers
- Global error state is thread-local

## Example Usage

```c
// Memory allocation and string handling
char *buffer = SafeMalloc(100);
if (!buffer) {
    // Handle allocation failure
    return;
}

if (!SafeStrCopy(buffer, 100, "Hello, World!")) {
    // Handle copy failure
    SafeFree((void**)&buffer);
    return;
}

// Array operations
int array[10];
if (!SafeWriteInt(array, 10, 5, 42)) {
    // Handle write failure
}

// Clean up
SafeFree((void**)&buffer);
```

## Best Practices

1. Always check return values for functions that return bool
2. Use SafeFree instead of free to prevent double-free
3. Initialize SafeFileOpts when using SafeFOpen
4. Use SAFE_FREE macro for typed pointers
5. Check SafeOpsGetLastError() for detailed error information

## License

This library is provided under the MIT License. See LICENSE file for details.

## Extended Examples

### 1. Safe String Manipulation
```c
#include "SafeOps.h"

void string_operations_example(void) {
    char buffer[100];
    char result[150];
    
    // Safe string copy
    if (!SafeStrCopy(buffer, sizeof(buffer), "Hello, World!")) {
        printf("String copy failed\n");
        return;
    }
    
    // Safe concatenation
    if (!SafeStrCat(buffer, sizeof(buffer), " How are you?")) {
        printf("String concatenation failed\n");
        return;
    }
    
    // Safe string replacement
    size_t newLen;
    if (!SafeStrReplace(buffer, sizeof(buffer), "World", "Everyone", &newLen)) {
        printf("String replacement failed\n");
        return;
    }
    
    // Safe string search
    size_t position;
    if (SafeStrFind(buffer, strlen(buffer), "Everyone", &position)) {
        printf("Found 'Everyone' at position: %zu\n", position);
    }
}
```

### 2. Memory Management with Arrays
```c
void array_operations_example(void) {
    // Allocate an array safely
    size_t arraySize = 100;
    int *numbers = SafeMalloc(arraySize * sizeof(int));
    if (!numbers) {
        printf("Memory allocation failed\n");
        return;
    }
    
    // Safely write to array
    for (size_t i = 0; i < arraySize; i++) {
        if (!SafeWriteInt(numbers, arraySize, i, (int)i * 2)) {
            printf("Failed to write to array at index %zu\n", i);
            SafeFree((void**)&numbers);
            return;
        }
    }
    
    // Safely read from array
    int value;
    if (!SafeReadInt(numbers, arraySize, 50, &value)) {
        printf("Failed to read from array\n");
    } else {
        printf("Value at index 50: %d\n", value);
    }
    
    // Clean up
    SafeFree((void**)&numbers);
}
```

### 3. Safe File Operations
```c
void file_operations_example(void) {
    SafeFileOpts opts = {
        .followSymlinks = false,
        .requireRegularFile = true,
        .createMode = 0644,
        .secureDelete = true
    };
    
    // Open file safely
    FILE *file = SafeFOpen("data.txt", "w", &opts);
    if (!file) {
        printf("Failed to open file: %s\n", strerror(errno));
        return;
    }
    
    // Write data
    fprintf(file, "Hello, World!\n");
    
    // Close file safely
    fclose(file);
}
```

### 4. Safe Arithmetic Operations
```c
void arithmetic_operations_example(void) {
    int a = 2147483640;  // Close to INT_MAX
    int b = 10;
    int result;
    
    // Safe addition
    if (!SafeAddInt(a, b, &result)) {
        printf("Addition would overflow!\n");
    }
    
    // Safe type conversion
    long long bigValue = 9223372036854775807LL;  // LLONG_MAX
    int smallerValue;
    
    if (!SafeCastLongLongToInt(bigValue, &smallerValue)) {
        printf("Value too large for int!\n");
    }
}
```

### 5. Combining Multiple Operations
```c
void combined_operations_example(void) {
    // Allocate buffer
    char *buffer = SafeMalloc(1024);
    if (!buffer) {
        return;
    }
    
    // Read file content
    SafeFileOpts opts = {
        .followSymlinks = false,
        .requireRegularFile = true
    };
    
    FILE *file = SafeFOpen("input.txt", "r", &opts);
    if (!file) {
        SafeFree((void**)&buffer);
        return;
    }
    
    // Process file content safely
    char line[256];
    size_t totalLen = 0;
    
    while (fgets(line, sizeof(line), file)) {
        if (!SafeStrCat(buffer, 1024, line)) {
            printf("Buffer would overflow!\n");
            break;
        }
    }
    
    fclose(file);
    SafeFree((void**)&buffer);
}
```

## Security Considerations

### Memory Safety
1. **Buffer Overflows**
    - All string and memory operations include bounds checking
    - Buffer sizes are verified before operations
    - Functions return false rather than overflow

2. **Use-After-Free**
    - SafeFree nulls pointers after freeing
    - Double-free prevention built into SafeFree
    - Memory is cleared before freeing in SafeFreeTyped

3. **Integer Overflow**
    - Arithmetic operations check for overflow
    - Array index calculations are checked
    - Size calculations for memory allocation are verified

### File Operation Safety
1. **TOCTOU (Time-of-Check-Time-of-Use)**
    - File operations use safe opening procedures
    - Option to prevent symlink following
    - Verification of file type before operations

2. **File Permissions**
    - Explicit mode settings for file creation
    - Platform-specific security features utilized
    - Regular file checking available

### String Manipulation
1. **String Truncation**
    - All string operations guarantee null-termination
    - Length checks include space for null terminator
    - String replacement checks total length needed

2. **Unicode Safety**
    - Wide string operations handle Unicode correctly
    - Length calculations account for wide characters
    - No assumptions about character size

### Best Security Practices
1. **Input Validation**
    - Always check function return values
    - Verify buffer sizes before operations
    - Use SafeFileOpts for file operations

2. **Error Handling**
    - Check error codes via SafeOpsGetLastError
    - Handle allocation failures gracefully
    - Clean up resources on error paths

3. **Resource Management**
    - Use SAFE_FREE macro for typed pointers
    - Always initialize SafeFileOpts struct
    - Close files and free memory in error cases

4. **Thread Safety**
    - Avoid global state
    - Use thread-local error handling
    - Don't share buffers between threads

### Known Limitations
1. Not all operations can be made completely safe in C
2. Some race conditions are inherent to the filesystem
3. Memory operations still depend on the system allocator
4. Complex operations may need additional security measures

### Security Auditing
Consider the following when using this library:
1. Regular security testing of applications
2. Code review focusing on error handling
3. Fuzz testing of input handling
4. Memory leak detection
5. Static analysis tools integration