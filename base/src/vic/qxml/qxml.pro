#--------------------------------------------------------------------
include (../../../base_pre.pri)
#--------------------------------------------------------------------

TEMPLATE= lib

CONFIG += qt 
HEADERS = \
          database.hpp 

SOURCES = \
          database.cpp
QT += xml
          
DEFINES += 

TARGET=vic_qxml

# --- INSTALL
install_pri.path=$$SHARE_DIR/vic/qmake
install_pri.files=vic_qxml.pri

install_inc.path=$$INCLUDE_DIR/vic/qxml/
install_inc.files=database.hpp
target.path=$$LIB_DIR

INSTALLS+= install_pri install_inc target


#--------------------------------------------------------------------
include (../../../base_post.pri)
#--------------------------------------------------------------------

