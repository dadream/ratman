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
##########################################################
# Include file, defining configurations for all base libs
##########################################################

INCLUDEPATH += ./ $$BASE_DIR/src/

include ($$BASE_DIR/src/vic/gl/src.pri)
include ($$BASE_DIR/src/vic/mpi/src.pri)
include ($$BASE_DIR/src/vic/xml/src.pri)
include ($$BASE_DIR/src/vic/qxml/src.pri)
include ($$BASE_DIR/src/vic/curlstream/src.pri)
include ($$BASE_DIR/src/vic/img/src.pri)
include ($$BASE_DIR/src/vic/math/src.pri)
include ($$BASE_DIR/src/vic/fetcher/src.pri)

##########################################################
## CHECK Berkeley db available?
## FIXME

HAVE_DB4=true

DB4_DIR=$(DB4_DIR)
!exists($$DB4_DIR/include/db.h) {
   DB4_DIR=/usr/
   !exists($$DB4_DIR/include/db.h) {
     DB4_DIR=/usr/local/
     !exists($$DB4_DIR/include/db.h) {
       HAVE_DB4=false
     }
   }
}


contains(HAVE_DB4, true) {
  message("USING DB4 PERSISTENT STORAGE")
  include ($$BASE_DIR/src/vic/persistent/src.pri)
} else {
  message("NOT USING DB4 PERSISTENT STORAGE")
}

