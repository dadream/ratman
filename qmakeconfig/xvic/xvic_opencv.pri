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
xvic_opencv{
  unix {
    OPENCV_DIR=$(OPENCV_DIR)
    !exists($$OPENCV_DIR/include/opencv/cv.h) {
    	OPENCV_DIR = /usr
	!exists($$OPENCV_DIR/include/opencv/cv.h) {
           OPENCV_DIR = /usr/local
	   !exists($$OPENCV_DIR/include/opencv/cv.h) {
              error("Cannot find OPENCV_DIR/include/opencv/cv.h -- please set OPENCV_DIR env var")
           }
        }
    }
    LIBS *= -lcxcore -lcv -lhighgui
  }

  DEFINES += CV_SSE2=0
  INCLUDEPATH *= $$OPENCV_DIR/include $$OPENCV_DIR/include/opencv

  message("Configured for xvic_opencv")
}
