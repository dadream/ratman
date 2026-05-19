xvic_lzo {
  unix {
    LZO_DIR = /usr
    LZO_LIB_DIR = /usr/lib/x86_64-linux-gnu
    LIBS += -L$$LZO_LIB_DIR -llzo2
  }
  win32 {
    LZO_DIR = $(LZO_DIR)
    LZO_LIB_DIR = $(LZO_LIB_DIR)
    LIBS += $$LZO_LIB_DIR/lzo2.lib
  }
  message("Configured for lzo")
}