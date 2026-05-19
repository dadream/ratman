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
#ifndef RATMAN_TERRAIN_PLACENAMES_HPP
#define RATMAN_TERRAIN_PLACENAMES_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/ratman/terrain_billboard_placemarks.hpp>
#include <vic/ratman/geonames_service.hpp>
#include <qimage.h>
#include <vector>
#include <map>

namespace ratman {

  /**
   * Class derived from terrain_billboard_placemarks. 
   * It manages a cache of geonames_entry. 
   * It access geonames information through a geoname_service
   * which asks data of a query 2d  box from a server
   * Available visible data is displayed as billboard.
   */
  class terrain_placenames: public terrain_billboard_placemarks {
  public:
    typedef terrain_placenames this_t;
    typedef terrain_billboard_placemarks super_t;
    typedef std::pair<std::size_t, int32_t> id_fadestatus_pair_t;     
  protected:
    QImage                            label_gl_img_;
    double                            label_size_;
    vector4f_t                        label_color_;

    geonames_search*                  geonames_service_;

    std::size_t tile_count_lon_;
    std::size_t tile_count_lat_;
    double      tile_width_;

    std::map< point2i_t, std::vector<geonames_entry> > placemarks_tiled_cache_;

  public:

    terrain_placenames(decorated_terrain_view* scene,
		       const std::string& name,
		       geonames_search* service,
		       const double label_size,
		       const vector4f_t& label_color,
		       const double min_distance,
		       const double max_distance,
		       const std::size_t max_render_placenames_count = 50,
		       QImage* icon = 0);

    virtual ~terrain_placenames();

    virtual vector2d_t billboard_pixel_extent(std::size_t id,  
					      const point3d_t& pl_xyz,
					      double viewport_width, 
					      double viewport_height) const;
    virtual void       billboard_render(std::size_t id,   
					const point3d_t& pl_xyz,
					float alpha);

  public:
    
    virtual void on_event_double_click(qgl_scene_view& qgl,
				       std::size_t pl_idx,
				       const point3d_t& pl_xyz,
				       const projective_map3d_t& P,
				       const rigid_body_map3d_t& V);

    virtual void on_event_single_click(qgl_scene_view& qgl,
				       std::size_t pl_idx,
				       const point3d_t& pl_xyz,
				       const projective_map3d_t& P,
				       const rigid_body_map3d_t& V);

    virtual void on_event_mouse_move(qgl_scene_view& qgl,
				     std::size_t pl_idx,
				     const point3d_t& pl_xyz,
				     const projective_map3d_t& P,
				     const rigid_body_map3d_t& V);

 inline void async_clear_cache(){
   placemarks_tiled_cache_.clear();
 }


  protected:

    void init_tiling_parameters();

    void cache_cleanup(const point2d_t& eye_lonlat);

    const geonames_entry* cache_entry(size_t id) const;

    void cache_update(const projective_map3d_t& P,
		      const rigid_body_map3d_t& V);

    /// Asynchronously update the object. Called in a separate thread.
    virtual void async_update_render_placemarks_candidates_in(std::vector<placemark_render_data_t>& candidates,
							      const projective_map3d_t& P,
							      const rigid_body_map3d_t& V,
							      const point3d_t& G);

    std::size_t tile_cache_entry_id(int i, int j, int k) const;
    std::pair<point2i_t,std::size_t> tile_cache_entry_indexes(std::size_t id) const;
    aabox2d_t query_box(int i, int j) const;

  };
  
}

#endif
