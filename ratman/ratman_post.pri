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
## Default post config

include($$SHARE_DIR/vic/qmakeconfig/default/default_post.pri)

# remove win32 warning C4244 possible loss of data
win32:QMAKE_CXXFLAGS_RELEASE -= /Wp64
#QMAKE_CXXFLAGS_RELEASE += -Wmissing-field-initializers -Wuninitialized

#debug mode
unix:QMAKE_CXXFLAGS_RELEASE -= -O
unix:QMAKE_CXXFLAGS_RELEASE += -O3
#unix:QMAKE_CXXFLAGS_RELEASE += -O0 -g


######################################
## Local post config

#####
## Internal packages config

include($$RATMAN_DIR/src/src.pri)

#####
## vic packages config

INCLUDEPATH += $$INCLUDE_DIR
LIBS += -L$$LIB_DIR

#include($$SHARE_DIR/vic/qmake/vic_cbdam_base.pri)
#include($$SHARE_DIR/vic/qmake/vic_geo_base.pri)
#include($$SHARE_DIR/vic/qmake/vic_geo_srs.pri)
include($$SHARE_DIR/vic/qmake/vic_img.pri)
include($$SHARE_DIR/vic/qmake/vic_os.pri)
include($$SHARE_DIR/vic/qmake/vic_curlstream.pri)
#include($$SHARE_DIR/vic/qmake/vic_vfs.pri)
include($$SHARE_DIR/vic/qmake/vic_gl.pri)
include($$SHARE_DIR/vic/qmake/vic_xml.pri)
include($$SHARE_DIR/vic/qmake/vic_mpi.pri)

#####
## External packages config

include($$SHARE_DIR/vic/qmakeconfig/xvic/xvic.pri)

#####
## Local DEFINE
item3d {
  DEFINES += ITEM3D
}

#####
## Dependencies

DEPENDPATH *= $$INCLUDEPATH


#--------------------------------------------------------------------
message("RATMAN configured") 
message("   CONFIG=$$CONFIG")
message("   PREFIX=$$PREFIX")
message("   INCLUDE_DIR=$$INCLUDE_DIR")
message("   BIN_DIR=$$BIN_DIR")
message("   LIB_DIR=$$LIB_DIR")
message("   DOC_DIR=$$DOC_DIR")
