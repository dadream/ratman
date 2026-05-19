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
include(../../ratman_pre.pri)
#--------------------------------------------------------------------

#QtSingleApplication
qtsingleapplication {
  include(../qtsingleapplication/qtsingleapplication.pri)
  DEFINES += QTSINGLEAPPLICATION
  message("Configured for QtSingleApplication")
}

TEMPLATE  = app 
CONFIG    -= staticlib
#CONFIG    += ratman_shaders ratman_ratman xvic_curl #console
CONFIG    += ratman_ratman xvic_curl xvic_sl console
QT        += network opengl xml widgets printsupport
FORMS     += meteo_dialog.ui
FORMS     += mainwindow.ui 
FORMS     += parametersdialog.ui
FORMS     += search_result.ui
FORMS     += bookmarks.ui
FORMS     += about.ui

win32:RC_FILE = appicon.rc

HEADERS   += \
version.hpp \
config.hpp \
base_layers_button_group.hpp \
overlay_layers_button_group.hpp \
layer_check_box.hpp qgl_nav3d_scene_view.hpp \
appwindow.hpp xml_config_parser.hpp \
meteo_dialog.hpp  \
about_dialog.hpp  \
parameters_dialog.hpp \
search_result_dialog.hpp \
search_result_item.hpp \
bookmarks_dialog.hpp \
bookmark_item.hpp 

SOURCES   += \
config.cpp \
base_layers_button_group.cpp \
overlay_layers_button_group.cpp \
layer_check_box.cpp qgl_nav3d_scene_view.cpp \
appwindow.cpp xml_config_parser.cpp main.cpp \
meteo_dialog.cpp \
about_dialog.cpp  \
parameters_dialog.cpp \
search_result_dialog.cpp \
search_result_item.cpp \
bookmarks_dialog.cpp \
bookmark_item.cpp


RESOURCES += ./graphics/resources.qrc
TRANSLATIONS = ./translations/ratman_it_IT.ts
TARGET     = vic_ratman_nav3d

target.path=$$BIN_DIR
INSTALLS+= target

#--------------------------------------------------------------------
include(../../ratman_post.pri)
#--------------------------------------------------------------------
LIBS += -lGLU

