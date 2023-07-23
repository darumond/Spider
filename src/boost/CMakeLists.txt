add_library(boost STATIC
  dispatcher.cc
  server.cc
  session.cc
  reverse_proxy.cc)

set_target_properties(boost PROPERTIES
  CXX_STANDARD 20
  CXX_EXTENSIONS OFF
  )

target_link_libraries(
  boost
  config
  compile_options
  request
  utils
  vhost

  ssl
  crypto
)