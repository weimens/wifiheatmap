project(wifiheatmap-tests LANGUAGES CXX)

enable_testing()
find_package(Qt5 COMPONENTS Gui Widgets Test REQUIRED)



add_executable(tst_measurements
    tst_measurements.cpp
    ../measurements.h
    ../measurements.cpp
)
add_test(tst_measurements tst_measurements)
target_include_directories(tst_measurements PRIVATE ..)
set_target_properties(tst_measurements
  PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    AUTOMOC ON
)
target_link_libraries(tst_measurements
    Qt5::Test
    Qt5::Core
)



add_executable(tst_bssmodel
    tst_bssmodel.cpp
    ../measurements.h
    ../measurements.cpp
    ../bssmodel.h
    ../bssmodel.cpp
)
add_test(tst_bssmodel tst_bssmodel)
target_include_directories(tst_bssmodel PRIVATE ..)
set_target_properties(tst_bssmodel
  PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    AUTOMOC ON
)
target_link_libraries(tst_bssmodel
    Qt5::Test
    Qt5::Core
    Qt5::Gui
)



add_executable(tst_measurementmodel
    tst_measurementmodel.cpp
    ../measurements.h
    ../measurements.cpp
    ../measurementmodel.h
    ../measurementmodel.cpp
    ../commands.h
    ../commands.cpp
)
add_test(tst_measurementmodel tst_measurementmodel)
target_include_directories(tst_measurementmodel PRIVATE ..)
set_target_properties(tst_measurementmodel
  PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    AUTOMOC ON
)
target_link_libraries(tst_measurementmodel
    Qt5::Test
    Qt5::Core
    Qt5::Widgets
)



add_executable(tst_document
    tst_document.cpp
    ../measurements.h
    ../measurements.cpp
    ../document.h
    ../document.cpp
)
add_test(tst_document tst_document)
target_include_directories(tst_document PRIVATE ..)
set_target_properties(tst_document
  PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    AUTOMOC ON
)
target_link_libraries(tst_document
    Qt5::Test
    Qt5::Core
    Qt5::Gui
    QuaZip5::QuaZip5
    Qt5::Widgets
)



add_executable(tst_heatmap
    tst_heatmap.cpp
    ../measurements.h
    ../measurements.cpp
    ../document.h
    ../document.cpp
    ../heatmap.h
    ../heatmap.cpp
    ../qml.qrc
)
add_test(tst_heatmap tst_heatmap)
target_include_directories(tst_heatmap PRIVATE ..)
set_target_properties(tst_heatmap
  PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
    AUTOMOC ON
    AUTORCC ON
)
target_link_libraries(tst_heatmap
    Qt5::Test
    Qt5::Core
    Qt5::Gui
    Qt5::Quick
    QuaZip5::QuaZip5
    Delaunator::Delaunator
    Qt5::Widgets
)
