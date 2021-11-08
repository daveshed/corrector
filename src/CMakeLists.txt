# Build a library that we can use to make the executable here and also build
# against in tests.

## Library
add_library(spellchecklib spellchecker.cpp)

## Executable
add_executable(spellchecker main.cpp)
target_link_libraries(spellchecker spellchecklib)