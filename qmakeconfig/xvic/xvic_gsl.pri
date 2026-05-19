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
xvic_gsl {
  win32 {
    GSL_DIR = $(GSL_DIR)
    !exists($$GSL_DIR/include/gsl/gsl_matrix.h) {
       error("Cannot find GSL_DIR/include/gsl/gsl_matrix.h -- please set GSL_DIR env var")
    }

    GSL_LIB_DIR=$$GSL_DIR
    !exists($$GSL_LIB_DIR/gsl.*) {
       error("Cannot find GSL_LIB_DIR/gsl.* -- please set GSL_LIB_DIR env var") 	
    }	

    LIBS += -L$$GSL_LIB_DIR -lgsl
     message("Configured for xvic_gsl")
  }
  unix {
    GSL_DIR=$(GSL_DIR)
    !exists($$GSL_DIR/include/gsl/gsl_matrix.h) {
      GSL_DIR=/usr/
      !exists($$GSL_DIR/include/gsl/gsl_matrix.h) {
        GSL_DIR=/usr/local/
          !exists($$GSL_DIR/include/gsl/gsl_matrix.h) {
            GSL_DIR=/u/vvr/packages/
            !exists($$GSL_DIR/include/gsl/gsl_matrix.h) {
	      GSL_DIR=$$(PREFIX)
              !exists($$GSL_DIR/include/gsl/gsl_matrix.h) {
                error("Cannot find GSL_DIR/include/gsl/gsl_matrix.h Please set GSL_DIR env var.")
              }
            }
         }
      }	
    }
   
    GSL_LIB_DIR=$(GSL_LIB_DIR)
    !exists($$GSL_LIB_DIR/libgsl.*) {
      GSL_LIB_DIR=$$GSL_DIR/lib64
      !exists($$GSL_LIB_DIR/libgsl.*) {
        GSL_LIB_DIR=$$GSL_DIR/lib
        !exists($$GSL_LIB_DIR/libgsl.*) {
	    error("Cannot find GSL_LIB_DIR/libgsl.* Please set GSL_LIB_DIR env var")
        }
      }
    }
    
    LIBS += -L$$GSL_LIB_DIR -lgsl 

    message("Configured for xvic_gsl - GNU Scientific Library")
  }
}

