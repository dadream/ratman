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
include(../qmakeconfig.pri)
CONFIG+= staticlib
TEMPLATE= lib
SOURCES=dummy.cpp

 PRI_FILES = \
  xvic.pri                xvic_mvdelta.pri        \
  xvic_opencv.pri         xvic_opentracker.pri    \
  xvic_sl.pri             xvic_mpi.pri            \
  xvic_lzo.pri            xvic_libxml2.pri        \
  xvic_shp.pri            xvic_gdal.pri           \
  xvic_proj.pri           xvic_lib3ds.pri         \
  xvic_curl.pri		  xvic_zlib.pri           \
  xvic_db4.pri            xvic_srbstream.pri      \
  xvic_vcg.pri            xvic_data_volume.pri    \
  xvic_dcmtk.pri          xvic_gle.pri 		  \  
  xvic_glew.pri           xvic_openscenegraph.pri \
  xvic_vtk.pri            xvic_gphoto2.pri        \
  xvic_cg.pri                                     \
  xvic_cuda.pri           xvic_cuda_sdk.pri       \
  


message("INSTALL PRI PATH " $$SHARE_DIR/vic/qmakeconfig/xvic)
install_pri.path=$$SHARE_DIR/vic/qmakeconfig/xvic
install_pri.destdir=$$SHARE_DIR/vic/qmakeconfig/xvic

install_pri.files=$$PRI_FILES

INSTALLS+= install_pri


