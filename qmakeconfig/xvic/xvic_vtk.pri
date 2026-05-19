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
xvic_vtk {
  win32 {
    VTK_DIR = $(VTK_DIR)
    !exists($$VTK_DIR/include/vtkDataSetReader.h) {
       error("Cannot find VTK_DIR/include/vtkDataSetReader.h  -- please set VTK_DIR env var")
    }
   VTK_LIB_DIR=$(VTK_LIB_DIR)
   !exists($$VTK_LIB_DIR/lib/vtk.*) {
     VTK_LIB_DIR=$$VTK_DIR	
     !exists($$VTK_LIB_DIR/lib/vtk.*) {
        error("Cannot find VTK_LIB_DIR/lib/vtk.* -- please set VTK_LIB_DIR env var")
     }
   }
   LIBS+= $$VTK_LIB_DIR/lib/vtk.lib
  }
  unix {
    VTK_DIR=$(VTK_DIR)
    !exists($$VTK_DIR/include/vtk-5.0/vtkDataSetReader.h) {
       VTK_DIR=/usr/
       !exists($$VTK_DIR/include/vtk-5.0/vtkDataSetReader.h) {
         VTK_DIR=/usr/local/
         !exists($$VTK_DIR/include/vtk-5.0/vtkDataSetReader.h) {
            error("Cannot find VTK_DIR/vtk-5.0/vtkDataSetReader.h -- please set VTK_DIR env var")
         }
       }
    }
    
    VTK_LIB_DIR=$(VTK_LIB_DIR)
    !exists($$VTK_LIB_DIR/libvtk*.*) {
      VTK_LIB_DIR=$$VTK_DIR/lib64
      !exists($$VTK_LIB_DIR/libvtk*.*) {
        VTK_LIB_DIR=$$VTK_DIR/lib
        !exists($$VTK_LIB_DIR/libvtk*.*) {
           error("Cannot find VTK_LIB_DIR/libvtk*.* -- please set VTK_LIB_DIR env var")
        }
      }
    }

    LIBS += -L$$VTK_LIB_DIR -lvtkCommon -lvtkDICOMParser -lvtkFiltering \
            -lvtkGenericFiltering -lvtkGraphics \
            -lvtkHybrid -lvtkIO  \
            -lvtkImaging -lvtkNetCDF -lvtkRendering \
	    -lvtkVolumeRendering -lvtkWidgets \
	    -lvtkexoIIc -lvtkfreetype -lvtkftgl -lvtksys
  }


  message("Configured for vtk")
}
