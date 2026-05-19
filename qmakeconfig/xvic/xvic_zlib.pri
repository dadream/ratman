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
xvic_zlib {
  unix {
    ZLIB_DIR = /usr
    ZLIB_LIB_DIR = /usr/lib/x86_64-linux-gnu
    LIBS += -L$$ZLIB_LIB_DIR -lz
    !equals(ZLIB_DIR, "/usr") {
    }
  }
  win32 {
    ZLIB_DIR = $(ZLIB_DIR)
    ZLIB_LIB_DIR = $(ZLIB_LIB_DIR)
    LIBS+= $$ZLIB_LIB_DIR/lib/zlib.lib
  }
  message("Configured for zlib")
}
