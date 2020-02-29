cmake_minimum_required(VERSION 3.5)

project(wifiheatmap LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
#    if (ANDROID_ABI STREQUAL "armeabi-v7a")
#        set(ANDROID_EXTRA_LIBS
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
#            ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
#    endif()
#endif()

find_package(Qt5 5.13 COMPONENTS Core Quick Widgets REQUIRED)
find_package(CGAL)
find_package(PkgConfig REQUIRED)
pkg_check_modules(NL3 REQUIRED libnl-3.0 libnl-genl-3.0)

set(HEADERS
    measurementmodel.h
    heatmap.h
    netlinkwrapper.h
    interfacemodel.h
    bssmodel.h
    document.h
    trigger_scan.h
    qmlsortfilterproxymodel.h
    imageprovider.h
    )
set(SOURCES
    main.cpp
    measurementmodel.cpp
    heatmap.cpp
    netlinkwrapper.cpp
    interfacemodel.cpp
    bssmodel.cpp
    document.cpp
    trigger_scan.cpp
    qmlsortfilterproxymodel.cpp
    imageprovider.cpp
    )
set(RESOURCES qml.qrc)

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

target_include_directories(${PROJECT_NAME} PRIVATE ${NL3_INCLUDE_DIRS})

target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME}
  PRIVATE Qt5::Core Qt5::Quick Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} PRIVATE CGAL::CGAL)
target_link_libraries(${PROJECT_NAME} PRIVATE ${NL3_LIBRARIES})

add_subdirectory(trigger_scan)

install(TARGETS ${PROJECT_NAME} DESTINATION bin)
