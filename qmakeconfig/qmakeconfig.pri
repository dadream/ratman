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
  LIB_DIR = $${PREFIX}/lib
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
message(prefix $$PREFIX)

QMAKE_CFLAGS_ISYSTEM =
QMAKE_CXXFLAGS_ISYSTEM =
