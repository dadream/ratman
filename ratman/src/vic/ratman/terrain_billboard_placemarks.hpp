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
#ifndef RATMAN_TERRAIN_BILLBOARD_PLACEMARKS_HPP
#define RATMAN_TERRAIN_BILLBOARD_PLACEMARKS_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/ratman/active_renderable.hpp>
#include <vector>

namespace ratman {

  /**
   * Information for a placemark: id, location and fade status
   * Fade is used to eliminate popping artifacts. 
   * Placemark rendering life: fadein - render - fadeout using alpha.
   */
  class placemark_render_data {
  protected:
    std::size_t id_;
    point2d_t   location_uv_;
    point3d_t   location_xyz_;
    point3d_t   render_location_xyz_;
    int32_t     fadestatus_;

  public:
    placemark_render_data(std::size_t id = 0,
			  const point2d_t& location_uv = point2d_t(),
			  const point3d_t& location_xyz = point3d_t(),
			  int32_t fadestatus = 0) :
      id_(id), location_uv_(location_uv), location_xyz_(location_xyz), render_location_xyz_(location_xyz), fadestatus_(fadestatus) {
    }
    
    inline std::size_t id() const { return id_; }
    inline const point2d_t& location_uv() const { return location_uv_; }
    inline const point3d_t& location_xyz() const { return location_xyz_; }
    inline const point3d_t& render_location_xyz() const { return render_location_xyz_; }
    inline int32_t fadestatus() const { return fadestatus_; }

    inline void set_location_xyz(const point3d_t& xyz) {
      location_xyz_ = xyz;
    }
    inline void set_render_location_xyz(const point3d_t& xyz) {
      render_location_xyz_ = xyz;
    }
    inline void set_fadestatus(const int32_t& x) {
      fadestatus_ = x;
    }
  };

  /**
   * Operator to sort placemarks by distcance to viewpoint.
   * Placemarks closer to the viewpoint are rendered first
   */
  class placemark_render_data_less_by_xyz_distance {
  protected:
    point3d_t eye_;
  public:
    placemark_render_data_less_by_xyz_distance(const point3d_t& eye) :
      eye_(eye) {
    }
      
    bool operator() (const placemark_render_data& x,
		     const placemark_render_data& y) const {
      return eye_.distance_squared_to(x.render_location_xyz()) < eye_.distance_squared_to(y.render_location_xyz());
    }
  };


  /**
   * Base class for rendering a billboard set.
   * Billoboards always face the viewer.
   * A billboard is rendered only if it is between a min and a max distance.
   * An occupancy map is used to properly distribute placemarks to be rendered
   */
  class terrain_billboard_placemarks: public active_renderable {
  public:
    typedef placemark_render_data placemark_render_data_t;
  protected:
    double                            min_distance_;
    double                            max_distance_;
    std::size_t                       max_render_placemarks_count_;
    bool                              is_checking_occlusion_;
    bool                              is_causing_occlusion_;
    bool                              is_handling_mouse_move_;
    std::size_t			      update_counter_;

  protected:
    // Placemarks to draw
    std::vector<placemark_render_data_t> render_placemarks_;
    bool                                 are_render_placemarks_new_;
  public:

    terrain_billboard_placemarks(decorated_terrain_view* scene,
				 const std::string& name,
				 const double min_distance,
				 const double max_distance,
				 const std::size_t max_render_placemarks_count = 40,
				 QImage* icon = 0);

    virtual ~terrain_billboard_placemarks();

    bool is_causing_occlusion() const;
    bool is_checking_occlusion() const;

    void set_causing_occlusion(bool x);
    void set_checking_occlusion(bool x);
    
  protected: // Helpers

    static double apparent_scale_at_distance(const projective_map3d_t& P, double d);

    static bool occupancy_map_optionally_test_and_set(bool do_test,
                                                      bool do_set,
                                                      occupancy_map_t& occupancy_map,
                                                      const projective_map3d_t& PV,
                                                      const point3d_t& pos,
                                                      const vector2d_t& wh);
    
    bool occupancy_map_test(occupancy_map_t& occupancy_map,
                            const projective_map3d_t& PV,
                            const point3d_t& pos,
                            const vector2d_t& wh) const;
    
    bool occupancy_map_test_and_set(occupancy_map_t& occupancy_map,
                                    const projective_map3d_t& PV,
                                    const point3d_t& pos,
                                    const vector2d_t& wh) const;

    bool occupancy_map_set(occupancy_map_t& occupancy_map,
                           const projective_map3d_t& PV,
                           const point3d_t& pos,
                           const vector2d_t& wh) const;

    virtual vector2d_t billboard_pixel_extent(std::size_t id, 
					      const point3d_t& pl_xyz,
					      double viewport_width, 
					      double viewport_height) const = 0;
    virtual void       billboard_render(std::size_t id,  
					const point3d_t& pl_xyz,
					float alpha) = 0;
    
  protected:

    void placemarks_update_locations_xyz(const qgl_scene_view& qgl,
					 const point3d_t& eye);

  public:

    /// Handle picking and return true if handled
    virtual bool on_event_self(qgl_scene_view& qgl,
			       QEvent* e,
			       const projective_map3d_t& P,
			       const rigid_body_map3d_t& V);
     
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
							      const point3d_t& G) = 0;

    /// Asynchronously update the object. Called in a separate thread.
    virtual void async_update_render_placemarks(const std::vector<placemark_render_data_t>& candidates,
						const projective_map3d_t& P,
						const rigid_body_map3d_t& V,
						const point3d_t& G);
  };
  
}

#endif
