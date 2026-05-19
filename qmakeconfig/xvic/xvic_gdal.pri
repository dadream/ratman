xvic_gdal {
  unix {
    GDAL_DIR = /usr
    GDAL_LIB_DIR = /usr/lib/x86_64-linux-gnu
    INCLUDEPATH += /usr/include/gdal
    LIBS += -L$$GDAL_LIB_DIR -lgdal
  }
  win32 {
    GDAL_DIR = $(GDAL_DIR)
    GDAL_LIB_DIR=$(GDAL_LIB_DIR)
    LIBS += $$GDAL_LIB_DIR/gdal.lib
  }
  message("Configured for gdal")
}