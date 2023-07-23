add_library(vhost STATIC
  vhost.cc
  vhost.hh
  vhost.hxx
        proxy_pass.cc proxy_pass.hh upstream.cc upstream.hh)

set_target_properties(vhost PROPERTIES
  CXX_STANDARD 20
  CXX_EXTENSIONS OFF
  )

target_link_libraries(
  vhost
  compile_options
  ssl
  crypto
)