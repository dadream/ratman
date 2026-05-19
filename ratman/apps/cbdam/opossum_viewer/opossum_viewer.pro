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
include(../../../ratman_pre.pri)
#--------------------------------------------------------------------

TEMPLATE 	= app
CONFIG -= staticlib

#The following line was inserted by qt3to4
QT +=  opengl qt3support 
CONFIG+= cbdam_opossum cbdam_opossum_buildings vic_cbdam_base vic_img opengl qt 
win32:CONFIG+= console

FORMS=opossum_window_gui.ui

HEADERS 	= 		glutil.h               \
qgl_window_base.hpp		qgl_window_opossum.hpp   \
opossum_window.hpp

SOURCES 	= 		glutil.c               \		
qgl_window_base.cpp		qgl_window_opossum.cpp \
opossum_window.cpp		opossum_viewer.cpp

TARGET		= vic_opossum_viewer
target.path=$$BIN_DIR
INSTALLS+= target

#unix:LIBS += -lCg -lCgGL
#win32:LIBS += $(CG_LIB_PATH)\cgGL.lib $(CG_LIB_PATH)\cg.lib 

INCLUDEPATH += ../../src/vic

#--------------------------------------------------------------------
include(../../../ratman_post.pri)
#--------------------------------------------------------------------
