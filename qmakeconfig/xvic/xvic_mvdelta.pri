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
xvic_mvdelta{
  unix {
    MVDELTA_DIR=$(MVDELTA_DIR)
    !exists($$MVDELTA_DIR/include/mv.h) {
       MVDELTA_DIR=/usr/
       !exists($$MVDELTA_DIR/include/mv.h) {
         MVDELTA_DIR=/usr/local/
         !exists($$MVDELTA_DIR/include/mv.h) {
            MVDELTA_DIR=/u/vvr/packages/
            !exists($$MVDELTA_DIR/include/mv.h) {
               error("Cannot find MVDELTA_DIR/include/mv.h -- please set MVDELTA_DIR env var")
            }
         }
       }
    }
    
    MVDELTA_LIB_DIR=$(MVDELTA_LIB_DIR)
    !exists($$MVDELTA_LIB_DIR/libslx.*) {
      MVDELTA_LIB_DIR=$$MVDELTA_DIR/lib64
      !exists($$MVDELTA_LIB_DIR/libslx.*) {
        MVDELTA_LIB_DIR=$$MVDELTA_DIR/lib
        !exists($$MVDELTA_LIB_DIR/libslx.*) {
           error("Cannot find MVDELTA_LIB_DIR/libslx.* -- please set MVDELTA_LIB_DIR env var")
        }
      }
    }
    LIBS *= -L$$MVDELTA_LIB_DIR -lslx -lmvos -lmvx11 -lXt -lXext -lX11
  }

  INCLUDEPATH *= $$MVDELTA_DIR/include 

  message("Configured for xvic_mvdelta")
}
