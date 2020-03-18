cmake_minimum_required(VERSION 3.5)

project(wifiheatmap LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(CGAL)
if(ANDROID)
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    set(QUAZIP_LIB_VERSION_SUFFIX 5)
    find_package(QuaZip5 REQUIRED CONFIG)
    set(ANDROID_EXTRA_LIBS ${QUAZIP_LIBRARIES})
endif()

if(NOT ANDROID)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(NL3 REQUIRED libnl-3.0 libnl-genl-3.0)
    set(QUAZIP_LIB_VERSION_SUFFIX 5) #WORKAROUND: QuaZip5Find only works if Qt5 is already found.
    find_package(QuaZip5 REQUIRED)
endif()

find_package(Qt5 5.13 COMPONENTS Core Quick Widgets REQUIRED)
if(ANDROID)
    find_package(Qt5 COMPONENTS AndroidExtras REQUIRED)
endif()

set(HEADERS
    measurementmodel.h
    heatmap.h
    bssmodel.h
    document.h
    qmlsortfilterproxymodel.h
    imageprovider.h
    measurements.h
    )
set(SOURCES
    main.cpp
    measurementmodel.cpp
    heatmap.cpp
    bssmodel.cpp
    document.cpp
    qmlsortfilterproxymodel.cpp
    imageprovider.cpp
    measurements.cpp
    )
set(RESOURCES qml.qrc)

if(NOT ANDROID)
    list(APPEND HEADERS
        linuxscan.h
        netlinkwrapper.h
        interfacemodel.h
    )
    list(APPEND SOURCES
        linuxscan.cpp
        netlinkwrapper.cpp
        interfacemodel.cpp
    )
endif()

if(ANDROID)
    list(APPEND HEADERS
        androidscan.h
        androidhelper.h
    )
    list(APPEND SOURCES
        androidscan.cpp
        androidhelper.cpp
    )
endif()

if(ANDROID)
    add_library(${PROJECT_NAME} SHARED
        ${SOURCES}
        ${HEADERS}
        ${RESOURCES}
    )
else()
    add_executable(${PROJECT_NAME}
        ${SOURCES}
        ${HEADERS}
        ${RESOURCES}
    )
endif()

target_include_directories(${PROJECT_NAME} PRIVATE ${QUAZIP_INCLUDE_DIRS})
if(NOT ANDROID)
    target_include_directories(${PROJECT_NAME} PRIVATE ${NL3_INCLUDE_DIRS})
endif()

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME}
  PRIVATE Qt5::Core Qt5::Quick Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE CGAL::CGAL)
target_link_libraries(${PROJECT_NAME} PRIVATE ${QUAZIP_LIBRARIES})
if(ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::AndroidExtras)
endif()
if(NOT ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${NL3_LIBRARIES})
    add_subdirectory(trigger_scan)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
