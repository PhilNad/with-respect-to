find_package (Eigen3 3.4 REQUIRED)

SET(PROJECT_NAME "WRT")
project(${PROJECT_NAME})
add_executable(${PROJECT_NAME} src/main.cpp)

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
