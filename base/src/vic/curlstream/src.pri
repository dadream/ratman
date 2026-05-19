vic_curlstream {
  win32 {
    PRE_TARGETDEPS += $$BASE_DIR/src/vic/curlstream/$$BUILD_SUBDIR/vic_curlstream$${TARGET_SUFFIX}.lib
    LIBS += $$BASE_DIR/src/vic/curlstream/$$BUILD_SUBDIR/vic_curlstream$${TARGET_SUFFIX}.lib
  }
  unix {
    PRE_TARGETDEPS += $$BASE_DIR/src/vic/curlstream/$$BUILD_SUBDIR/libvic_curlstream$${TARGET_SUFFIX}.a
    LIBS += $$BASE_DIR/src/vic/curlstream/$$BUILD_SUBDIR/libvic_curlstream$${TARGET_SUFFIX}.a 
   }
  CONFIG += xvic_curl
  INCLUDEPATH += 
}