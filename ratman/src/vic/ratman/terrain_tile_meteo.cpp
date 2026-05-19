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
#include <vic/ratman/terrain_tile_meteo.hpp>
#include <vic/ratman/qgl_scene_view.hpp>
#include <vic/ratman/terrain_renderable.hpp>
#include <vic/ratman/decorated_terrain_view.hpp>
#include <vic/cbdam/base/terrain_model.hpp>

#include <sl/utility.hpp>
#include <qfile.h>
#include <qtextstream.h>
#include <qstringlist.h>
#include <algorithm>
#include <stdlib.h> // FIXME
#include <vic/ratman/network.hpp>


const int UPDATE_TIME=300000;

namespace ratman {
  
  void terrain_tile_meteo::load_meteo(const std::string& file_name) {

    clear();

    std::string local_dir = sl::pathname_directory(file_name);
    
    std::istream* in_file = network::instance()->istream_open(file_name);
    if (!in_file) {
      std::cerr << "ERROR unable to open input places file" << file_name << std::endl;
    } else {
      int line = 0;
      int id=0;
      while ( in_file->good() ) {
        ++line;
	std::string str; std::getline(*in_file, str);
	if(!str.empty()) {
	  QString tab_separated_line = QString(str.c_str());
	  QStringList site_data = tab_separated_line.split('\t'); 
	  if (site_data.size() != 5) {
	    std::cerr << "ERROR: parse error at " << file_name << ":" << line <<  std::endl;
	  } else {
	    QString name = site_data[0];
	    std::string url = site_data[1].toStdString();
	    double lat = site_data[2].toDouble();
	    double lon = site_data[3].toDouble();
	    double elv = site_data[4].toDouble();
	    
	    // Metric conversion
	    point3d_t location_uvh_k = scene()->terrain_layer()->model()->uvh_from_WGS84_lonlat(point3d_t(lon, lat, elv));
	    point3d_t location_xyz_k = scene()->terrain_layer()->model()->xyz_from_uvh(location_uvh_k);
	    if (vic::geo::srs::spatial_reference_transformation::is_valid(location_xyz_k)) {
	      station_placemarks_.push_back(placemark_render_data(id,
								  point2d_t(location_uvh_k[0], 
									    location_uvh_k[1]),
								  location_xyz_k)); // Approx for sorting
	      ++id;
	      station_urls_.push_back(url);
	      station_data_.push_back(new meteo_data(name));
	      station_thumbnails_updated_.push_back(false);
	      station_thumbnails_.push_back(QImage());
	    } else {
	      std::cerr << "ERROR: cartographic projection error at " << file_name << ":" << line <<  " lon=" << lon << " lat= " << lat << std::endl;
	    }
	  }
	}
      } // while
      
    }

    // Close file
    network::instance()->istream_close(in_file);
  }
  
  void terrain_tile_meteo::on_event_double_click(qgl_scene_view& qgl,
						 std::size_t pl_idx,
						 const point3d_t& pl_xyz,
						 const projective_map3d_t& P,
						 const rigid_body_map3d_t& V) {
    super_t::on_event_double_click(qgl, pl_idx, pl_xyz, P, V);
    qgl.handling_event(station_data_[pl_idx]);
  }

  terrain_tile_meteo::terrain_tile_meteo(decorated_terrain_view* scene,
					 const std::string& name,
					 const std::string& file_name,
					 const double min_distance,
					 const double max_distance,
					 const std::size_t max_render_meteo_count,
					 QImage* icon)
      :
      terrain_billboard_placemarks(scene, name, min_distance, max_distance, max_render_meteo_count, icon) {
    is_causing_occlusion_ = false;
    is_checking_occlusion_ = false;
    load_meteo(file_name);
    timer_ = new QTime();
  }

  terrain_tile_meteo::~terrain_tile_meteo() {
    clear();
  }
 
  void terrain_tile_meteo::clear() {
    for (std::size_t i=0; i<station_data_.size(); ++i) {
      if (station_data_[i]) delete station_data_[i];
      station_data_[i]=0;
    }
    station_data_.clear();
    station_urls_.clear();
    station_thumbnails_.clear();
    station_thumbnails_updated_.clear();
    station_placemarks_.clear();
  }

  vector2d_t terrain_tile_meteo::billboard_pixel_extent(std::size_t /*pl_idx*/, const point3d_t& /*pl_xyz*/, double viewport_width, double viewport_height) const {
    const float lo_x  =   0.0f;
    const float hi_x  =  64.0f;
    const float lo_y  =   0.0f;
    const float hi_y  =  64.0f;
    
    const float dx = (hi_x-lo_x)/viewport_width;
    const float dy = (hi_y-lo_y)/viewport_height;

    return vector2d_t(dx, dy);
  }

  void terrain_tile_meteo::billboard_render(std::size_t i, 
					    const point3d_t& /*pl_xyz*/, 
					    float alpha) {
    if(station_data_[i]->temperature()==NOSIG) return;
    if(!station_thumbnails_updated_[i]) {
      QImage img = station_data_[i]->meteo_icon_image();
      station_thumbnails_[i]=QGLWidget::convertToGLFormat(img.scaled(64, 64, Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
      mutex_.lock(); {
	station_thumbnails_updated_[i]=true;
      } mutex_.unlock();
    }
    const QImage& thumbimg = station_thumbnails_[i];

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexImage2D(GL_TEXTURE_2D, 
                 0, 
                 (thumbimg.hasAlphaChannel() ? 4 : 3), // FIXME 
                 thumbimg.width(), 
                 thumbimg.height(),
                 0,
                 GL_RGBA, 
                 GL_UNSIGNED_BYTE, 
                 thumbimg.bits());
    glDisable(GL_ALPHA_TEST);
    const int texture_width = thumbimg.width();

    glColor4f(1.0f, 1.0f, 1.0f, alpha);
    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(         0.0f,          0.0f, 0.0f); 
    glTexCoord2f(1.0f, 0.0f); glVertex3f(texture_width,          0.0f, 0.0f); 
    glTexCoord2f(1.0f, 1.0f); glVertex3f(texture_width, texture_width, 0.0f); 
    glTexCoord2f(0.0f, 1.0f); glVertex3f(         0.0f, texture_width, 0.0f); 
    glEnd();
  }

  void terrain_tile_meteo::render_self(qgl_scene_view& qgl,
				       occupancy_map_t& occupancy_map,
				       const projective_map3d_t& P,
				       const rigid_body_map3d_t& V,
				       const point3d_t& C) {
    super_t::render_self(qgl, occupancy_map, P, V, C);
  }

  
  void terrain_tile_meteo::async_update_self(const projective_map3d_t& P,
					     const rigid_body_map3d_t& V,
					     const point3d_t& G) {
    super_t::async_update_self(P, V, G);
    if (pending_requests_.empty() && (timer_->isNull() || timer_->elapsed()>UPDATE_TIME) ) {
      //      std::cerr << "posting requests..." << std::endl;
      if(timer_->isNull()) {
	timer_->start();
      } else {
	timer_->restart();
      }
      for(unsigned i=0; i<station_urls_.size(); ++i) {
	//	std::cerr << "posting requests... " << station_urls_[i].c_str() << std::endl;
	http_request *req = new http_request(station_urls_[i].c_str(),i);
	pending_requests_.insert(req);
	req->start();
      }
    } else {
      for (std::set<http_request*>::iterator it = pending_requests_.begin();
	   it != pending_requests_.end();
	   ) {
	std::set<http_request*>::iterator this_it = it;
	++it;
	  
	http_request* request = (*this_it);
	if (!request->isFinished()) {
	  //	  std::cerr << "waiting: ["  << request->id() << "] " << std::endl;
	}  else if (request->error()) {
	  SL_TRACE_OUT(1) << "Error!" << std::endl;
	  delete request;
	  pending_requests_.erase(this_it);
	} else {
	  QStringList lines = request->data().split("\n");
	  if (lines.size() < 3) {
	    std::cerr << "BAD response: " << std::endl;
	    for (std::size_t i=0; i<std::size_t(lines.size()); ++i) {
	      std::cerr << "[" << i << "]: " << lines[i].toStdString() << std::endl;
	    }
	  } else {
	    //	    std::cerr << "FINISHED: ["  << request->id() << "] " << std::endl;	    
	    mutex_.lock(); {
	      station_data_[request->id()]->set_metar(lines[1]);
	      station_data_[request->id()]->decode();
	      station_thumbnails_updated_[request->id()]=false;
	    } mutex_.unlock();
	    //std::cerr << "FINISHED: ["  << station_data_[request->id()].metar().toStdString() << "] " << std::endl;	    
	    //std::cerr << "FINISHED: ["  << station_data_[request->id()].temperature() << "] " << std::endl;	    
	  }
	  delete request;
	  pending_requests_.erase(this_it);
	}
      }
    }
  }

  void terrain_tile_meteo::async_update_render_placemarks_candidates_in(std::vector<placemark_render_data_t>& candidates,
									const projective_map3d_t& /*P*/,
									const rigid_body_map3d_t& V,
									const point3d_t& G) {
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

    for(std::size_t i=0; i<station_placemarks_.size(); ++i) {
      candidates.push_back(station_placemarks_[i]);
#if 0
      point2d_t loc_WGS84_k=station_placemarks_[i].location_uv();
      if (loc_WGS84_k[0]>lookat_lon0 && loc_WGS84_k[0]<lookat_lon1 && 
	  loc_WGS84_k[1]>lookat_lat0 && loc_WGS84_k[1]<lookat_lat1) {		    	  

      }
#endif
    }
  }
   
}
