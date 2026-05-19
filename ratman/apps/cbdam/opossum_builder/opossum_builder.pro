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

TEMPLATE=app
CONFIG -= staticlib

QT +=  opengl qt3support 
CONFIG+=  cbdam_opossum cbdam_opossum_buildings vic_cbdam_base qt
win32:CONFIG+= console

HEADERS=
SOURCES= opossum_builder.cpp

TARGET=vic_opossum_builder
target.path=$$BIN_DIR
INSTALLS+= target

INCLUDEPATH += ../../src/vic

#--------------------------------------------------------------------
include (../../../ratman_post.pri)
#--------------------------------------------------------------------
