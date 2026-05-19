xvic_glew {
  unix {
    GLEW_DIR = /usr
    GLEW_LIB_DIR = /usr/lib/x86_64-linux-gnu
    LIBS += -L$$GLEW_LIB_DIR -lGLEW
    !equals(GLEW_DIR, "/usr") {
    }
  }
  win32 {
    GLEW_DIR = $(GLEW_DIR)
    GLEW_LIB_DIR = $(GLEW_LIB_DIR)
    LIBS += $$GLEW_LIB_DIR/glew32.lib
  }
  message("Configured for glew")
}