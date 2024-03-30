# ==============================================================
# Unit Tests
# ==============================================================

cmake_minimum_required(VERSION 3.20)

project (google_tests)

include(GNUInstallDirs)

find_path(
    MHD_INCLUDE_DIR
    NAMES microhttpd.h
    DOC "microhttpd include dir"
)

find_library(
    MHD_LIBRARY
    NAMES microhttpd microhttpd-10 libmicrohttpd libmicrohttpd-dll
    DOC "microhttpd library"
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(mhd DEFAULT_MSG
    MHD_INCLUDE_DIR MHD_LIBRARY)

mark_as_advanced(MHD_INCLUDE_DIR MHD_LIBRARY)

set(MHD_INCLUDE_DIRS ${MHD_INCLUDE_DIR})
set(MHD_LIBRARIES ${MHD_LIBRARY})

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${HTTPMOCKSERVER_INCLUDE_DIRS}
    ${CPR_INCLUDE_DIRS}
    ${MHD_INCLUDE_DIRS}
    ${GTEST_INCLUDE_DIRS}
)

find_package(GTest)

enable_testing()

cmake_path(GET CMAKE_CURRENT_SOURCE_DIR PARENT_PATH PARENT_DIR)

# Add a testing executable
add_executable(unit_tests 
    unit_tests.cpp
    functional_tests.cpp
    ${PARENT_DIR}/src/gain_capital_api.cpp
    ${PARENT_DIR}/src/httpmockserver/mock_server.cc)

target_include_directories(unit_tests PRIVATE ${PARENT_DIR}/include)

target_link_libraries(unit_tests PRIVATE cpr::cpr)

target_link_libraries(unit_tests LINK_PUBLIC ${Boost_LIBRARIES} GTest::GTest GTest::Main ${HTTPMOCKSERVER_LIBRARIES} ${MHD_LIBRARIES})

# we cannot analyse results without gcov
find_program(GCOV_PATH gcov)
if(NOT GCOV_PATH)
  message(FATAL_ERROR "Code coverage analysis requires gcov!")
endif()

add_test(test_all unit_tests)

