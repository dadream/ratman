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
#############################
## Default config

CONFIG += staticlib warn_on sse sl thread 
CONFIG -= debug release

unix:CONFIG += system-png system-zlib
win32:CONFIG += cygwin exceptions

exists(/usr/lib64) {
  CONFIG += lib64
} else {
  CONFIG += lib32
}

#--------------------------------------------------------------------
# INCLUDE_DIR and LIB_DIR specify where to install the include files and the library.
# Use qmake INCLUDE_DIR=... LIB_DIR=... , or qmake PREFIX=... to customize your installation.

isEmpty( PREFIX ) {
  PREFIX = $$(PREFIX)
}
isEmpty( PREFIX ) {
  PREFIX=$$(CYG_DIR)/usr/local
}
isEmpty( LIB_DIR ) {
  lib64 {
    LIB_DIR = $${PREFIX}/lib64
  } else {
    LIB_DIR = $${PREFIX}/lib
  }  
}
isEmpty( BIN_DIR ) {
  BIN_DIR = $${PREFIX}/bin
}
isEmpty( INCLUDE_DIR ) {
  INCLUDE_DIR = $${PREFIX}/include
}

isEmpty( SHARE_DIR ) {
  SHARE_DIR = $${PREFIX}/share
}

isEmpty( DOC_DIR ) {
  DOC_DIR = $${SHARE_DIR}/doc
}

###### functions #########

defineTest(vic_find_include) {
  files = $$ARGS
  for(file, files) {
    message(searching $$file)
    !exists($$INCLUDE_DIR/$$file) {
      return(false)
    }
  }
  return(true)
}

defineTest(vic_check_include) {
  !vic_find_include($$ARGS) {
     error("Cannot find include $$ARGS in $$INCLUDE_DIR")
     return(false)
  }
  return(true)
}

defineTest(vic_find_lib) {
  files = $$ARGS
  for(file, files) {
    !exists($$LIB_DIR/$$file) {
      return(false)
    }
  }
  return(true)
}

defineTest(vic_check_lib) {
  !vic_find_lib($$ARGS) {
     error("Cannot find include $$ARGS in $$LIB_DIR")
     return(false)
  }
  return(true)
}



