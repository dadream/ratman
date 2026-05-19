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
xvic_dcmtk {
  win32 {
  }
  unix {
    DCMTK_DIR=$(DCMTK_DIR)
    !exists($$DCMTK_DIR/include/dcmtk/dcmdata/dcfilefo.h) {
      DCMTK_DIR=/usr
      !exists($$DCMTK_DIR/include/dcmtk/dcmdata/dcfilefo.h) {
        DCMTK_DIR=/usr/dicom
        !exists($$DCMTK_DIR/include/dcmtk/dcmdata/dcfilefo.h) {
          DCMTK_DIR=/usr/local/dicom
          !exists($$DCMTK_DIR/include/dcmtk/dcmdata/dcfilefo.h) {
             error("Cannot find DCMTK_DIR/include/dcmdata/dcfilefo.h -- please set DCMTK_DIR env var")
          }
        }
      }
    }
    
    DCMTK_LIB_DIR=$(DCMTK_LIB_DIR)
    !exists($$DCMTK_LIB_DIR/libdcmdata.*) {
      DCMTK_LIB_DIR=$$DCMTK_DIR/lib64
      !exists($$DCMTK_LIB_DIR/libdcmdata.*) {
        DCMTK_LIB_DIR=$$DCMTK_DIR/lib
        !exists($$DCMTK_LIB_DIR/libdcmdata.*) {
          error("Cannot find DCMTK_LIB_DIR/libdcmdata.* -- please set DCMTK_LIB_DIR env var")
        }
      }
    }

    LIBS += -L$$DCMTK_LIB_DIR -ldcmdata -ldcmdsig -ldcmimage -ldcmimgle -ldcmjpeg -ldcmnet -ldcmpstat -ldcmsr -ldcmtls -ldcmwlm -lijg12 -lijg16 -lijg8 -lofstd

  } 

  DEFINES *= HAVE_CONFIG_H NDEBUG _REENTRANT _XOPEN_SOURCE_EXTENDED _XOPEN_SOURCE=500 _BSD_SOURCE _BSD_COMPAT _OSF_SOURCE _POSIX_C_SOURCE=199506L

  message("Configured for xvic_dcmtk")
}
