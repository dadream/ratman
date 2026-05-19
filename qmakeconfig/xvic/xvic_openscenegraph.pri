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
xvic_openscenegraph {
  win32 {
    OPENSCENEGRAPH_DIR = $(OPENSCENEGRAPH_DIR)
    !exists($$OPENSCENEGRAPH_DIR/include/osg/Node) {
       error("Cannot find OPENSCENEGRAPH_DIR/include/osg/Node -- please set OPENSCENEGRAPH_DIR env var")
    }
   OPENSCENEGRAPH_LIB_DIR=$(OPENSCENEGRAPH_LIB_DIR)
   !exists($$OPENSCENEGRAPH_LIB_DIR/lib/libosg*.*) {
     OPENSCENEGRAPH_LIB_DIR=$$OPENSCENEGRAPH_DIR	
     !exists($$OPENSCENEGRAPH_LIB_DIR/lib/libosg*.*) {
        error("Cannot find OPENSCENEGRAPH_LIB_DIR/lib/libosg*.* -- please set OPENSCENEGRAPH_LIB_DIR env var")
     }
   }
   LIBS+= $$OPENSCENEGRAPH_LIB_DIR/lib/libosg*.lib
  }
  unix {
    OPENSCENEGRAPH_DIR=$(OPENSCENEGRAPH_DIR)
    !exists($$OPENSCENEGRAPH_DIR/include/osg/Node) {
       OPENSCENEGRAPH_DIR=/usr/
       !exists($$OPENSCENEGRAPH_DIR/include/osg/Node) {
         OPENSCENEGRAPH_DIR=/usr/local/
         !exists($$OPENSCENEGRAPH_DIR/include/osg/Node) {
            OPENSCENEGRAPH_DIR=/u/vvr/packages/
            !exists($$OPENSCENEGRAPH_DIR/include/osg/Node) {
               error("Cannot find OPENSCENEGRAPH_DIR/include/osg/Node -- please set OPENSCENEGRAPH_DIR env var")
            }
         }
       }
    }
    
    OPENSCENEGRAPH_LIB_DIR=$(OPENSCENEGRAPH_LIB_DIR)
    !exists($$OPENSCENEGRAPH_LIB_DIR/libosg*.*) {
      OPENSCENEGRAPH_LIB_DIR=$$OPENSCENEGRAPH_DIR/lib64
      !exists($$OPENSCENEGRAPH_LIB_DIR/libosg*.*) {
        OPENSCENEGRAPH_LIB_DIR=$$OPENSCENEGRAPH_DIR/lib        
        !exists($$OPENSCENEGRAPH_LIB_DIR/libosg*.*) {
          OPENSCENEGRAPH_LIB_DIR=$$OPENSCENEGRAPH_DIR/local/lib64        
          !exists($$OPENSCENEGRAPH_LIB_DIR/libosg*.*) {
            OPENSCENEGRAPH_LIB_DIR=$$OPENSCENEGRAPH_DIR/local/lib        
            !exists($$OPENSCENEGRAPH_LIB_DIR/libosg*.*) {
              error("Cannot find OPENSCENEGRAPH_LIB_DIR/libosg*.* -- please set OPENSCENEGRAPH_LIB_DIR env var")
            }
          }
        }
      }
    }

    LIBS += -L$$OPENSCENEGRAPH_LIB_DIR -losg -losgDB -losgGA -losgUtil -losgViewer

  } 

  message("Configured for OpenSceneGraph")
}
