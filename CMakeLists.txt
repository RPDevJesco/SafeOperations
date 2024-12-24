cmake_minimum_required(VERSION 3.30)
project(SafeOperations
        VERSION 1.0.0
        DESCRIPTION "Safe operations library in C99"
        LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Set output directories
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Define source files
set(LIB_SOURCES
        src/SafeOps.c
)

set(LIB_HEADERS
        include/SafeOps.h
)

# Create static library
add_library(SafeOperations_static STATIC ${LIB_SOURCES} ${LIB_HEADERS})
set_target_properties(SafeOperations_static PROPERTIES
        OUTPUT_NAME "SafeOperations"
        PREFIX "lib"
)

# Create shared library
add_library(SafeOperations_shared SHARED ${LIB_SOURCES} ${LIB_HEADERS})
set_target_properties(SafeOperations_shared PROPERTIES
        OUTPUT_NAME "SafeOperations"
        PREFIX "lib"
        VERSION ${PROJECT_VERSION}
        SOVERSION ${PROJECT_VERSION_MAJOR}
)

# Set include directories for both targets
target_include_directories(SafeOperations_static
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_include_directories(SafeOperations_shared
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

# Create test executable
add_executable(SafeOperationsTest tests/test_SafeOps.c)
target_link_libraries(SafeOperationsTest PRIVATE SafeOperations_static)
target_include_directories(SafeOperationsTest PRIVATE include)

# Installation rules remain the same...
include(GNUInstallDirs)
install(TARGETS SafeOperations_static SafeOperations_shared
        EXPORT SafeOperationsTargets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES ${HEADERS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/SafeOperations
)