#############################################
# Unit tests

cmake_minimum_required(VERSION 3.20)

project (google_tests)

include(GNUInstallDirs)

# Add an library for the example classes
# add_library(google_tests
#     Reverse.cpp
#     Palindrome.cpp
# )

# include_directories(
#     ${HTTPMOCKSERVER_INCLUDE_DIRS}
#     ${MHD_INCLUDE_DIRS}
# )

# add_library(httpmockserver STATIC mock_server.cc)

# install(TARGETS httpmockserver ARCHIVE DESTINATION lib)



include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${HTTPMOCKSERVER_INCLUDE_DIRS}
    ${CPR_INCLUDE_DIRS}
    ${MHD_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
)

find_package(GTest)

# enable CTest testing
enable_testing()

# Add a testing executable
add_executable(unit_tests 
    unit_tests.cpp)

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)

target_include_directories(unit_tests PRIVATE ${PARENT_DIR}/include)

target_link_libraries(unit_tests
    GTest::GTest 
    GTest::Main
)

# target_link_libraries(test-server
#     ${HTTPMOCKSERVER_LIBRARIES}
#     ${MHD_LIBRARIES}
#     ${CPR_LIBRARIES}
#     ${GTEST_LIBRARIES}
#     -lpthread
# )


add_test(test_all unit_tests)

# Find microhttpd. From nanopool/genoil-ethereum under MIT license
#
# Find the microhttpd includes and library
#
# if you need to add a custom library search path, do it via via CMAKE_PREFIX_PATH
#
# This module defines
#  MHD_INCLUDE_DIRS, where to find header, etc.
#  MHD_LIBRARIES, the libraries needed to use jsoncpp.
# #  MHD_FOUND, If false, do not try to use jsoncpp.

# find_path(
#     MHD_INCLUDE_DIR
#     NAMES microhttpd.h
#     DOC "microhttpd include dir"
# )

# find_library(
#     MHD_LIBRARY
#     NAMES microhttpd microhttpd-10 libmicrohttpd libmicrohttpd-dll
#     DOC "microhttpd library"
# )

# include(FindPackageHandleStandardArgs)
# find_package_handle_standard_args(mhd DEFAULT_MSG
#     MHD_INCLUDE_DIR MHD_LIBRARY)

# mark_as_advanced(MHD_INCLUDE_DIR MHD_LIBRARY)

# set(MHD_INCLUDE_DIRS ${MHD_INCLUDE_DIR})
# set(MHD_LIBRARIES ${MHD_LIBRARY})



