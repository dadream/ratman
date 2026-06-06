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
vic_vfs {
  isEmpty(RATMAN_BUILD_DIR) {
    RATMAN_BUILD_DIR = $$RATMAN_DIR
  }
  win32 {
    PRE_TARGETDEPS += $$RATMAN_BUILD_DIR/src/vic/vfs/$$BUILD_SUBDIR/vic_vfs$${TARGET_SUFFIX}.lib
    LIBS += $$RATMAN_BUILD_DIR/src/vic/vfs/$$BUILD_SUBDIR/vic_vfs$${TARGET_SUFFIX}.lib
  }
  unix {
    PRE_TARGETDEPS += $$RATMAN_BUILD_DIR/src/vic/vfs/$$BUILD_SUBDIR/libvic_vfs$${TARGET_SUFFIX}.a
    LIBS += $$RATMAN_BUILD_DIR/src/vic/vfs/$$BUILD_SUBDIR/libvic_vfs$${TARGET_SUFFIX}.a
  }
    CONFIG += xvic_curl xvic_zlib xvic_db4

  # check if berkeley db4 is installed
  DB4_CHECK_DIR=$(DB4_DIR)
  !exists($$DB4_CHECK_DIR/include/db.h) {
     DB4_CHECK_DIR=/usr/
     !exists($$DB4_CHECK_DIR/include/db.h) {
       DB4_CHECK_DIR=/usr/local/
       !exists($$DB4_CHECK_DIR/include/db.h) {
          CONFIG -= xvic_db4
	  message("VFS configured without DB4")
       }
     }
  } 

  INCLUDEPATH += 
}
