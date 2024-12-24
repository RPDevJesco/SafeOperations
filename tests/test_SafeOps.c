#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SafeOps.h"

// Function prototypes for our tests
void test_memory_operations(void);
void test_string_operations(void);
void test_wide_string_operations(void);
void test_array_operations(void);
void test_arithmetic_operations(void);
void test_file_operations(void);
void pause_console(void);

int main() {
    int choice;
    do {
        printf("\nSafeOperations Library Test Suite\n");
        printf("================================\n");
        printf("1. Test Memory Operations\n");
        printf("2. Test String Operations\n");
        printf("3. Test Wide String Operations\n");
        printf("4. Test Array Operations\n");
        printf("5. Test Arithmetic Operations\n");
        printf("6. Test File Operations\n");
        printf("7. Run All Tests\n");
        printf("0. Exit\n");
        printf("\nEnter your choice: ");

        if (scanf("%d", &choice) != 1) {
            printf("Invalid input. Please enter a number.\n");
            while (getchar() != '\n'); // Clear input buffer
            continue;
        }

        printf("\n");

        switch (choice) {
            case 1:
                test_memory_operations();
                break;
            case 2:
                test_string_operations();
                break;
            case 3:
                test_wide_string_operations();
                break;
            case 4:
                test_array_operations();
                break;
            case 5:
                test_arithmetic_operations();
                break;
            case 6:
                test_file_operations();
                break;
            case 7:
                test_memory_operations();
                test_string_operations();
                test_wide_string_operations();
                test_array_operations();
                test_arithmetic_operations();
                test_file_operations();
                break;
            case 0:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }

        if (choice != 0) {
            pause_console();
        }

    } while (choice != 0);

    return 0;
}

void test_memory_operations(void) {
    printf("Testing Memory Operations\n");
    printf("========================\n");

    // Test SafeMalloc
    printf("Testing SafeMalloc...\n");
    void* ptr = SafeMalloc(100);
    if (ptr) {
        printf("SUCCESS: Memory allocated successfully\n");

        // Test SafeFree
        printf("Testing SafeFree...\n");
        SafeFree(&ptr);
        if (ptr == NULL) {
            printf("SUCCESS: Memory freed and pointer nulled\n");
        } else {
            printf("FAIL: Pointer not nulled after free\n");
        }
    } else {
        printf("FAIL: Memory allocation failed\n");
    }

    // Test SafeFreeTyped
    printf("\nTesting SafeFreeTyped...\n");
    int* numbers = SafeMalloc(sizeof(int) * 10);
    if (numbers) {
        if (SafeFreeTyped((void**)&numbers, sizeof(int) * 10)) {
            printf("SUCCESS: Typed memory freed successfully\n");
        } else {
            printf("FAIL: Typed memory free failed\n");
        }
    }
    printf("\n");
}

void test_string_operations(void) {
    printf("Testing String Operations\n");
    printf("========================\n");

    char dest[50] = {0};
    const char* src = "Hello, World!";

    // Test SafeStrCopy
    printf("Testing SafeStrCopy...\n");
    if (SafeStrCopy(dest, sizeof(dest), src)) {
        printf("SUCCESS: String copied: '%s'\n", dest);
    } else {
        printf("FAIL: String copy failed\n");
    }

    // Test SafeStrCat
    printf("\nTesting SafeStrCat...\n");
    if (SafeStrCat(dest, sizeof(dest), " How are you?")) {
        printf("SUCCESS: String concatenated: '%s'\n", dest);
    } else {
        printf("FAIL: String concatenation failed\n");
    }

    // Test SafeStrFind
    printf("\nTesting SafeStrFind...\n");
    size_t pos;
    if (SafeStrFind(dest, strlen(dest), "World", &pos)) {
        printf("SUCCESS: Found 'World' at position: %zu\n", pos);
    } else {
        printf("FAIL: String find failed\n");
    }

    // Test SafeStrReplace
    printf("\nTesting SafeStrReplace...\n");
    size_t newLen;
    if (SafeStrReplace(dest, sizeof(dest), "World", "Everyone", &newLen)) {
        printf("SUCCESS: String replaced: '%s'\n", dest);
    } else {
        printf("FAIL: String replacement failed\n");
    }
    printf("\n");
}

void test_wide_string_operations(void) {
    printf("Testing Wide String Operations\n");
    printf("============================\n");

    wchar_t wdest[50] = {0};
    const wchar_t* wsrc = L"Hello, Wide World!";

    // Test SafeWStrNCopy
    printf("Testing SafeWStrNCopy...\n");
    if (SafeWStrNCopy(wdest, 50, wsrc, wcslen(wsrc))) {
        printf("SUCCESS: Wide string copied\n");
    } else {
        printf("FAIL: Wide string copy failed\n");
    }

    // Test SafeWStrNCat
    printf("\nTesting SafeWStrNCat...\n");
    const wchar_t* wappend = L" How are you?";
    if (SafeWStrNCat(wdest, 50, wappend, wcslen(wappend))) {
        printf("SUCCESS: Wide string concatenated\n");
    } else {
        printf("FAIL: Wide string concatenation failed\n");
    }
    printf("\n");
}

void test_array_operations(void) {
    printf("Testing Array Operations\n");
    printf("=======================\n");

    int array[10] = {0};
    size_t arraySize = sizeof(array) / sizeof(array[0]);

    // Test SafeWriteInt
    printf("Testing SafeWriteInt...\n");
    if (SafeWriteInt(array, arraySize, 5, 42)) {
        printf("SUCCESS: Wrote value 42 at index 5\n");
    } else {
        printf("FAIL: Array write failed\n");
    }

    // Test SafeReadInt
    printf("\nTesting SafeReadInt...\n");
    int value;
    if (SafeReadInt(array, arraySize, 5, &value)) {
        printf("SUCCESS: Read value %d from index 5\n", value);
    } else {
        printf("FAIL: Array read failed\n");
    }

    // Test out-of-bounds
    printf("\nTesting out-of-bounds access...\n");
    if (!SafeWriteInt(array, arraySize, 10, 100)) {
        printf("SUCCESS: Out-of-bounds write prevented\n");
    } else {
        printf("FAIL: Out-of-bounds write not caught\n");
    }
    printf("\n");
}

void test_arithmetic_operations(void) {
    printf("Testing Arithmetic Operations\n");
    printf("===========================\n");

    int result;

    // Test SafeAddInt
    printf("Testing SafeAddInt...\n");
    if (SafeAddInt(5, 3, &result)) {
        printf("SUCCESS: 5 + 3 = %d\n", result);
    } else {
        printf("FAIL: Addition failed\n");
    }

    // Test overflow
    printf("\nTesting overflow...\n");
    if (!SafeAddInt(INT_MAX, 1, &result)) {
        printf("SUCCESS: Overflow detected\n");
    } else {
        printf("FAIL: Overflow not caught\n");
    }

    // Test SafeCastLongLongToInt
    printf("\nTesting SafeCastLongLongToInt...\n");
    long long bigNum = 42LL;
    if (SafeCastLongLongToInt(bigNum, &result)) {
        printf("SUCCESS: Cast %lld to %d\n", bigNum, result);
    } else {
        printf("FAIL: Cast failed\n");
    }
    printf("\n");
}

void test_file_operations(void) {
    printf("Testing File Operations\n");
    printf("=====================\n");

    SafeFileOpts opts = {
            .followSymlinks = false,
            .requireRegularFile = true,
            .createMode = 0644,
            .secureDelete = false
    };

    // Test file writing
    printf("Testing file writing...\n");
    FILE* file = SafeFOpen("test.txt", "w", &opts);
    if (file) {
        fprintf(file, "Test content\n");
        fclose(file);
        printf("SUCCESS: File written successfully\n");

        // Test file reading
        printf("\nTesting file reading...\n");
        file = SafeFOpen("test.txt", "r", &opts);
        if (file) {
            char buffer[100];
            if (fgets(buffer, sizeof(buffer), file)) {
                printf("SUCCESS: Read content: '%s'\n", buffer);
            } else {
                printf("FAIL: Could not read file content\n");
            }
            fclose(file);
        } else {
            printf("FAIL: Could not open file for reading\n");
        }
    } else {
        printf("FAIL: Could not open file for writing\n");
    }
    printf("\n");
}

void pause_console(void) {
    printf("\nPress Enter to continue...");
    while (getchar() != '\n'); // Clear any remaining characters
    getchar(); // Wait for Enter
}