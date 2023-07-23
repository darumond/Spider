add_library(
  misc STATIC
  fd.cc
  json.hh
)

set_target_properties(misc PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF)

target_link_libraries(
  misc
  PRIVATE compile_options
          readiness
)

add_subdirectory(readiness)
