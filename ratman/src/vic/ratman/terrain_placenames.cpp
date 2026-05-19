//+++HDR+++
//======================================================================
//   This file is part of the RATMAN software framework.
//   Copyright (C) 2009 by CRS4, Pula, Italy.
//
//   For more information, visit the CRS4 Visual Computing Group
//   web pages at http://www.crs4.it/vic/
//
//   This file may be used under the terms of the GNU General Public
//   License as published by the Free Software Foundation and appearing
//   in the file LICENSE included in the packaging of this file.
//
//   CRS4 reserves all rights not expressly granted herein.
//  
//   This file is provided AS IS with NO WARRANTY OF ANY KIND, 
//   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS 
//   FOR A PARTICULAR PURPOSE.
//
//======================================================================
//---HDR---//
#include <vic/ratman/terrain_placenames.hpp>
#include <vic/ratman/qgl_scene_view.hpp>
#include <vic/ratman/terrain_renderable.hpp>
#include <vic/ratman/decorated_terrain_view.hpp>
#include <vic/cbdam/base/terrain_model.hpp>

#include <vic/geo/srs/spatial_reference.hpp>

#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <algorithm>
#include <vic/ratman/network.hpp>
#include <vic/ratman/browser.hpp>

namespace ratman {
    
  terrain_placenames::terrain_placenames(decorated_terrain_view* scene,
					 const std::string& name,
					 geonames_search* service,
					 const double label_size,
					 const vector4f_t& label_color,
					 const double min_distance,
					 const double max_distance,
					 const std::size_t max_render_placenames_count,
					 QImage* icon)
      :
      terrain_billboard_placemarks(scene, name, min_distance, max_distance, max_render_placenames_count, icon),
      label_size_(label_size),
      label_color_(label_color) {
    // Event customization
    is_handling_mouse_move_ = true;

    // find best texture size
    int texture_size = 16;
    while (2*texture_size <= label_size && 2*texture_size <= icon_->width()) texture_size*=2;
    label_gl_img_=QGLWidget::convertToGLFormat(icon_->scaled(texture_size, texture_size, 
							     Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
    
    geonames_service_ = service;

    init_tiling_parameters();
  }

  terrain_placenames::~terrain_placenames() {
    delete geonames_service_;
    geonames_service_ = 0;
  }

  void terrain_placenames::on_event_double_click(qgl_scene_view& qgl,
						 std::size_t pl_idx,
						 const point3d_t& pl_xyz,
						 const projective_map3d_t& P,
						 const rigid_body_map3d_t& V) {
    // We have the mutex, no need to lock
    super_t::on_event_double_click(qgl, pl_idx, pl_xyz, P, V);

    //    std::cerr << "terrain_placenames::on_event_double_click():pos " << pl_xyz  << std::endl;

    const geonames_entry* e = cache_entry(pl_idx);
 
    if (e && e->url() != "") {
      browser::open_url(e->url().c_str());
    }
  }

  void terrain_placenames::on_event_single_click(qgl_scene_view& qgl,
						 std::size_t pl_idx,
						 const point3d_t& pl_xyz,
						 const projective_map3d_t& P,
						 const rigid_body_map3d_t& V) {
    on_event_double_click(qgl, pl_idx, pl_xyz, P, V);
  }

  void terrain_placenames::on_event_mouse_move(qgl_scene_view& qgl,
					       std::size_t pl_idx,
					       const point3d_t& /*pl_xyz*/,
					       const projective_map3d_t& /*P*/,
					       const rigid_body_map3d_t& /*V*/) {
    const geonames_entry* e = cache_entry(pl_idx);
    if (e && e->url() != "") {
      // = std::cerr << "SETCURSOR" << std::endl;
      qgl.setCursor(QCursor(Qt::PointingHandCursor));
    }
  }
 
  vector2d_t terrain_placenames::billboard_pixel_extent(std::size_t pl_idx,
							const point3d_t& /*pl_xyz*/,
							double viewport_width, 
							double viewport_height) const {
    const geonames_entry* e = cache_entry(pl_idx);
    if (!e) {
      SL_TRACE_OUT(1) << "NO ENTRY FOR INDEX " << pl_idx << std::endl;
      return vector2d_t();
    } else {
      const std::string& pl_label = e->name();
      const double default_font_scale = shared_3dfont().scale_x(); // FIXME

      float lo_x, lo_y, hi_x, hi_y;
      shared_3dfont().string_bbox(pl_label.c_str(), lo_x, lo_y, hi_x, hi_y);
      const float dx = (1.5+(hi_x-lo_x)/default_font_scale)*label_size_/viewport_width;
      const float dy = (0.0+(hi_y-lo_y)/default_font_scale)*label_size_/viewport_height;

      return vector2d_t(dx, dy);
    }
  }

  void terrain_placenames::billboard_render(std::size_t id,   
					    const point3d_t& /*pl_xyz*/,
					    float alpha) {
    const geonames_entry* e = cache_entry(id);
    if (!e) {
      SL_TRACE_OUT(1) << "NO ENTRY FOR INDEX " << id << std::endl;
    } else {
      const std::string& pl_label = e->name();
      const double default_font_scale = shared_3dfont().scale_x();
      float lo_x, lo_y, hi_x, hi_y;
      shared_3dfont().string_bbox(pl_label.c_str(), lo_x, lo_y, hi_x, hi_y);

      glEnable(GL_TEXTURE_2D );
      glBindTexture(GL_TEXTURE_2D, 0);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexImage2D(GL_TEXTURE_2D, 
		   0, 
		   (label_gl_img_.hasAlphaChannel() ? 4 : 3), // FIXME 
		   label_gl_img_.width(), 
		   label_gl_img_.height(),
		   0,
		   GL_RGBA, 
		   GL_UNSIGNED_BYTE, 
		   label_gl_img_.bits());
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(GL_GREATER, 0);
      const int texture_width = label_gl_img_.width();
      glColor4f(1.0f, 1.0f, 1.0f, alpha);
      glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f); glVertex3f(0.0f,          -0.5f*texture_width, 0.0f); 
      glTexCoord2f(1.0f, 0.0f); glVertex3f(texture_width, -0.5f*texture_width, 0.0f); 
      glTexCoord2f(1.0f, 1.0f); glVertex3f(texture_width,  0.5f*texture_width, 0.0f); 
      glTexCoord2f(0.0f, 1.0f); glVertex3f(0.0f,           0.5f*texture_width, 0.0f); 
      glEnd();
      glDisable(GL_ALPHA_TEST);
      
      glColor4f(label_color_[0], label_color_[1], label_color_[2], alpha);
      glTranslatef(1.5f*texture_width, 0.0f, 0.0f);
      glScaled(label_size_/default_font_scale,  label_size_/default_font_scale, 1.0);
      glPushMatrix();
      {
	shared_3dfont().write(pl_label.c_str());
      }
      glPopMatrix();
      glDisable( GL_TEXTURE_2D);


      // Fill z for all bbox to avoid crossing of text
      glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
      glBegin(GL_QUADS);
      glVertex3f(lo_x, lo_y, 0.0f);
      glVertex3f(hi_x, lo_y, 0.0f);
      glVertex3f(hi_x, hi_y, 0.0f);
      glVertex3f(lo_x, hi_y, 0.0f);
      glEnd();
    }
  }
 
  void terrain_placenames::init_tiling_parameters() {
    const double R = 6370000; // FIXME Hardcoded Earth radius - to make it work also for planar 

    double tile_width = 0.0;
    double factor = max_distance_ / R;
    if (factor < 1) {
      tile_width = rad2deg(std::abs(std::asin(factor)*2.0));
    } else {
      tile_width = 180.0;
    }
    tile_width = (180.0 / (int)(180 / tile_width));
    if (tile_width < 0.1) tile_width = 0.1;

    tile_count_lon_ = std::size_t(360.0 / tile_width + 0.5); 
    if (tile_count_lon_ % 2) ++tile_count_lon_;

    tile_count_lat_ = tile_count_lon_/2;

    tile_width_ = 360.0/tile_count_lon_;

    SL_TRACE_OUT(1) << "TILES: " << tile_count_lon_ << " deg = " << tile_width_ << std::endl;
  }

  const std::size_t MAX_ENTRY_COUNT_PER_TILE = 4096; // FIXME

  std::size_t terrain_placenames::tile_cache_entry_id(int i, int j, int k) const {
    //const std::size_t Ni = tile_count_lon_;
    const std::size_t Nj = tile_count_lat_;
    const std::size_t Nk = MAX_ENTRY_COUNT_PER_TILE;
    
    return 
      i * (Nj * Nk) + 
      j * (Nk) + 
      k;
  }
  
  std::pair<point2i_t,std::size_t> terrain_placenames::tile_cache_entry_indexes(std::size_t id) const {
    //const std::size_t Ni = tile_count_lon_;
    const std::size_t Nj = tile_count_lat_;
    const std::size_t Nk = MAX_ENTRY_COUNT_PER_TILE;
    
    std::size_t i = id / (Nj*Nk);
    std::size_t j = (id - i * (Nj*Nk)) / (Nk);
    std::size_t k = (id - i * (Nj*Nk) - j * Nk);

    return std::make_pair(point2i_t(int(i), int(j)), k);
  }

  aabox2d_t terrain_placenames::query_box(int i, int j) const {
    const double l = tile_width_;
    return aabox2d_t(point2d_t(-180.0+double(i+0)*l, -90.0+double(j+0)*l),
		     point2d_t(-180.0+double(i+1)*l, -90.0+double(j+1)*l));
  }

  const geonames_entry* terrain_placenames::cache_entry(std::size_t id) const {
    // Assume cache locked
    const geonames_entry* result = 0;

    std::pair<point2i_t,std::size_t> idx = tile_cache_entry_indexes(id);

    std::map< point2i_t, std::vector<geonames_entry> >::const_iterator it = placemarks_tiled_cache_.find(idx.first);
    if (it !=placemarks_tiled_cache_.end() && (idx.second < it->second.size())) {
      result = &it->second[idx.second];
    }

    return result;
  }
  
  void terrain_placenames::cache_cleanup(const point2d_t& /*eye_lonlat*/) {
    // FIXME NO CLEANUP
    SL_TRACE_OUT(1) << "NOT IMPLEMENTED" << std::endl;
    mutex_.lock();
    {
      // FIXME
    }
    mutex_.unlock();
  }

  /// Asynchronously update the object. Called in a separate thread.
  void terrain_placenames::async_update_render_placemarks_candidates_in(std::vector<placemark_render_data_t>& candidates,
									const projective_map3d_t& /*P*/,
									const rigid_body_map3d_t& V,
									const point3d_t& G) {
    candidates.clear();
  
    matrix4x4d_t IV = V.inverse().as_matrix();
    point3d_t    eye_xyz = point3d_t(IV(0,3), IV(1,3), IV(2,3));
    point3d_t    lookat_xyz = G;

    point3d_t    lookat_WGS84_lonlat = scene()->terrain_layer()->model()->WGS84_lonlat_from_xyz(lookat_xyz);

    double       lookat_distance = eye_xyz.distance_to(lookat_xyz);
    const double R = 6370000; // FIXME Hardcoded Earth radius - to make it work also for planar 

    double lookat_tile_width = 0.0;
    double factor = lookat_distance / R;
    if (factor < 1.0) {
      lookat_tile_width = std::max(0.01, rad2deg(std::abs(std::asin(factor)*2.0)));
    } else {
      lookat_tile_width = 180.0;
    }

    double lookat_lon0 = lookat_WGS84_lonlat[0]-lookat_tile_width;
    double lookat_lon1 = lookat_WGS84_lonlat[0]+lookat_tile_width;
    double lookat_lat0 = lookat_WGS84_lonlat[1]-lookat_tile_width;
    double lookat_lat1 = lookat_WGS84_lonlat[1]+lookat_tile_width;
     
    // FIXME OPTIMIZE
    for (std::size_t i = 0; i<tile_count_lon_; ++i) {
      double lon0 = -180.0 + (i+0)*tile_width_;
      double lon1 = -180.0 + (i+1)*tile_width_;
      if (lon0<lookat_lon1 && lon1>lookat_lon0) {
	for (std::size_t j = 0; j<tile_count_lat_; ++j) {
	  double lat0 = -90.0 + (j+0)*tile_width_;
	  double lat1 = -90.0 + (j+1)*tile_width_;
	  if (lat0<lookat_lat1 && lat1>lookat_lat0) {
	    point2i_t ij = point2i_t(i, j);
	    std::map< point2i_t, std::vector<geonames_entry> >::iterator it = placemarks_tiled_cache_.find(ij);
	    if (it ==placemarks_tiled_cache_.end()) {
	      // NOT IN CACHE, MAKE A QUERY
	      aabox2d_t qb = aabox2d_t(point2d_t(lon0, lat0),
				       point2d_t(lon1, lat1)); // query_box(i,j);
	      geonames_service_->search_box(qb);
	      if (geonames_service_->last_search_ok()) {
		mutex_.lock();
		placemarks_tiled_cache_[ij] = geonames_service_->last_search_results(); // COPY
		mutex_.unlock();
		it = placemarks_tiled_cache_.find(ij);
	      } else {
		SL_TRACE_OUT(-1) << "Search error!!" << std::endl;
	      }
	    }
	    if (it!=placemarks_tiled_cache_.end()) {
	      // Update candidates
	      const std::vector<geonames_entry>& entries = it->second;

	      // Refine 
	      for (std::size_t k=0; k<entries.size(); ++k) {
		point3d_t loc_WGS84_k = entries[k].location();
		if (loc_WGS84_k[0]>lookat_lon0 && loc_WGS84_k[0]<lookat_lon1 && 
		    loc_WGS84_k[1]>lookat_lat0 && loc_WGS84_k[1]<lookat_lat1) {		    
		  point3d_t location_uvh_k = scene()->terrain_layer()->model()->uvh_from_WGS84_lonlat(loc_WGS84_k);
		  point3d_t location_xyz_k = scene()->terrain_layer()->model()->xyz_from_uvh(location_uvh_k);
		  candidates.push_back(placemark_render_data(tile_cache_entry_id(i, j, k),
							     point2d_t(location_uvh_k[0], 
								       location_uvh_k[1]),
							     location_xyz_k)); // Approx for sorting
		}
	      }
	    }	    
	  }
	}
      }
    }  

    SL_TRACE_OUT(1) << "FOUND: " << candidates.size() << std::endl;

    // Sort placenames by distance to eye
    std::sort(candidates.begin(),
	      candidates.end(),
	      placemark_render_data_less_by_xyz_distance(lookat_xyz));
    if (candidates.size() > max_render_placemarks_count_) {
      candidates.resize(max_render_placemarks_count_);
    }

    cache_cleanup(point2d_t(lookat_WGS84_lonlat[0],
			    lookat_WGS84_lonlat[1]));
  }

}
