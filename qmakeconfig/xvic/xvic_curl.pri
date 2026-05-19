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
xvic_curl {
  unix {
    CURL_DIR = /usr
    CURL_LIB_DIR = /usr/lib/x86_64-linux-gnu
    LIBS += -L$$CURL_LIB_DIR -lcurl
    # DO NOT add /usr/include to INCLUDEPATH to avoid include_next issues
    !equals(CURL_DIR, "/usr") {
    }
  }
  win32 {
    CURL_DIR = $(CURL_DIR)
    CURL_LIB_DIR = $(CURL_LIB_DIR)
    LIBS+= $$CURL_LIB_DIR/lib/libcurl.lib wsock32.lib winmm.lib 
    DEFINES += CURL_STATICLIB
  }
  message("Configured for curl")
}