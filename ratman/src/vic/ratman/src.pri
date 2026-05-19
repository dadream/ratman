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
message(CONFIG= $$CONFIG)
ratman_ratman {
  win32 {
    PRE_TARGETDEPS += $$RATMAN_DIR/src/vic/ratman/$$BUILD_SUBDIR/vic_ratman$${TARGET_SUFFIX}.lib
    LIBS += $$RATMAN_DIR/src/vic/ratman/$$BUILD_SUBDIR/vic_ratman$${TARGET_SUFFIX}.lib shell32.lib
  }
  unix {
    LIBS += -L$$RATMAN_BUILD_DIR/src/vic/ratman/$$BUILD_SUBDIR -lvic_ratman$${TARGET_SUFFIX}
  }

  INCLUDEPATH +=


  CONFIG+= vic_cbdam_base vic_gl qt thread opengl x11 xvic_proj ratman_shaders

  item3d {
    CONFIG += xvic_openscenegraph
  }


#  CONFIG+= cbdam_base vic_geo_base vic_geo_srs vic_gl qt thread opengl x11 xvic_curl xvic_proj ratman_shaders
#  CONFIG+= vic_opossum vic_opossum_buildings vic_vfs vic_curlstream vic_gl qt thread opengl x11 xvic_curl xvic_proj

  message("Configured for ratman_ratman")
}
