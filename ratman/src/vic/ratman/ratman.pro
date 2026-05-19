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
#--------------------------------------------------------------------
include (../../../ratman_pre.pri)
#--------------------------------------------------------------------
QT += opengl network xml widgets printsupport
CONFIG += vic_cbdam_base vic_geo_base vic_geo_srs vic_gl qt thread opengl x11 xvic_curl xvic_proj ratman_shaders

item3d {
  CONFIG += xvic_openscenegraph
}

TEMPLATE=lib


!qtsingleapplication {
HEADERS= \
  tcp_client.hpp \
  tcp_server.hpp \
}

HEADERS += \
configuration.hpp \
string_utility.hpp \
s3d_parser.hpp \
oriented_position.hpp \
network.hpp \
geonames_service.hpp \
bookmarks_service.hpp \
local_geonames_service.hpp \
wfs_geonames_service.hpp \
active_renderable.hpp terrain_renderable.hpp \
decorated_terrain_view.hpp \
placemark_icon.xpm \
camera_controller.hpp  camera_animation.hpp \
compass.xpm compass.hpp \
control_buttons.xpm control_buttons.hpp \
film_tile.xpm snapshots.hpp \
tape_tile.xpm \
logo.xpm logo.hpp \
sundisk.hpp atmosphere.hpp \
fixed_label.hpp \
qgl_scene_view.hpp \
http_request.hpp meteo_data.hpp \
terrain_tile_meteo.hpp \
browser.hpp \
terrain_billboard_placemarks.hpp \
terrain_placenames.hpp \
terrain_item3d.hpp \
bookmarks.hpp \
copyright.hpp 

item3d {
  HEADERS += \
  osg_item3d.hpp \
  item3d_factory.hpp \
  osg_item3d_factory.hpp \
  item3d_manager.hpp \
  item3d_xml_reader.hpp
}

HEADERS += \
Icons/bkg.xpm \
Icons/bkn_s.xpm \
Icons/close_hand.xpm \
Icons/clr_s.xpm \
Icons/few_s.xpm \
Icons/gr_s.xpm \
Icons/nodata.xpm \
Icons/open_hand.xpm \
Icons/ovc_s.xpm \
Icons/ra_s.xpm \
Icons/sct_s.xpm \
Icons/sn_s.xpm \
Icons/rot_3d.xpm 

!qtsingleapplication {
SOURCES = \
  tcp_client.cpp \
  tcp_server.cpp \
}

SOURCES += \
configuration.cpp \
s3d_parser.cpp \
string_utility.cpp \
network.cpp \
oriented_position.cpp \
bookmarks_service.cpp \
local_geonames_service.cpp \
wfs_geonames_service.cpp \
active_renderable.cpp terrain_renderable.cpp \
decorated_terrain_view.cpp \
camera_controller.cpp  camera_animation.cpp \
compass.cpp \
control_buttons.cpp \
snapshots.cpp \
logo.cpp \
atmosphere.cpp \
fixed_label.cpp \
qgl_scene_view.cpp \
http_request.cpp meteo_data.cpp \
terrain_tile_meteo.cpp \
browser.cpp \
terrain_billboard_placemarks.cpp \
terrain_placenames.cpp \
terrain_item3d.cpp \
bookmarks.cpp \
copyright.cpp 

item3d {
  SOURCES += \
  osg_item3d.cpp \
  osg_item3d_factory.cpp \
  item3d_manager.cpp \
  item3d_xml_reader.cpp 
}

TARGET=vic_ratman

# --- EXTRA

win32:LIBS += -lshell32

# --- INSTALL

install_pri.path=$$SHARE_DIR/vic/qmake
install_pri.files=vic_ratman.pri

install_inc.path=$$INCLUDE_DIR/vic/ratman/
install_inc.files=$$HEADERS
target.path=$$LIB_DIR

INSTALLS+= install_pri install_inc target

#--------------------------------------------------------------------
include (../../../ratman_post.pri)
#--------------------------------------------------------------------

