vic_qxml {
  win32 {
    PRE_TARGETDEPS += $$BASE_DIR/src/vic/qxml/$$BUILD_SUBDIR/vic_qxml$${TARGET_SUFFIX}.lib
    LIBS += $$BASE_DIR/src/vic/qxml/$$BUILD_SUBDIR/vic_qxml$${TARGET_SUFFIX}.lib
  }
  unix {
    PRE_TARGETDEPS += $$BASE_DIR/src/vic/qxml/$$BUILD_SUBDIR/libvic_qxml$${TARGET_SUFFIX}.a
    LIBS += $$BASE_DIR/src/vic/qxml/$$BUILD_SUBDIR/libvic_qxml$${TARGET_SUFFIX}.a
  }
  QT += xml
  INCLUDEPATH += 
}
