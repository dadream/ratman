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
vic_mpi {
  win32 {
    PRE_TARGETDEPS += $$BASE_DIR/src/vic/mpi/$$BUILD_SUBDIR/vic_mpi$${TARGET_SUFFIX}.lib
    LIBS += $$BASE_DIR/src/mpi/$$BUILD_SUBDIR/vic_mpi$${TARGET_SUFFIX}.lib
  }
  unix {
    PRE_TARGETDEPS += $$BASE_DIR/src/vic/mpi/$$BUILD_SUBDIR/libvic_mpi$${TARGET_SUFFIX}.a
    LIBS += $$BASE_DIR/src/vic/mpi/$$BUILD_SUBDIR/libvic_mpi$${TARGET_SUFFIX}.a 
   }
  CONFIG += xvic_mpi xvic_sl
  INCLUDEPATH += 
}