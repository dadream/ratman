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
xvic_opentracker{
  unix {
    OPENTRACKER_DIR=$(OPENTRACKER_DIR)
    !exists($$OPENTRACKER_DIR/include/OpenTracker/OpenTracker.h) {
    	OPENTRACKER_DIR = /usr
	!exists($$OPENTRACKER_DIR/include/OpenTracker/OpenTracker.h) {
           OPENTRACKER_DIR = /usr/local
	   !exists($$OPENTRACKER_DIR/include/OpenTracker/OpenTracker.h) {
              error("Cannot find OPENTRACKER_DIR/include/OpenTracker/OpenTracker.h -- please set OPENTRACKER_DIR env var")
           }
        }
    }

    OPENTRACKER_LIB_DIR=$(OPENTRACKER_LIB_DIR)
    !exists($$OPENTRACKER_LIB_DIR/libOpenTracker.*) {
      OPENTRACKER_LIB_DIR=$$OPENTRACKER_DIR/lib64
      !exists($$OPENTRACKER_LIB_DIR/libOpenTracker.*) {
        OPENTRACKER_LIB_DIR=$$OPENTRACKER_DIR/lib
        !exists($$OPENTRACKER_LIB_DIR/libOpenTracker.*) {
          error("Cannot find OPENTRACKER_LIB_DIR/libOpenTracker.* -- please set OPENTRACKER_LIB_DIR env var")
        }
      }
    }

    INCLUDEPATH *= $$OPENTRACKER_DIR/include $$OPENTRACKER_DIR/include/OpenTracker 
    LIBS *= -L$$OPENTRACKER_LIB_DIR -lOpenTracker -lACE -lxerces-c -lncurses

   !exists($$ART_DIR/include/AR/ar.h) {
    	ART_DIR = /usr
	!exists($$ART_DIR/include/AR/ar.h) {
           ART_DIR = /usr/local
        }
    }

    !exists($$(ART_LIB_DIR)/libAR.*) {
      ART_LIB_DIR=$$ART_DIR/lib64
      !exists($$ART_LIB_DIR/libAR.*) {
        ART_LIB_DIR=$$ART_DIR/lib
        !exists($$ART_LIB_DIR/libAR.*) {
          message("  No ARToolKit")
        }
      }
    }

    exists($$ART_LIB_DIR/libAR.*) {
      LIBS+= -L$$ART_LIB_DIR -lARgsub -lARvideo -lAR -lARMulti
      message("  Using ARTToolikit")
    }
  }

  message("Configured for xvic_opentracker")
}
