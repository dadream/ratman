#--------------------------------------------------------------------
include (../../../base_pre.pri)
#--------------------------------------------------------------------

TEMPLATE= lib

CONFIG += xvic_curl xvic_zlib

HEADERS= url.hpp curlstream.hpp 
SOURCES= url.cpp curlstream.cpp 

#SOURCES += curlstreamtest.cpp

TARGET= vic_curlstream

# --- INSTALL
install_pri.path=$$SHARE_DIR/vic/qmake
install_pri.files=vic_curlstream.pri

install_inc.path=$$INCLUDE_DIR/vic/curlstream
install_inc.files=$$HEADERS
target.path=$$LIB_DIR

INSTALLS+= install_pri install_inc target


#--------------------------------------------------------------------
include (../../../base_post.pri)
#--------------------------------------------------------------------

