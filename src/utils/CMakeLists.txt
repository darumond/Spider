add_library(utils STATIC
    types.hh
    utils.cc
    utils.hh
)

set_target_properties(utils PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
)

target_link_libraries(
    utils
    compile_options
)