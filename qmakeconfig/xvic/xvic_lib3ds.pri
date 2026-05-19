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
xvic_lib3ds {
  win32 {
    LIB3DS_DIR = $(PROGRAMFILES)/lib3ds/
    LIBS += $$LIB3DS_DIR/lib/3ds.lib
  }
  unix {
    LIB3DS_DIR=$$(LIB3DS_DIR)
    !exists($$LIB3DS_DIR/include/lib3ds/file.h) {
       LIB3DS_DIR=/usr/
       !exists($$LIB3DS_DIR/include/lib3ds/file.h) {
         LIB3DS_DIR=/usr/local/
         !exists($$LIB3DS_DIR/include/lib3ds/file.h) {
            LIB3DS_DIR=/u/vvr/packages/
            !exists($$LIB3DS_DIR/include/lib3ds/file.h) {
               error("Cannot find LIB3DS_DIR/include/lib3ds/file.h -- please set LIB3DS_DIR env var")
            }
         }
       }
    }

    LIB3DS_LIB_DIR=$$(LIB3DS_LIB_DIR)
    !exists($$LIB3DS_LIB_DIR/lib3ds.*) {
      LIB3DS_LIB_DIR=$$LIB3DS_DIR/lib64
      !exists($$LIB3DS_LIB_DIR/lib3ds.*) {
        LIB3DS_LIB_DIR=$$LIB3DS_DIR/lib
        !exists($$LIB3DS_LIB_DIR/lib3ds.*) {
          error("Cannot find LIB3DS_LIB_DIR/lib3ds.* -- please set LIB3DS_LIB_DIR env var")
        }
      }
    }

    LIBS += -L$$LIB3DS_LIB_DIR -l3ds
  } 

  message("Configured for xvic_lib3ds")
}
