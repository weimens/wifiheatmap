cmake_minimum_required(VERSION 3.5)

project(wifiheatmap LANGUAGES CXX)

find_package(CGAL)
if(ANDROID)
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    set(QUAZIP_LIB_VERSION_SUFFIX 5)
    find_package(QuaZip5 REQUIRED CONFIG)
    set(ANDROID_EXTRA_LIBS ${QUAZIP_LIBRARIES})
endif()

if(NOT ANDROID)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GENL3 REQUIRED libnl-3.0 libnl-genl-3.0)
    add_library(nl3::genl INTERFACE IMPORTED)
    set_target_properties(nl3::genl
      PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${GENL3_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${GENL3_LIBRARIES}"
    )

    set(QUAZIP_LIB_VERSION_SUFFIX 5) #WORKAROUND: QuaZip5Find only works if Qt5 is already found.
    find_package(QuaZip5 REQUIRED)
endif()

if (NOT TARGET QuaZip5::QuaZip5)
  add_library(QuaZip5::QuaZip5 INTERFACE IMPORTED)
  set_target_properties(QuaZip5::QuaZip5
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${QUAZIP_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES "${QUAZIP_LIBRARIES}"
  )
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

set_target_properties(${PROJECT_NAME}
  PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    AUTOUIC ON
    AUTOMOC ON
    AUTORCC ON
)
target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Qt5::Core
    Qt5::Quick
    Qt5::Widgets
    CGAL::CGAL
    QuaZip5::QuaZip5
)
if(ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::AndroidExtras)
endif()
if(NOT ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE nl3::genl)
    add_subdirectory(trigger_scan)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()
