find_package (Eigen3 3.4 REQUIRED)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

SET(PROJECT_NAME "WRT-tests")
project(${PROJECT_NAME})
add_executable(${PROJECT_NAME} src/test.cpp)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/../lib/src/)

target_link_libraries(${PROJECT_NAME}
    wrt-lib
    Eigen3::Eigen
    SQLiteCpp
    sqlite3
    pthread
    dl
)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
