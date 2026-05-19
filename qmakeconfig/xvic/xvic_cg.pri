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
xvic_cg {
  win32 {
    CG_DIR = $(CG_DIR)
    !exists($$CG_DIR/include/Cg/cg.h) {
      error("Cannot find CG_DIR/include/Cg/cg.h -- please set CG_DIR env var") 
    }
    CG_LIB_DIR = $(CG_LIB_DIR)
    !exists($$CG_LIB_DIR/lib/CgGL.*) {
      CG_LIB_DIR=$$CG_DIR/lib
      !exists($$CG_LIB_DIR/lib/CgGL.*) {
        error("Cannot find CG_LIB_DIR/CgGL.* -- please set CG_LIB_DIR env var")
      }
    }
  }

  unix {
    CG_DIR=$(CG_DIR)
    !exists($$CG_DIR/include/Cg/cg.h) {
       CG_DIR=/usr/
       !exists($$CG_DIR/include/Cg/cg.h) {
         CG_DIR=/usr/local/
         !exists($$CG_DIR/include/Cg/cg.h) {
            CG_DIR=/u/vvr/packages/
            !exists($$CG_DIR/include/Cg/cg.h) {
               error("Cannot find CG_DIR/include/Cg/cg.h -- please set CG_DIR env var")
            }
         }
       }
    }
    
    CG_LIB_DIR=$(CG_LIB_DIR)
    !exists($$CG_LIB_DIR/libCgGL.*) {
      CG_LIB_DIR=$$CG_DIR/lib64
      !exists($$CG_LIB_DIR/libCgGL.*) {
        CG_LIB_DIR=$$CG_DIR/lib
        !exists($$CG_LIB_DIR/libCgGL.*) {
           error("Cannot find CG_LIB_DIR/libCgGL.* -- please set CG_LIB_DIR env var")
        }
      }
    }
  }

  LIBS += -L$$CG_LIB_DIR -lCg -lCgGL 
  CONFIG += opengl

  message("Configured for NVIDIA Cg")

}