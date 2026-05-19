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
xvic_data_volume {
  win32 {
  }
  unix {
    DATA_VOLUME_DIR=$(DATA_VOLUME_DIR)
    !exists($$DATA_VOLUME_DIR/include/data_volume/data_volume.hpp) {
      DATA_VOLUME_DIR=/usr/
      !exists($$DATA_VOLUME_DIR/include/data_volume/data_volume.hpp) {
        DATA_VOLUME_DIR=/usr/local/
        !exists($$DATA_VOLUME_DIR/include/data_volume/data_volume.hpp) {
          DATA_VOLUME_DIR=/u/vvr/packages/
          !exists($$DATA_VOLUME_DIR/include/data_volume/data_volume.hpp) {
             error("Cannot find DATA_VOLUME_DIR/include/data_volume/data_volume.hpp -- please set DATA_VOLUME_DIR env var")
          }
        }
      }
    }
    
    DATA_VOLUME_LIB_DIR=$(DATA_VOLUME_LIB_DIR)
    !exists($$DATA_VOLUME_LIB_DIR/libdata_volume.*) {
      DATA_VOLUME_LIB_DIR=$$DATA_VOLUME_DIR/lib64
      !exists($$DATA_VOLUME_LIB_DIR/libdata_volume.*) {
        DATA_VOLUME_LIB_DIR=$$DATA_VOLUME_DIR/lib
        !exists($$DATA_VOLUME_LIB_DIR/libdata_volume.*) {
          error("Cannot find DATA_VOLUME_LIB_DIR/libdata_volume.* -- please set DATA_VOLUME_LIB_DIR env var")
        }
      }
    }

    LIBS += -L$$DATA_VOLUME_LIB_DIR -ldata_volume
    CONFIG += xvic_dcmtk

  } 


  message("Configured for xvic_data_volume")
}
