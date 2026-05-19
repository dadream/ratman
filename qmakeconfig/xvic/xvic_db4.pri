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
xvic_db4 {
  unix {
    DB4_DIR = /usr
    DB4_LIB_DIR = /usr/lib/x86_64-linux-gnu
    LIBS += -L$$DB4_LIB_DIR -ldb -ldb_cxx
    !equals(DB4_DIR, "/usr") {
    }
  }
  win32 {
    DB4_DIR = $(DB4_DIR)
    DB4_LIB_DIR = $(DB4_LIB_DIR)
    LIBS += -L$$DB4_LIB_DIR -llibdb45 -llibdb_cxx
  }
  message("Configured for db4")
}
