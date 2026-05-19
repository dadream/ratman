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
  vic_check_include(vic/vfs/virtual_file_system.hpp) {}
  vic_check_lib(*vic_vfs.*) {}

  LIBS += -lvic_vfs
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

  xvic_db4 {
    message("Configured for vic_vfs using db_repository")
  } else {
    message("Configured for vic_vfs using sl_repository")
  }
}
