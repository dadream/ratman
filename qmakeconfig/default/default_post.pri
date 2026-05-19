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
############################
## Misc config

QMAKE_EXT_H = .hpp

debug {
  release {
    error("Choose debug or release")
  }
}

!debug {
  !release {
    error("Choose debug or release")
  }
}

debug {
  TARGET_SUFFIX=_dbg
  BUILD_SUBDIR=Debug
  DEFINES *= _GLIBCXX_DEBUG
  message("Configured for debug build")
}

release {
  TARGET_SUFFIX=
  DEFINES -= _GLIBCXX_DEBUG
  DEFINES *= NDEBUG
  BUILD_SUBDIR=Release
  message("Configured for release build")
}

TARGET=$${TARGET}$${TARGET_SUFFIX}

lib32 {
  DEFINES += __LIB32__
}

lib64 {
  DEFINES += __LIB64__
}

contains(TEMPLATE,lib) {
  DESTDIR=$$BUILD_SUBDIR
}

OBJECTS_DIR=$$BUILD_SUBDIR
MOC_DIR=$$BUILD_SUBDIR
UI_DIR=

### to enable RTTI support under win32
win32:QMAKE_CXXFLAGS_RELEASE += /GR

unix:QMAKE_CXXFLAGS_RELEASE -= -O -O2
unix:QMAKE_CXXFLAGS_RELEASE += -O3 -ffast-math
#unix:QMAKE_CXXFLAGS_RELEASE += -O3 -foptimize-sibling-calls -ffast-math -fno-math-errno -funsafe-math-optimizations -fno-trapping-math -funroll-loops -funroll-all-loops

win32:QMAKE_CXXFLAGS_RELEASE -= -Od -Ob1s
win32:QMAKE_CXXFLAGS_RELEASE += -O2

#### LARGE FILES

DEFINES += _FILE_OFFSET_BITS=64 _LARGEFILE_SOURCE _LARGEFILE64_SOURCE

#--------------------------------------------------------------------
message("DEFAULT configured") 
message("   PREFIX=$$PREFIX")
message("   INCLUDE_DIR=$$INCLUDE_DIR")
message("   BIN_DIR=$$BIN_DIR")
message("   LIB_DIR=$$LIB_DIR")
message("   SHARE_DIR=$$SHARE_DIR")
message("   DOC_DIR=$$DOC_DIR")
