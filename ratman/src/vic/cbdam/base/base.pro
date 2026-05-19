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
include(../../../../ratman_pre.pri)
#--------------------------------------------------------------------

TEMPLATE = lib

QT +=  opengl
#QT +=  qt3support 
CONFIG += vic_cbdam_os vic_vfs vic_curlstream vic_img vic_geo_base vic_geo_srs vic_xml xvic_curl xvic_zlib xvic_sl xvic_gdal xvic_shp qt opengl xvic_glew

win32:CONFIG+= vic_persistent

FORMS = 


HEADERS = \
imgfilter.hpp                      imgfilter_bell.hpp \
img_operations.hpp \
builder.hpp                        geometry_layer.hpp \
building_hierarchy.hpp             grid_diamond.hpp \
building_renderer.hpp              grid_diamond_graph.hpp \
byte_array_accessor.hpp            grid_diamond_graph_incore.hpp \
camera.hpp                         grid_diamond_graph_off_core.hpp \
camera_controller_base.hpp         grid_diamond_state.hpp \
camera_controller_flight.hpp       grid_point.hpp \
camera_controller_vtrackball.hpp   grid_texture_quadtree.hpp \
cbdam_diamond_fetcher.hpp          null_compressor.hpp \
opengl_cached_data_renderer.hpp \
color_rgb.hpp                      priority_diamond.hpp \
compressed_rgba32_image.hpp        progress_bar.hpp \
config.hpp                         raw_image.hpp \
coordinate_transform.hpp           ray.hpp \
delta_codec.hpp                    reference_counted_cache.hpp \
delta_height_codec.hpp             repository_parameters.hpp \
diamond_graph_builder.hpp          terrain_model.hpp \
diamond_operator.hpp               terrain_model_renderer.hpp \
diamond_patch_accessor.hpp         terrain_scene_compiler.hpp \
diamond_repository.hpp \
diamond_repository_procedural.hpp  texture_layer.hpp \
diamond_repository_storage.hpp     texture_manager.hpp \
diamond_vertices.hpp               texture_refiner.hpp \
dummy_geoimage_quad_fetcher.hpp    background_thread.hpp \
geodata_fetcher.hpp                triangulate.hpp \
geoimage_quad_fetcher.hpp          victms_geoimage_quad_fetcher.hpp \
wms_geoimage_quad_fetcher.hpp      loaded_geoimage_quad_fetcher.hpp 

win32:HEADERS -= grid_diamond_graph_off_core.hpp

SOURCES = \
img_operations.cpp \
building_hierarchy.cpp            grid_diamond_graph_incore.cpp \
building_renderer.cpp             grid_point.cpp \
camera.cpp                        grid_texture_quadtree.cpp \
camera_controller_base.cpp        opengl_cached_data_renderer.cpp \
camera_controller_flight.cpp      progress_bar.cpp \
camera_controller_vtrackball.cpp  raw_image.cpp \
cbdam_diamond_fetcher.cpp         repository_parameters.cpp \
compressed_rgba32_image.cpp       terrain_model.cpp \
delta_height_codec.cpp            terrain_model_renderer.cpp \
diamond_graph_builder.cpp         terrain_scene_compiler.cpp \
diamond_operator.cpp \
dummy_geoimage_quad_fetcher.cpp   texture_layer.cpp \
geodata_fetcher.cpp               texture_manager.cpp \
geoimage_quad_fetcher.cpp         texture_refiner.cpp \
geometry_layer.cpp                victms_geoimage_quad_fetcher.cpp \
wms_geoimage_quad_fetcher.cpp     loaded_geoimage_quad_fetcher.cpp 

unix:SOURCES += background_thread_unix.cpp
win32:SOURCES += background_thread_win32.cpp

TARGET= vic_cbdam_base

# --- INSTALL
install_pri.path=$$SHARE_DIR/vic/qmake
install_pri.files=vic_cbdam_base.pri

install_inc.path=$$INCLUDE_DIR/vic/cbdam/base/
install_inc.files=$$HEADERS
install_inc.CONFIG += no_check_exist

target.path=$$LIB_DIR

INSTALLS+= install_pri install_inc target

message(INCLUDE PATH= $$INCLUDEPATH)

#--------------------------------------------------------------------
include (../../../../ratman_post.pri)
#--------------------------------------------------------------------

