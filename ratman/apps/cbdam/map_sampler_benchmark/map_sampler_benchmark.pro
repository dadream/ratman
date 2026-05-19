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

CONFIG += vic_cbdam_geo vic_cbdam_base 
CONFIG -= staticlib

CONFIG += qt opengl
QT += opengl

win32:CONFIG+= console

HEADERS=
SOURCES= map_sampler_benchmark.cpp

LIBS += -lglut

TARGET=vic_map_sampler_benchmark

target.path=$$BIN_DIR
INSTALLS+= target

#--------------------------------------------------------------------
include (../../../ratman_post.pri)
#--------------------------------------------------------------------
