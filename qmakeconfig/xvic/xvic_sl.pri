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
xvic_sl {
  SL_DIR = $(SL_DIR)
  !exists($$SL_DIR/include/sl/any.hpp) {
     SL_DIR = /usr
     !exists($$SL_DIR/include/sl/any.hpp) {
        SL_DIR = $$(PREFIX)/
        !exists($$SL_DIR/include/sl/any.hpp) {
          error("Cannot find SL_DIR/include/sl/any.hpp -- please set SL_DIR env var")
        }
     }
  }  
  $SL_LIB_DIR=$(SL_LIB_DIR)
  !exists($$SL_LIB_DIR/libsl.a) {
     SL_LIB_DIR = $$SL_DIR/lib64
     !exists($$SL_LIB_DIR/libsl.a) {      
       SL_LIB_DIR = $$SL_DIR/lib32
       !exists($$SL_LIB_DIR/libsl.a) {      
          SL_LIB_DIR = $$SL_DIR/lib
          !exists($$SL_LIB_DIR/libsl.a) {      
             error("Cannot find SL_LIB_DIR/libsl.a -- please set SL_LIB_DIR env var")
          }
       }
     }
  }
 
  LIBS+= $$SL_LIB_DIR/libsl.a
  
  DEFINES += _ISOC9X_SOURCE=1 _ISOC99_SOURCE=1 __USE_ISOC9X=1 __USE_ISOC99=1
 
  message("Configured for xvic_sl")
}
