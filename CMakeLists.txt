cmake_minimum_required(VERSION 3.10)

project(Elometry VERSION 1.0)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(WARNING_FLAGS "-Wall -Wextra -Wpedantic -Werror=return-type")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")
    
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG}")
elseif(MSVC)
    set(WARNING_FLAGS "/W4")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS}")
endif()

include_directories(include)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/static)

file(GLOB STATIC_FILES ${CMAKE_SOURCE_DIR}/static/*)
foreach(FILE ${STATIC_FILES})
    configure_file(${FILE} ${CMAKE_BINARY_DIR}/static/ COPYONLY)
endforeach()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

file(GLOB_RECURSE SRC_FILES src/*.cpp)

find_package(unofficial-sqlite3 CONFIG QUIET)

if(unofficial-sqlite3_FOUND)
    set(SQLITE3_TARGET unofficial::sqlite3::sqlite3)
else()
    find_package(SQLite3 REQUIRED)
    set(SQLITE3_TARGET SQLite::SQLite3)
endif()

find_package(CURL REQUIRED)
find_package(OpenMP REQUIRED)
find_package(GLPK REQUIRED)
find_package(Qt6 CONFIG REQUIRED COMPONENTS Widgets Network Charts Concurrent)

file(GLOB_RECURSE UI_HEADERS include/gui/*.h)

qt6_wrap_cpp(MOC_SOURCES ${UI_HEADERS})

qt6_add_resources(RESOURCES src/gui/resources/resources.qrc)

add_executable(Elometry ${SRC_FILES} ${MOC_SOURCES} ${RESOURCES})

target_compile_definitions(Elometry PRIVATE QT_CHARTS_LIB)

target_include_directories(Elometry PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CURL_INCLUDE_DIRS}
)

set_property(TARGET Elometry PROPERTY AUTOMOC ON)

if(WIN32)
    add_compile_definitions(_CRT_SECURE_NO_WARNINGS)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif(APPLE)
    set(CMAKE_MACOSX_RPATH ON)
endif()

target_link_libraries(Elometry 
    PRIVATE 
    GLPK::GLPK 
    ${SQLITE3_TARGET} 
    OpenMP::OpenMP_CXX 
    Qt6::Widgets 
    Qt6::Network
    Qt6::Charts
    Qt6::Concurrent
    ${CURL_LIBRARIES}
)

file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/gui/components)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/src/gui/components)
