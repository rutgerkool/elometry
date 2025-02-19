cmake_minimum_required(VERSION 3.10)

project(FootballELOValuator VERSION 1.0)

set(CMAKE_CXX_STANDARD_20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

add_executable(FootballELOValuator ${SRC_FILES})

find_package(SQLite3 REQUIRED)
find_package(Boost COMPONENTS program_options REQUIRED)
find_package(OpenMP REQUIRED)

target_link_libraries(FootballELOValuator Boost::program_options SQLite::SQLite3 OpenMP::OpenMP_CXX)
