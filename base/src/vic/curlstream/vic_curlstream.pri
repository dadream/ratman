vic_curlstream {
  vic_check_include(vic/curlstream/curlstream.hpp) {}
  vic_check_lib(*vic_curlstream.*) {}

  LIBS += -lvic_curlstream
  CONFIG += xvic_curl 
  message(Configured for vic_curlstream)
}
