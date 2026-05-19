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
xvic_vcg {
  win32 {
  }
  unix {
    !exists($$VCG_DIR/vcg/simplex/vertex/vertex.h) {
      VCG_DIR = /usr/include/vcg
      !exists($$VCG_DIR/vcg/simplex/vertex/vertex.h) {
        VCG_DIR = /usr/local/include/vcg
        !exists($$VCG_DIR/vcg/simplex/vertex/vertex.h) {
           error("Cannot find VCG_DIR/vcg/simplex/vertex/vertex.h  -- please set VCG_DIR env var");
        }
      }
    }
  } 


  message("Configured for xvic_vcg")
}
