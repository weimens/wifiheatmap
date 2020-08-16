
set(DELAUNATOR_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/foreign/delaunator-cpp/include")
message(${DELAUNATOR_INCLUDE_DIRS})
add_library(Delaunator::Delaunator INTERFACE IMPORTED)
set_target_properties(Delaunator::Delaunator
  PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${DELAUNATOR_INCLUDE_DIRS}"
)
