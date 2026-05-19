xvic_shp {
  unix {
    SHP_DIR = /usr
    SHP_LIB_DIR = /usr/lib/x86_64-linux-gnu
    INCLUDEPATH += /wksp/include
    LIBS += -L$$SHP_LIB_DIR -lshp
    # Some systems use shapefil.h, others libshp/shapefil.h
    exists(/usr/include/shapefil.h) {
    } else {
    }
  }
  win32 {
    SHP_DIR = $(SHP_DIR)
    SHP_LIB_DIR = $(SHP_LIB_DIR)
    LIBS += $$SHP_LIB_DIR/libshp/shapelib.lib
  }
  message("Configured for shp")
}