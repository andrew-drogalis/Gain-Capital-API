cmake_minimum_required(VERSION 3.20)

# Set the project name
project (google_tests)

# Add an library for the example classes
add_library(google_tests
    Reverse.cpp
    Palindrome.cpp
)


#############################################
# Unit tests

find_package(include(GoogleTest))

# enable CTest testing
enable_testing()

# Add a testing executable
add_executable(unit_tests unit_tests.cpp)

target_link_libraries(unit_tests
    google_tests
    GTest::GTest 
    GTest::Main
)

add_test(test_all unit_tests)