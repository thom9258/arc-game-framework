cmake_minimum_required(VERSION 3.1)
project(ArcCore VERSION 0.1.0)

# Example used for this configuration:
# https://surfertas.github.io/cmake/cpp/projecteuler/2019/05/01/cmake.html

if (NOT CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    message(STATUS "[${PROJECT_NAME}] has a top-level project called [${CMAKE_PROJECT_NAME}]")
else()
    message(STATUS "[${PROJECT_NAME}] This project is top-level")
endif()


set(CMAKE_CXX_FLAGS "-Wall -Wextra -ggdb")
set(CMAKE_CXX_STANDARD 17)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/inc)

add_library(${PROJECT_NAME} SHARED 
                            src/Logger.cpp
                            src/SceneManager.cpp
)
