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
#ifndef RATMAN_TERRAIN_TILE_METEO_HPP
#define RATMAN_TERRAIN_TILE_METEO_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/ratman/terrain_billboard_placemarks.hpp>
#include <vic/ratman/meteo_data.hpp>
#include <vic/ratman/http_request.hpp>

#include <QTime>
#include <QImage>
#include <vector>
#include <map>
#include <set>

namespace ratman {

  /**
   * Vector of all meteo billboards (with synthetic graphic meteo info).
   * Manages connections to read meteo data, and handle
   * billboard eteo data display.
   */
  class terrain_tile_meteo: public terrain_billboard_placemarks {
  public:
    typedef terrain_tile_meteo this_t;
    typedef terrain_billboard_placemarks super_t;
    typedef std::pair<std::size_t, int32_t> id_fadestatus_pair_t;     
  protected:
    QImage                            label_gl_img_;
    
    std::vector<placemark_render_data_t> station_placemarks_;
    std::vector<bool>                 station_thumbnails_updated_;
    std::vector<QImage>               station_thumbnails_; // map id->thumbnails
    std::vector<std::string>          station_urls_;
    std::vector<meteo_data*>          station_data_;     

    std::set<http_request*> pending_requests_;
    QTime *timer_;

  protected:
    void load_meteo(const std::string& file_name);
    void clear();
  public:

    terrain_tile_meteo(decorated_terrain_view* scene,
		       const std::string& name,
		       const std::string& file_name,
		       const double min_distance,
		       const double max_distance,
		       const std::size_t max_render_meteo_count = 5,
		       QImage* icon = 0);

    virtual ~terrain_tile_meteo();

    virtual vector2d_t billboard_pixel_extent(std::size_t id, 
					      const point3d_t& pl_xyz,
					      double viewport_width, 
					      double viewport_height) const;

    virtual void       billboard_render(std::size_t i, 
					const point3d_t& pl_xyz, 
					float alpha);

  public:
       
    virtual void on_event_double_click(qgl_scene_view& qgl,
				       std::size_t pl_idx,
				       const point3d_t& pl_xyz,
				       const projective_map3d_t& P,
				       const rigid_body_map3d_t& V);

    /// Render the hierarchy object rooted at this
    virtual void render_self(qgl_scene_view& qgl,
                             occupancy_map_t& occupancy_map,
			     const projective_map3d_t& P,
                             const rigid_body_map3d_t& V,
			     const point3d_t& C);

    /// Asynchronously update the object. Called in a separate thread.
    virtual void async_update_self(const projective_map3d_t& P,
                                   const rigid_body_map3d_t& V,
				   const point3d_t& G);
 
   /// Asynchronously update the object. Called in a separate thread.
    virtual void async_update_render_placemarks_candidates_in(std::vector<placemark_render_data_t>& candidates,
							      const projective_map3d_t& P,
							      const rigid_body_map3d_t& V,
							      const point3d_t& G);

  };
  
}

#endif
