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
xvic_gphoto2{
  unix {
    GPHOTO2_DIR=$(GPHOTO2_DIR)
    !exists($$GPHOTO2_DIR/include/gphoto2/gphoto2.h) {
    	GPHOTO2_DIR = /usr
	!exists($$GPHOTO2_DIR/include/gphoto2/gphoto2.h) {
           GPHOTO2_DIR = /usr/local
	   !exists($$GPHOTO2_DIR/include/gphoto2/gphoto2.h) {
              error("Cannot find GPHOTO2_DIR/include/gphoto2/gphoto2.h -- please set GPHOTO2_DIR env var")
           }
        }
    }
    LIBS *= -lgphoto2
  }

  DEFINES +=
  INCLUDEPATH *= $$GPHOTO2_DIR/include $$GPHOTO2_DIR/include/gphoto2

  message("Configured for xvic_gphoto2")
}
