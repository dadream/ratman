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
include (../../../../ratman_pre.pri)
#--------------------------------------------------------------------

TEMPLATE= lib

CONFIG += xvic_gdal

unix:CONFIG += vic_mpi

HEADERS= geo_utility.hpp quad_accessor.hpp quad_processor.hpp \
         geo_transform.hpp color_remap_transform.hpp quad_warper.hpp \
         quad_builder.hpp
unix:HEADERS+= mpi_quad_builder.hpp

SOURCES= geo_utility.cpp quad_accessor.cpp quad_processor.cpp \
         geo_transform.cpp color_remap_transform.cpp quad_warper.cpp \
         quad_builder.cpp
unix:SOURCES+= mpi_quad_builder.cpp

TARGET= vic_geo_builder

# --- INSTALL
install_pri.path=$$SHARE_DIR/vic/qmake
install_pri.files=vic_geo_builder.pri

install_inc.path=$$INCLUDE_DIR/vic/geo/builder
install_inc.files=$$HEADERS
target.path=$$LIB_DIR

INSTALLS+= install_pri install_inc target


#--------------------------------------------------------------------
include (../../../../ratman_post.pri)
#--------------------------------------------------------------------

