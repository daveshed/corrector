# Build a library that we can use to make the executable here and also build
# against in tests.
#           lib           target
add_library(spellchecklib spellchecker.cpp)
#              executable   targets
add_executable(spellchecker spellchecker.cpp main.cpp)