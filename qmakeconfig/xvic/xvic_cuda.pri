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
xvic_cuda {
  win32 {
    CUDA_DIR = $(CUDA_DIR)    
    !exists($$CUDA_DIR/include/cuda.h) {
    	error("Cannot find CUDA_LIB_DIR/lib/libcudart*.* -- please set CUDA_DIR env var")          
    }

   CUDA_LIB_DIR=$(CUDA_DIR)
   !exists($$CUDA_LIB_DIR/lib/libcu*.*) {
     CUDA_LIB_DIR=$$CUDA_DIR	
     !exists($$CUDA_LIB_DIR/lib/libcu*.*) {
        error("Cannot find CUDA_LIB_DIR/lib/libcudart*.* -- please set CUDA_LIB_DIR env var")
     }
   }
      
   LIBS+= $$CUDA_LIB_DIR/lib/libcu*.lib
  }
  unix {
    CUDA_DIR=$(CUDA_DIR)
    !exists($$CUDA_DIR/include/cuda.h) {
       CUDA_DIR=/opt/cuda
       !exists($$CUDA_DIR/include/cuda.h) {
         CUDA_DIR=/usr/local/cuda
         !exists($$CUDA_DIR/include/cuda.h) {
            CUDA_DIR=/u/vvr/packages/
            !exists($$CUDA_DIR/include/cuda.h) {
               error("Cannot find CUDA_DIR/include/cuda.h -- please set CUDA_DIR env var")
            }
         }
       }
    }
    
    CUDA_LIB_DIR=$(CUDA_DIR)
    !exists($$CUDA_LIB_DIR/libcu*.*) {
      CUDA_LIB_DIR=$$CUDA_DIR/lib64
      !exists($$CUDA_LIB_DIR/libcu*.*) {
        CUDA_LIB_DIR=$$CUDA_DIR/lib        
        !exists($$CUDA_LIB_DIR/libcu*.*) {
          CUDA_LIB_DIR=$$CUDA_DIR/local/lib64        
          !exists($$CUDA_LIB_DIR/libcu*.*) {
            CUDA_LIB_DIR=$$CUDA_DIR/local/lib        
            !exists($$CUDA_LIB_DIR/libcu*.*) {
              error("Cannot find CUDA_LIB_DIR/libcu*.* -- please set CUDA_LIB_DIR env var")
            }
          }
        }
      }
    }

    LIBS += -L$$CUDA_LIB_DIR -lcudart -lcufft -lcublas

  } 

  message("Configured for CUDA Toolkit")
}
