cmake_minimum_required(VERSION 3.10)

project(FootballELOValuator VERSION 1.0)

set(CMAKE_CXX_STANDARD_20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(FootballELOValuator ${SRC_FILES})

find_package(SQLite3 REQUIRED)
find_package(OpenMP REQUIRED)
find_package(GLPK REQUIRED)

if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

target_link_libraries(FootballELOValuator GLPK::GLPK SQLite::SQLite3 OpenMP::OpenMP_CXX)
