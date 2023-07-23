add_library(request STATIC
    io_operation.cc
    middleware.cc
    request.cc
    request.hh
    request.hxx
    response.cc
    response.hh
)

set_target_properties(config PROPERTIES
    CXX_STANDARD 20
    CXX_EXTENSIONS OFF
)

#add_executable(
#    response_test
#    response_test.cc
#    response.cc
#)
#
#add_executable(
#    request_test
#    request_test.cc
#    request.cc
#)
#
#target_link_libraries(
#    response_test
#    boost
#    compile_options
#    GTest::gtest_main
#    request
#)

#target_link_libraries(
#    request_test
#    config
#    GTest::gtest_main
#    request
#    utils
#    vhost
#)

target_link_libraries(
    request
    misc
)

#gtest_discover_tests(response_test)
#gtest_discover_tests(request_test)