cmake_minimum_required(VERSION 3.1)
project(ArcCore VERSION 0.1.0)

# Compilation stuff
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -ggdb -std=c++17")

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -----------------------------------------
# CREATION OF A INCLUDEABLE SHARED LIBRARY
# https://www.pragmaticlinux.com/2022/02/create-a-shared-library-in-c-with-cmake/
# -----------------------------------------

# -----------------------------------------
# Link to all the files of the library
# -----------------------------------------
#   set(LIB_HEADERS
#   "{CMAKE_CURRENT_SOURCE_DIR}inc/Defs.hpp"
#   "{CMAKE_CURRENT_SOURCE_DIR}inc/Logger.hpp"
#   #"inc/SceneManager.hpp"
#   )

# -----------------------------------------
# Links to source files
# -----------------------------------------
set(LIB_SOURCES
  "${CMAKE_CURRENT_SOURCE_DIR}/src/Logger.cpp"
  #"src/SceneManager.cpp"
)

# -----------------------------------------
# Setup Library definition
# -----------------------------------------
add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})
