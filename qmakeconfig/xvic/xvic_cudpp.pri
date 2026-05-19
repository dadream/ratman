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
xvic_cudpp {
  win32 {

    CUDPP_DIR = $(CUDA_SDK_DIR)
    !exists($$CUDPP_DIR/common/inc/cudpp/cudpp.h) {
       error("Cannot find CUDPP_DIR/common/inc/cudpp/cudpp.h -- please set CUDPP_DIR env var")
    }
    
   CUDPP_LIB_DIR=$(CUDPP_DIR)
   !exists($$CUDPP_LIB_DIR/common/lib/linux/libcudpp*.*) {
     CUDA_LIB_DIR=$$CUDA_DIR	
     !exists($$CUDPP_LIB_DIR/common/lib/linux/libcudpp*.*) {
        error("Cannot find CUDA_LIB_DIR/lib/linux/libcudpp*.* -- please set CUDPP_LIB_DIR env var")
     }
   }
   
   LIBS+= $$CUDPP_LIB_DIR/common/lib/cudpp*.lib
  }
  unix {
    CUDA_SDK_DIR=$(CUDA_SDK_DIR)
    !exists($$CUDA_SDK_DIR/common/inc/cutil.h) {
       CUDA_SDK_DIR=/opt/cuda-sdk
       !exists($$CUDA_SDK_DIR/common/inc/cutil.h) {
         CUDA_SDK_DIR=/usr/local/cuda-sdk
         !exists($$CUDA_SDK_DIR/common/inc/cutil.h) {
            CUDA_SDK_DIR=/u/vvr/packages/
            !exists($$CUDA_SDK_DIR/common/inc/cutil.h) {
               error("Cannot find CUDA_SDK_DIR/common/inc/cutil.h -- please set CUDA_SDK_DIR env var")
            }
         }
       }
    }

    CUDPP_DIR=$(CUDA_SDK_DIR)
    !exists($$CUDPP_DIR/common/inc/cudpp/cudpp.h) {
       CUDPP_DIR=/usr/
       !exists($$CUDPP_DIR/common/inc/cudpp/cudpp.h) {
         CUDPP_DIR=/usr/local/
         !exists($$CUDPP_DIR/common/inc/cudpp/cudpp.h) {
            CUDPP_DIR=/u/vvr/packages/
            !exists($$CUDPP_DIR/common/inc/cudpp/cudpp.h) {
               error("Cannot find CUDPP_DIR/common/inc/cudpp/cudpp.h -- please set CUDPP_DIR env var")
            }
         }
       }
    }

    CUDPP_LIB_DIR=$$CUDPP_DIR/common/lib/linux
    !exists($$CUDPP_LIB_DIR/lib*.*) {
      CUDPP_LIB_DIR=$$CUDPP_DIR/lib64
      !exists($$CUDPP_LIB_DIR/lib*.*) {
        CUDPP_LIB_DIR=$$CUDPP_DIR/lib        
        !exists($$CUDPP_LIB_DIR/lib*.*) {
          CUDPP_LIB_DIR=$$CUDPP_DIR/local/lib64        
          !exists($$CUDPP_LIB_DIR/lib*.*) {
            CUDPP_LIB_DIR=$$CUDPP_DIR/local/lib        
            !exists($$CUDPP_LIB_DIR/lib*.*) {
              error("Cannot find $$CUDPP_LIB_DIR/lib*.* -- please set CUDPP_LIB_DIR env var")
            }
          }
        }
      }
    }

    LIBS += -L$$CUDPP_LIB_DIR -lcudpp64

  } 

  message("Configured for CUDPP")
}
