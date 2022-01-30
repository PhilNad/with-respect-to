set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package (Eigen3 3.4 REQUIRED)
add_subdirectory(SQLiteCpp-3.1.1)

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