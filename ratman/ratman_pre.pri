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

include(/wksp/share/vic/qmakeconfig/default/default_pre.pri)

######################################
## Local config

CONFIG += release

unix:CONFIG += system-png system-zlib
win32:CONFIG += cygwin exceptions

####
#ITEM3D Flag

#CONFIG += item3d


#############################
## RATMAN_DIR selection

!exists($$RATMAN_DIR/ratman_pre.pri) {
  RATMAN_DIR = $$PWD
  !exists($$RATMAN_DIR/ratman_pre.pri) {
  RATMAN_DIR = .
  !exists($$RATMAN_DIR/ratman_pre.pri) {
    RATMAN_DIR = ..
    !exists($$RATMAN_DIR/ratman_pre.pri) {
       RATMAN_DIR = ../..
       !exists($$RATMAN_DIR/ratman_pre.pri) {
       	  RATMAN_DIR = ../../..
       	  !exists($$RATMAN_DIR/ratman_pre.pri) {
          	error("Cannot find ratman_pre.pri - please set RATMAN_DIR env. var")
       	  }
        }
      }	
    }
  }
}

