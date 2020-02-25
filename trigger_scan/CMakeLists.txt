cmake_minimum_required(VERSION 3.5)

project(wifiheatmap_trigger_scan LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5Core)

add_executable(${PROJECT_NAME}
  main.cpp
  trigger_scan.h
)
target_link_libraries(${PROJECT_NAME} Qt5::Core)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
