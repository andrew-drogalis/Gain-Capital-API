cmake_minimum_required(VERSION 3.20)
project(Functional_Test
        VERSION 0.0.1
        DESCRIPTION ""
        LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Test_bin functional_test.cpp
                src/gain_capital_api.cpp
                src/lightstreamer_handler.cpp
                src/keychain/keychain_linux.cpp
                src/lightstreamer/base_client.cpp
                src/lightstreamer/default_handler.cpp
                src/lightstreamer/ssl_client.cpp
                src/lightstreamer/tcp_client.cpp
                src/lightstreamer/url_encode.cpp)

target_include_directories(Test_bin PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GLIB2 IMPORTED_TARGET glib-2.0)
pkg_check_modules(LIBSECRET IMPORTED_TARGET libsecret-1)

target_link_libraries(Test_bin
    PRIVATE
        PkgConfig::GLIB2
        PkgConfig::LIBSECRET)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 3020c34ae2b732121f37433e61599c34535e68a8) # The commit hash for 1.10.x. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

target_link_libraries(Test_bin PRIVATE cpr::cpr)

find_package(Boost COMPONENTS program_options log log_setup REQUIRED)

include_directories( ${Boost_INCLUDE_DIRS})

target_link_libraries(Test_bin LINK_PUBLIC ${Boost_LIBRARIES})


