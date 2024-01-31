cmake_minimum_required(VERSION 3.20)
project(gain_capital_api VERSION 1.1.0 DESCRIPTION "Unofficial C++ API for Gain Capital's Forex.com.")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

add_library(${PROJECT_NAME} SHARED
                src/gain_capital_api.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES 
                        VERSION ${PROJECT_VERSION}
                        SOVERSION ${PROJECT_VERSION_MAJOR}
                        PUBLIC_HEADER include/${PROJECT_NAME}.h)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

include(FetchContent)
FetchContent_Declare(cpr GIT_REPOSITORY https://github.com/libcpr/cpr.git
                         GIT_TAG 3020c34ae2b732121f37433e61599c34535e68a8) # The commit hash for 1.10.x. Replace with the latest from: https://github.com/libcpr/cpr/releases
FetchContent_MakeAvailable(cpr)

target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)

find_package(Boost COMPONENTS program_options log log_setup REQUIRED)

include_directories( ${Boost_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC ${Boost_LIBRARIES})

# Install library
install(TARGETS ${PROJECT_NAME}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

        
# Build Example Executable
add_executable(Example examples/example.cpp
                src/gain_capital_api.cpp)

target_include_directories(Example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(Example PRIVATE cpr::cpr)

target_link_libraries(Example LINK_PUBLIC ${Boost_LIBRARIES})