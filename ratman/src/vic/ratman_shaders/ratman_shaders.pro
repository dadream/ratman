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

TEMPLATE= lib
TARGET=vic_ratman_shaders

unix {
QMAKE_EXTRA_UNIX_TARGETS += lightwave 
}
win32 {
QMAKE_EXTRA_WIN32_TARGETS += lightwave 
}
CG2CPP=vic_cg2cpp

lightwave.target = lightwave.hpp
lightwave.commands =$$CG2CPP  -v -profile best -entry lightwave_main -classname lightwave lightwave.cg -o lightwave.hpp
lightwave.depends = lightwave.cg
QMAKE_CLEAN += $$lightwave.target
PRE_TARGETDEPS += $$lightwave.target


# --- INSTALL

install_inc.path=$$INCLUDE_DIR/vic/ratman_shaders/
install_inc.files=$${lightwave.target} 
install_inc.CONFIG += no_check_exist


INSTALLS+= install_inc

#--------------------------------------------------------------------
include (../../../ratman_post.pri)
#--------------------------------------------------------------------
