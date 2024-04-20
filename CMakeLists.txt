cmake_minimum_required(VERSION 3.20)
project(
  gain_capital_api
  VERSION 2.0.2
  DESCRIPTION "Unofficial C++ API for Gain Capital's Forex.com.")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE "Debug")

include(GNUInstallDirs)

add_library(${PROJECT_NAME} SHARED src/gain_capital_client.cpp src/gain_capital_exception.cpp)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES VERSION ${PROJECT_VERSION}
             SOVERSION ${PROJECT_VERSION_MAJOR}
             PUBLIC_HEADER include/${PROJECT_NAME}.h)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

# ------------------------------
# Build Configuration
include(cmake/CompilerWarnings.cmake)
include(cmake/Sanitizers.cmake)
include(cmake/StaticAnalyzers.cmake)
include(cmake/Tests.cmake)
include(cmake/Hardening.cmake)
include(cmake/PreventInSourceBuilds.cmake)
include(cmake/ClangFormat.cmake)

myproject_assure_out_of_source_builds()
myproject_set_project_warnings(
  ${PROJECT_NAME}
  TRUE
  "X"
  ""
  ""
  "X")
myproject_enable_hardening(${PROJECT_NAME} TRUE FALSE)
add_clang_format_target(RUN_CLANG-FORMAT ${CMAKE_CURRENT_SOURCE_DIR})

add_compile_options(-pipe -fPIC)
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  add_compile_options(-NDDEBUG)
endif()
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
  if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    myproject_enable_clang_tidy(${PROJECT_NAME} FALSE)
  endif()
  #myproject_enable_include_what_you_use()
  #myproject_enable_cppcheck(FALSE "X")
  gcc_analyzer_enable()
  myproject_enable_coverage(${PROJECT_NAME})
  myproject_enable_sanitizers(
    ${PROJECT_NAME}
    TRUE
    TRUE
    TRUE
    FALSE
    TRUE)
endif()

# ------------------------------
# Libraries
include(FetchContent)
FetchContent_Declare(
  cpr
  GIT_REPOSITORY https://github.com/libcpr/cpr.git
  GIT_TAG 3020c34ae2b732121f37433e61599c34535e68a8)
# The commit hash for 1.10.x. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)

# ------------------------------
# Install library
install(
  TARGETS ${PROJECT_NAME}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# ------------------------------
# Testing
enable_testing()
add_subdirectory(test)

# ===================================================================
# Build Example Executable
# ===================================================================
add_executable(Example examples/example.cpp src/gain_capital_client.cpp src/gain_capital_exception.cpp)

target_include_directories(Example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Example PRIVATE cpr::cpr)
