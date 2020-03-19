cmake_minimum_required(VERSION 3.5)

project(wifiheatmap_trigger_scan LANGUAGES CXX)

find_package(Qt5 COMPONENTS Core)

add_executable(${PROJECT_NAME}
  main.cpp
  trigger_scan.h
)

set_target_properties(wifiheatmap_trigger_scan
  PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    AUTOMOC ON
)
target_link_libraries(${PROJECT_NAME} Qt5::Core)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
