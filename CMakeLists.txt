cmake_minimum_required(VERSION 3.12)

# Set default compiler before project call
set(CMAKE_CXX_COMPILER "g++")

# Set the project name
project(Spider
        VERSION 0.0.0
        LANGUAGES CXX)

# Generate a compile db that can be used by LSPs.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#include(FetchContent)
#FetchContent_Declare(
#        googletest
#        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
#)
## For Windows: Prevent overriding the parent project's compiler/linker settings
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
#FetchContent_MakeAvailable(googletest)

set(BOOST_ROOT /opt/boost)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost COMPONENTS system)
include_directories(${Boost_INCLUDE_DIR})

# Create the `spider` target
add_executable(spider src/main.cc)

# ERROR HANDLING
if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "No build type selected, default to Release")
    set(CMAKE_BUILD_TYPE "Release")
endif ()

add_library(compile_options INTERFACE)

target_compile_options(compile_options INTERFACE
        "-Wall"
        "-Wextra"
        "-fno-strict-aliasing"
        "-Werror"
        "-DBOOST_LOG_DYN_LINK"

        $<$<CONFIG:Release>:-Werror>
        $<$<CONFIG:Release>:-O3>

        $<$<CONFIG:Debug>:-g>
        $<$<CONFIG:Debug>:-fno-inline>
        $<$<CONFIG:Debug>:-fsanitize=address>
        )

target_link_libraries(compile_options INTERFACE
        $<$<CONFIG:Debug>:-fsanitize=address>
        )

set_target_properties(spider PROPERTIES
        CXX_STANDARD 20
        CXX_EXTENSIONS OFF
        )

include_directories(spider PUBLIC src/misc lib/ src/ src/config src/boost src/request src/utils)

# SUBDIRECTORY Cmake will be called recursively in those directory


enable_testing()
#include(GoogleTest)

add_subdirectory(src/misc)
add_subdirectory(src/config)
add_subdirectory(src/boost)
add_subdirectory(src/request)
add_subdirectory(src/vhost)
add_subdirectory(src/utils)

target_link_libraries(
        spider
        PRIVATE
        compile_options
        misc
        config
        boost
        request
        vhost
        utils
        ${Boost_LIBRARIES}
        -lboost_log
)


