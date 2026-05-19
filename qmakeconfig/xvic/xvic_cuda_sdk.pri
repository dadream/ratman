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
xvic_cuda_sdk {
  win32 {

    CUDA_SDK_DIR = $(CUDA_SDK_DIR)
    !exists($$CUDA_SDK_DIR/common/inc/cutil.h) {
       error("Cannot find CUDA_SDK_DIR/common/inc/cutil.h -- please set CUDA_SDK_DIR env var")
    }
    
   CUDA_SDK_LIB_DIR=$(CUDA_SDK_DIR)
   !exists($$CUDA_SDK_LIB_DIR/common/lib/libcudpp*.*) {
     CUDA_LIB_DIR=$$CUDA_DIR	
     !exists($$CUDA_SDK_LIB_DIR/common/lib/libcudpp*.*) {
        error("Cannot find CUDA_LIB_DIR/lib/libcudpp*.* -- please set CUDA_SDK_LIB_DIR env var")
     }
   }
   
   LIBS+= $$CUDA_SDK_LIB_DIR/lib/lib*.lib
  }
  unix {
    CUDA_SDK_DIR=$(CUDA_SDK_DIR)
    !exists($$CUDA_SDK_DIR/common/inc/cutil.h) {
       CUDA_SDK_DIR=/opt/cuda/sdk
       !exists($$CUDA_SDK_DIR/common/inc/cutil.h) {
         CUDA_SDK_DIR=/usr/local/cuda/sdk
         !exists($$CUDA_SDK_DIR/common/inc/cutil.h) {
            CUDA_SDK_DIR=/u/vvr/packages/
            !exists($$CUDA_SDK_DIR/common/inc/cutil.h) {
               error("Cannot find CUDA_SDK_DIR/common/inc/cutil.h -- please set CUDA_SDK_DIR env var")
            }
         }
       }
    }

    CUDA_SDK_LIB_DIR=$(CUDA_SDK_DIR)/lib
    !exists($$CUDA_SDK_LIB_DIR/libcutil.*) {
      CUDA_SDK_LIB_DIR=$$CUDA_SDK_DIR/lib64
      !exists($$CUDA_SDK_LIB_DIR/libcutil.*) {
        CUDA_SDK_LIB_DIR=$$CUDA_SDK_DIR/lib        
        !exists($$CUDA_SDK_LIB_DIR/libcutil.*) {
          CUDA_SDK_LIB_DIR=$$CUDA_SDK_DIR/local/lib64        
          !exists($$CUDA_SDK_LIB_DIR/libcutil.*) {
            CUDA_SDK_LIB_DIR=$$CUDA_SDK_DIR/local/lib        
            !exists($$CUDA_SDK_LIB_DIR/libcutil.*) {
              error("Cannot find $$CUDA_SDK_LIB_DIR/lib*.* -- please set CUDA_SDK_LIB_DIR env var")
            }
          }
        }
      }
    }

    LIBS += -L$$CUDA_SDK_LIB_DIR -lcutil -lparamgl -lrendercheckgl

  } 

  message("Configured for CUDA SDK")
}
