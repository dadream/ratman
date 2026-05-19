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
vic_cbdam_base {
  win32 {
    PRE_TARGETDEPS += $$RATMAN_DIR/src/vic/cbdam/base/$$BUILD_SUBDIR/vic_cbdam_base$${TARGET_SUFFIX}.lib
    LIBS += $$RATMAN_DIR/src/vic/cbdam/base/$$BUILD_SUBDIR/vic_cbdam_base$${TARGET_SUFFIX}.lib
  }
  unix {
    LIBS += -L$$RATMAN_BUILD_DIR/src/vic/cbdam/base/$$BUILD_SUBDIR -lvic_cbdam_base$${TARGET_SUFFIX}
  }

  INCLUDEPATH += 
  DEPENDPATH += $$RATMAN_DIR/src/vic/cbdam/base/

  CONFIG+= vic_cbdam_os vic_img vic_vfs vic_curlstream vic_xml vic_geo_base vic_geo_srs xvic_curl xvic_sl xvic_gdal xvic_shp zlib xvic_glew
  QT+=opengl
  unix:CONFIG+= vic_persistent
}