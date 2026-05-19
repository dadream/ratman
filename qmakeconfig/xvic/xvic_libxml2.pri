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
xvic_libxml2 {
  win32 {
    LIBXML2_DIR = $(LIBXML2_DIR)
    !exist($$LIBXML2_DIR/include/libxml/tree.h) {
	error("Cannot find LIBXML2_DIR/include/libxml/tree.h -- please set LIBXML2_DIR env var")
    }
    LIBXML2_LIB_DIR=$(LIBXML2_LIB_DIR)
    !exists($$LIBXML2_LIB_DIR/libxml2.*) {
       LIBXML2_LIB_DIR=$$LIBXML2_DIR/lib
        !exists($$LIBXML2_LIB_DIR/libxml2.*) {
          error("Cannot find LIBXML2_LIB_DIR/libxml2.* -- please set LIBXML2_LIB_DIR env var")
       }
     }
    LIBS+= $$LIBXML2_LIB_DIR/libxml2.lib	    
  }

  unix {
    LIBXML2_DIR=$(LIBXML2_DIR)
    !exists($$LIBXML2_DIR/include/libxml2/libxml/tree.h) {
       LIBXML2_DIR=/usr/
       !exists($$LIBXML2_DIR/include/libxml2/libxml/tree.h) {
         LIBXML2_DIR=/usr/local/
         !exists($$LIBXML2_DIR/include/libxml2/libxml/tree.h) {
            LIBXML2_DIR=/u/vvr/packages/
            !exists($$LIBXML2_DIR/include/libxml2/libxml/tree.h) {
               error("Cannot find LIBXML2_DIR/include/libxml2/libxml/tree.h -- please set LIBXML2_DIR env var")
            }
         }
       }
    }

    LIBXML2_LIB_DIR=$(LIBXML2_LIB_DIR)
    !exists($$LIBXML2_LIB_DIR/libxml2.*) {
      LIBXML2_LIB_DIR=$$LIBXML2_DIR/lib64
      !exists($$LIBXML2_LIB_DIR/libxml2.*) {
        LIBXML2_LIB_DIR=$$LIBXML2_DIR/lib
        !exists($$LIBXML2_LIB_DIR/libxml2.*) {
          error("Cannot find LIBXML2_LIB_DIR/libxml2.* -- please set LIBXML2_LIB_DIR env var")
        }
      }
    }

    LIBS += -L$$LIBXML2_LIB_DIR -lxml2
  } 

  message("Configured for libxml2")
}
