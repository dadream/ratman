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
include (../../../ratman_pre.pri)
#--------------------------------------------------------------------

# check if berkeley db4 is installed
CONFIG += use_berkeley_db
DB4_CHECK_DIR=$(DB4_DIR)
!exists($$DB4_CHECK_DIR/include/db.h) {
   DB4_CHECK_DIR=/usr/
   !exists($$DB4_CHECK_DIR/include/db.h) {
     DB4_CHECK_DIR=/usr/local/
     !exists($$DB4_CHECK_DIR/include/db.h) {
        CONFIG -= use_berkeley_db
     }
   }
} 

use_berkeley_db {
   message("Berkeley db installed: use db_repository")
   CONFIG += xvic_db4
#   HEADERS = db_repository.hpp
   SOURCES = db_repository.cpp	repository_using_db.cpp
} else {
   message("Berkeley db NOT installed: use sl_repository")
#   HEADERS = sl_repository.hpp
   SOURCES = sl_repository.cpp	repository_using_sl.cpp
}
 

CONFIG += qt thread xvic_zlib xvic_sl xvic_curl
TEMPLATE= lib
DEPENDPATH*= $$INCLUDEPATH

HEADERS+= \
virtual_file_system.hpp         virtual_file_system_local.hpp    \
virtual_file_system_network.hpp repository.hpp                  \

SOURCES+= \
virtual_file_system_local.cpp	virtual_file_system_network.cpp	\

TARGET=vic_vfs

# --- INSTALL
install_pri.path=$$SHARE_DIR/vic/qmake
install_pri.files=vic_vfs.pri

install_inc.path=$$INCLUDE_DIR/vic/vfs
install_inc.files=$$HEADERS
target.path=$$LIB_DIR

INSTALLS+= install_pri install_inc target

#--------------------------------------------------------------------
include (../../../ratman_post.pri)
#--------------------------------------------------------------------
