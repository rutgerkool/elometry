cmake_minimum_required(VERSION 3.10)

project(FootballELOValuator VERSION 1.0)

set(CMAKE_CXX_STANDARD_17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(FootballELOValuator
    src/main.cpp
    src/CSVReader.cpp
)

find_package(Boost COMPONENTS program_options REQUIRED)
target_link_libraries(FootballELOValuator Boost::program_options)
