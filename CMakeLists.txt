cmake_minimum_required(VERSION 3.10)

project(FootballELOValuator VERSION 1.0)

set(CMAKE_CXX_STANDARD_20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(FootballELOValuator
    src/main.cpp
    src/models/PlayerRating.cpp
    src/utils/DataLoader.cpp
)

find_package(Boost COMPONENTS program_options REQUIRED)
target_link_libraries(FootballELOValuator Boost::program_options)
