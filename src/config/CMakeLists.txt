add_library(config STATIC
    config.cc
    config.hh
    config.hxx
)

set_target_properties(config PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
)

target_link_libraries(
    compile_options
)

