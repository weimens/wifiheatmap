cmake_minimum_required(VERSION 3.12)

project(wifiheatmap LANGUAGES CXX)

list (APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)
list (APPEND CMAKE_MODULE_PATH /usr/share/cmake) #WORKAROUND: for fedora 32 QuaZip5Find


find_package(Delaunator REQUIRED)
if(ANDROID)
    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
else()
    set(QUAZIP_LIB_VERSION_SUFFIX 5) #WORKAROUND: QuaZip5Find only works if Qt5 is already found.
    find_package(QuaZip5 REQUIRED)
endif()

if(UNIX AND NOT ANDROID)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GENL3 REQUIRED libnl-3.0 libnl-genl-3.0)
    add_library(nl3::genl INTERFACE IMPORTED)
    set_target_properties(nl3::genl
      PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${GENL3_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${GENL3_LIBRARIES}"
    )
    find_library(IPERF iperf)
endif()

if (NOT TARGET QuaZip5::QuaZip5)
  add_library(QuaZip5::QuaZip5 INTERFACE IMPORTED)
  set_target_properties(QuaZip5::QuaZip5
    PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${QUAZIP_INCLUDE_DIRS}"
      INTERFACE_LINK_LIBRARIES "${QUAZIP_LIBRARIES}"
  )
endif()

find_package(Qt5 5.12 COMPONENTS Core Quick Widgets REQUIRED)
if(ANDROID)
    find_package(Qt5 COMPONENTS AndroidExtras REQUIRED)
endif()


if(ANDROID)
    add_library(${PROJECT_NAME} SHARED)
else()
    add_executable(${PROJECT_NAME})
endif()

target_sources(${PROJECT_NAME} PRIVATE
    main.cpp
    measurementmodel.cpp
    heatmap.cpp
    bssmodel.cpp
    document.cpp
    qmlsortfilterproxymodel.cpp
    imageprovider.cpp
    measurements.cpp
    helper.cpp
    commands.cpp
    measurementtypemodel.cpp
    measurementcontroller.cpp
    statusqueue.cpp

    measurementmodel.h
    heatmap.h
    bssmodel.h
    document.h
    qmlsortfilterproxymodel.h
    imageprovider.h
    measurements.h
    helper.h
    commands.h
    measurementtypemodel.h
    entries/bss.h
    entries/measurement.h
    entries/measurement_entry.h
    entries/measurement_type.h
    entries/position.h
    measurementcontroller.h
    statusqueue.h

    qml.qrc
)

if(ANDROID)
    target_sources(${PROJECT_NAME} PRIVATE
        androidscan.cpp
        androidhelper.cpp
        androidscan.h
        androidhelper.h
    )
endif()
if(UNIX AND NOT ANDROID)
    target_sources(${PROJECT_NAME} PRIVATE
        linuxscan.cpp
        netlinkwrapper.cpp
        interfacemodel.cpp
        iperfwrapper.cpp
        iperf.cpp
        linuxscan.h
        netlinkwrapper.h
        interfacemodel.h
        iperfwrapper.h
        iperf.h
    )
endif()
if(WIN32)
    target_sources(${PROJECT_NAME} PRIVATE
        windowsscan.cpp
        wlanapiwrapper.cpp
        windowsscantrigger.cpp
        windowsinterfacemodel.cpp
        windowsscan.h
        wlanapiwrapper.h
        windowsscantrigger.h
        windowsinterfacemodel.h
    )
endif()

set_target_properties(${PROJECT_NAME}
  PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    AUTOUIC ON
    AUTOMOC ON
    AUTORCC ON
)
target_compile_definitions(${PROJECT_NAME}
  PRIVATE $<$<OR:$<CONFIG:Debug>,$<CONFIG:RelWithDebInfo>>:QT_QML_DEBUG>)
if(UNIX AND NOT ANDROID)
    if(TRIGGER_SCAN_BIN)
        target_compile_definitions(${PROJECT_NAME}
            PRIVATE TRIGGER_SCAN_BIN="${TRIGGER_SCAN_BIN}"
        )
    else()
        target_compile_definitions(${PROJECT_NAME}
            PRIVATE TRIGGER_SCAN_BIN="$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/trigger_scan/wifiheatmap_trigger_scan>$<INSTALL_INTERFACE:${CMAKE_INSTALL_PREFIX}/bin/wifiheatmap_trigger_scan>"
        )
    endif()
endif()

target_link_libraries(${PROJECT_NAME}
  PRIVATE
    Qt5::Core
    Qt5::Quick
    Qt5::Widgets
    Delaunator::Delaunator
)
if(ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::AndroidExtras)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE QuaZip5::QuaZip5)
endif()
if(UNIX AND NOT ANDROID)
    target_link_libraries(${PROJECT_NAME} PRIVATE nl3::genl)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${IPERF})
    add_subdirectory(trigger_scan)
    add_subdirectory(installer/XDGData)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)
endif()

if(NOT ANDROID)
    add_subdirectory(tests)
endif()
