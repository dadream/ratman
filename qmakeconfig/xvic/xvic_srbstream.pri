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
xvic_srbstream {
  win32 {
    SRBSTREAM_DIR = $(SRBSTREAM_DIR)
    !exists($$SRBSTREAM_DIR/include/srbstream/srbstreamconf.h) {
       error("Cannot find SRBSTREAM_DIR/include/srbstream/srbstreamconf.h -- please set SRBSTREAM_DIR env var")
    }
    SRBSTREAM_LIB_DIR=$$SRBSTREAM_DIR
    !exists($$SRBSTREAM_LIB_DIR/srbstream2.*) {
       error("Cannot find SRBSTREAM_LIB_DIR/srbstream2.* -- please set SRBSTREAM_LIB_DIR env var") 	
    }	
    LIBS += -L$$SRBSTREAM_LIB_DIR -lsrbstream2
  }
  unix {
    SRBSTREAM_DIR=$(SRBSTREAM_DIR)
    !exists($$SRBSTREAM_DIR/include/srbstream/srbstream.hpp) {
       SRBSTREAM_DIR=/usr/
       !exists($$SRBSTREAM_DIR/include/srbstream/srbstream.hpp) {
          SRBSTREAM_DIR=/usr/local/
          !exists($$SRBSTREAM_DIR/include/srbstream/srbstream.hpp) {
             SRBSTREAM_DIR=/u/vvr/packages/
             !exists($$SRBSTREAM_DIR/include/srbstream/srbstream.hpp) {
               error("Cannot find SRBSTREAM_DIR/include/srbstream/srbstream.hpp -- please set SRBSTREAM_DIR env var")
             }
          }
       }
    }    
    SRBSTREAM_LIB_DIR=$(SRBSTREAM_LIB_DIR)
    !exists($$SRBSTREAM_LIB_DIR/libsrbstream.*) {
      SRBSTREAM_LIB_DIR=$$SRBSTREAM_DIR/lib64
      !exists($$SRBSTREAM_LIB_DIR/libsrbstream.*) {
        SRBSTREAM_LIB_DIR=$$SRBSTREAM_DIR/lib
        !exists($$SRBSTREAM_LIB_DIR/libsrbstream.*) {
          error("Cannot find SRBSTREAM_LIB_DIR/libsrbstream.* -- please set SRBSTREAM_LIB_DIR env var")
        }
      }
    }

    LIBS += -L$$SRBSTREAM_LIB_DIR -lsrbstream -lSrbClient
   
  }

  DEFINES += _LARGEFILE_SOURCE _FILE_OFFSET_BITS=64 _LARGEFILE64_SOURCE PORTNAME_linux PARA_OPR=1 FED_MCAT MCAT_VERSION_20


  message("Configured for xvic_srbstream")
}
