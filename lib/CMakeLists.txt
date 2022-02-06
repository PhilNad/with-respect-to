cmake_minimum_required(VERSION 3.0.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")

find_package (Eigen3 3.4 REQUIRED)
find_package(SQLiteCpp REQUIRED)

SET(PROJECT_NAME "wrt-lib")
project(${PROJECT_NAME})

file(GLOB SRC_LIB_FILES src/*.cpp)
add_library(${PROJECT_NAME} ${SRC_LIB_FILES})

target_link_libraries(${PROJECT_NAME}
    Eigen3::Eigen
    SQLiteCpp
    sqlite3
    pthread
    dl
)