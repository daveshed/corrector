add_executable(run-unittests test.cpp main.cpp)
message(STATUS "${CMAKE_SOURCE_DIR}/src")
target_include_directories(
    run-unittests PUBLIC
    ${CMAKE_SOURCE_DIR}/src
)
target_link_libraries(
    run-unittests
    spellchecklib
    libCppUTest.a
    libCppUTestExt.a
)
