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
######################################
## Default config

isEmpty( PREFIX ) {
  PREFIX=$$(PREFIX)
}
isEmpty( PREFIX ) {
  PREFIX=$$(CYG_DIR)/usr/local
}
exists(/wksp/output/share/vic/qmakeconfig/default/default_pre.pri) {
    include(/wksp/output/share/vic/qmakeconfig/default/default_pre.pri)
} else {
    include($${PREFIX}/share/vic/qmakeconfig/default/default_pre.pri)
}

######################################
## Local config

CONFIG += release

!exists($$BASE_DIR/base_pre.pri) {
  BASE_DIR = $$PWD
  !exists($$BASE_DIR/base_pre.pri) {
  BASE_DIR = .
  !exists($$BASE_DIR/base_pre.pri) {
    BASE_DIR = ..
    !exists($$BASE_DIR/base_pre.pri) {
       BASE_DIR = ../..
       !exists($$BASE_DIR/base_pre.pri) {
          BASE_DIR = ../../..
          !exists($$BASE_DIR/base_pre.pri) {
                error("Cannot find base_pre.pri - please set BASE_DIR env. var")
          }
        }
      }
    }
  }
}

