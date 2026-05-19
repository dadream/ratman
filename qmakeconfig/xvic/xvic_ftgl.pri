#+++HDR+++
#======================================================================
#   This file is part of the RATMAN software framework.
#   Copyright (C) 2009 by CRS4, Pula, Italy.
#
#   For more information, visit the CRS4 Visual Computing Group
#   web pages at http://www.crs4.it/vic/
#
#   This file may be used under the terms of the GNU General Public
#   License as published by the Free Software Foundation and appearing
#   in the file LICENSE included in the packaging of this file.
#
#   CRS4 reserves all rights not expressly granted herein.
#  
#   This file is provided AS IS with NO WARRANTY OF ANY KIND, 
#   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS 
#   FOR A PARTICULAR PURPOSE.
#
#======================================================================
#---HDR---#
xvic_ftgl {
  win32 {
    FTGL_DIR = $(PROGRAMFILES)/ftgl
    LIBS += $$FTGL_DIR/lib/libftgl.lib
  }
 unix {
    FTGL_DIR=$$(FTGL_DIR)
    !exists($$FTGL_DIR/include/freetype2/freetype/freetype.h) {
       FTGL_DIR=/usr/
       !exists($$FTGL_DIR/include/freetype2/freetype/freetype.h) {
         FTGL_DIR=/usr/local/
         !exists($$FTGL_DIR/include/freetype2/freetype/freetype.h) {
            FTGL_DIR=/u/vvr/packages/
            !exists($$FTGL_DIR/include/freetype2/freetype/freetype.h) {
               error("Cannot find FTGL_DIR/include/freetype.h -- please set FTGL_DIR env var")
            }
         }
       }
    }

    FTGL_LIB_DIR=$$(FTGL_LIB_DIR)
    !exists($$FTGL_LIB_DIR/libftgl.*) {
      FTGL_LIB_DIR=$$FTGL_DIR/lib64
      !exists($$FTGL_LIB_DIR/libftgl.*) {
        FTGL_LIB_DIR=$$FTGL_DIR/lib
        !exists($$FTGL_LIB_DIR/libftgl.*) {
          error("Cannot find FTGL_LIB_DIR/libftgl.* -- please set FTGL_LIB_DIR env var")
        }
      }
    }

    LIBS += -L$$FTGL_LIB_DIR -lftgl
  } 

  message("Configured for ftgl")
}
