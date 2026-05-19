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
xvic_gle {
  win32 {
    GLE_DIR = $(GLE_DIR)
    !exists($$GLE_DIR/include/gle/gle.h) {
       error("Cannot find GLE_DIR/include/gle/gleconf.h -- please set GLE_DIR env var")
    }
    GLE_LIB_DIR=$$GLE_DIR
    !exists($$GLE_LIB_DIR/gle.*) {
       error("Cannot find GLE_LIB_DIR/gle2.* -- please set GLE_LIB_DIR env var") 	
    }	
    LIBS += -L$$GLE_LIB_DIR -lgle
  }
  unix {
    GLE_DIR=$(GLE_DIR)
    !exists($$GLE_DIR/include/GL/gle.h) {
      GLE_DIR=/usr
      !exists($$GLE_DIR/include/GL/gle.h) {
        GLE_DIR=/usr/local
        !exists($$GLE_DIR/include/GL/gle.h) {
          GLE_DIR=/u/vvr/packages
          !exists($$GLE_DIR/include/GL/gle.h) {
            error("Cannot find GLE_DIR/include/GL/gle.h -- please set GLE_DIR env var")
          }
        }
      }
    }
    
    GLE_LIB_DIR=$(GLE_LIB_DIR)
    !exists($$GLE_LIB_DIR/libgle.*) {
      GLE_LIB_DIR=$$GLE_DIR/lib64
      !exists($$GLE_LIB_DIR/libgle.*) {
        GLE_LIB_DIR=$$GLE_DIR/lib
        !exists($$GLE_LIB_DIR/libgle.*) {
          error("Cannot find GLE_LIB_DIR/libgle.* -- please set GLE_LIB_DIR env var")
        }
      }
    }

    LIBS += -L$$GLE_LIB_DIR -lgle
  
  } 

  message("Configured for xvic_gle")
}
