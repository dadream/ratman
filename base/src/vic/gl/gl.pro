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
include (../../../base_pre.pri)
#--------------------------------------------------------------------

TEMPLATE= lib
#TEMPLATE= app

CONFIG += opengl
QT += opengl

HEADERS= gl.hpp font.hpp arial16-csrc.c
SOURCES= font.cpp 

#SOURCES += glfont_test.cpp

TARGET= vic_gl

# --- INSTALL
install_pri.path=$$SHARE_DIR/vic/qmake
install_pri.files=vic_gl.pri

install_inc.path=$$INCLUDE_DIR/vic/gl
install_inc.files=$$HEADERS
target.path=$$LIB_DIR

INSTALLS+= install_pri install_inc target

#--------------------------------------------------------------------
include (../../../base_post.pri)
#--------------------------------------------------------------------

