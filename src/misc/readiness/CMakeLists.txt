add_library(
  readiness
  STATIC readiness.cc
)

set_target_properties(readiness PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF)

target_link_libraries(readiness PRIVATE compile_options)
