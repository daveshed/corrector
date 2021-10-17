# Project specifics
cmake_minimum_required (VERSION 3.10)
project (Spellchecker)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_BUILD_TYPE debug)  # compile with debug symbols

# Build spellchecker library in the src dir. This top-level CMakeFile calls
# the subdirectory CMakeFile...
add_subdirectory(src)
# build the tests...
add_subdirectory(test)
# run tests...
enable_testing()
add_test(NAME unittest COMMAND run-unittests)
