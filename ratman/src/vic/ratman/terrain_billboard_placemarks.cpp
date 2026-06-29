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
#include <vic/ratman/terrain_billboard_placemarks.hpp>
#include <vic/ratman/qgl_scene_view.hpp>
#include <vic/cbdam/base/terrain_model.hpp>
#include <vic/cbdam/base/background_thread.hpp>
#include <QMouseEvent>
#include <algorithm>

#include "placemark_icon.xpm" 

namespace ratman {
    
  terrain_billboard_placemarks::terrain_billboard_placemarks(decorated_terrain_view* scene,
							     const std::string& name,
							     const double min_distance,
							     const double max_distance,
							     const std::size_t max_render_placemarks_count,
							     QImage* icon) :
    active_renderable(scene, name, icon ? icon : new QImage(placemark_icon_xpm)),
    min_distance_(min_distance),
    max_distance_(max_distance),
    max_render_placemarks_count_(max_render_placemarks_count),
    is_checking_occlusion_(true),
    is_causing_occlusion_(true),
    is_handling_mouse_move_(false),
    are_render_placemarks_new_(true)
  {
    update_counter_ = rand();
  }

  terrain_billboard_placemarks::~terrain_billboard_placemarks() {
  }

  bool terrain_billboard_placemarks::is_causing_occlusion() const {
    return is_causing_occlusion_;
  }

  bool terrain_billboard_placemarks::is_checking_occlusion() const {
    return is_checking_occlusion_;
  }

  void terrain_billboard_placemarks::set_causing_occlusion(bool x) {
    is_causing_occlusion_ = x;
  }
  
  void terrain_billboard_placemarks::set_checking_occlusion(bool x) {
    is_checking_occlusion_ = x;
  }

  double terrain_billboard_placemarks::apparent_scale_at_distance(const projective_map3d_t& P, double d) {
    vector4d_t pa = vector4d_t(0.0, -0.5, d, 1.0);
    vector4d_t pb = vector4d_t(0.0,  0.5, d, 1.0);
    vector4d_t pa1 = P.as_matrix() * pa;
    vector4d_t pb1 = P.as_matrix() * pb;
    return 
      (point3d_t(pa1[0]/pa1[3], pa1[1]/pa1[3], pa1[2]/pa1[3]) -
       point3d_t(pb1[0]/pb1[3], pb1[1]/pb1[3], pb1[2]/pb1[3])).two_norm();
  }

  
  bool terrain_billboard_placemarks::occupancy_map_optionally_test_and_set(bool do_test,
									   bool do_set,
									   occupancy_map_t& occupancy_map,
									   const projective_map3d_t& PV,
									   const point3d_t& pos,
									   const vector2d_t& wh) {
    vector4d_t posh = PV.as_matrix() * vector4d_t(pos[0], pos[1], pos[2], 1.0);
    point2d_t lo = point2d_t(0.5+0.5*posh[0]/posh[3], 0.5+0.5*posh[1]/posh[3]);

    int sz_x = int(occupancy_map.extent()[0]);
    int sz_y = int(occupancy_map.extent()[1]);
    
    int lo_x = int(lo[0]*sz_x);
    int lo_y = int(lo[1]*sz_y);
    if (lo_x >= sz_x || lo_y >= sz_y) {
      return false; // out of bounds
    } else {
      int hi_x = int((lo[0]+wh[0])*sz_x);
      int hi_y = int((lo[1]+wh[1])*sz_y);
      if (hi_x <= 0 || hi_y<= 0) {
        return false; // out of bounds
      } else {
        lo_x = std::max(lo_x, 0);
        lo_y = std::max(lo_y, 0);
        hi_x = std::min(hi_x, sz_x);
        hi_y = std::min(hi_y, sz_y);

        if (do_test) {
          for (int x=lo_x; x<hi_x; ++x) {
            for (int y=lo_y; y<hi_y; ++y) {
              if (occupancy_map(x,y)) return false;
            }
          }
        }

        if (do_set) {
          // Was free, mark it occupied
          for (int x=lo_x; x<hi_x; ++x) {
            for (int y=lo_y; y<hi_y; ++y) {
              occupancy_map(x,y) = true;
            }
          }
        }
        
        return true; // visible!
      }
    }
  }
  
  bool terrain_billboard_placemarks::occupancy_map_test(occupancy_map_t& occupancy_map,
							const projective_map3d_t& PV,
							const point3d_t& pos,
							const vector2d_t& wh) const {
    return occupancy_map_optionally_test_and_set(is_checking_occlusion_, false, occupancy_map, PV, pos, wh);
  }

  bool terrain_billboard_placemarks::occupancy_map_test_and_set(occupancy_map_t& occupancy_map,
								const projective_map3d_t& PV,
								const point3d_t& pos,
								const vector2d_t& wh) const {
    return occupancy_map_optionally_test_and_set(is_checking_occlusion_, is_causing_occlusion_, occupancy_map, PV, pos, wh);
  }
  
  bool terrain_billboard_placemarks::occupancy_map_set(occupancy_map_t& occupancy_map,
						       const projective_map3d_t& PV,
						       const point3d_t& pos,
						       const vector2d_t& wh) const {
    return occupancy_map_optionally_test_and_set(false, is_causing_occlusion_, occupancy_map, PV, pos, wh);
  }


  void terrain_billboard_placemarks::placemarks_update_locations_xyz(const qgl_scene_view& qgl,
								     const point3d_t& eye) {
    mutex_.lock();
    {
      // Recompute locations based on current terrain
      for (std::size_t i=0; i<render_placemarks_.size(); ++i) {
 	point2d_t pl_uv = render_placemarks_[i].location_uv();

	point3d_t render_pl_xyz = render_placemarks_[i].render_location_xyz();
	point3d_t pl_xyz = render_placemarks_[i].location_xyz();
	if (render_pl_xyz == pl_xyz || (rand() % 5 == 0)) {
	  // Never updated or probabilistically updated a while ago:
	  // compute elevation from terrain
	  // ... 
	  // 
	  std::pair<bool, double> r = qgl.terrain()->current_representation_ground_elevation_from_uv(pl_uv);
	  double pl_h = (!r.first) ? 0.0 : r.second;
	  pl_h += 10.0; // offset above terrain in m
	  pl_xyz = qgl.terrain()->xyz_from_uvh(point3d_t(pl_uv[0], pl_uv[1], pl_h));
	  render_placemarks_[i].set_location_xyz(pl_xyz);
	  render_pl_xyz = pl_xyz;
	}

	// Offset toward camera
	render_pl_xyz = pl_xyz + 0.03 *(eye-pl_xyz);           // move slightly toward camera 

	render_placemarks_[i].set_render_location_xyz(render_pl_xyz);
      }

      // Sort placenames by distance to eye
      std::sort(render_placemarks_.begin(),
		render_placemarks_.end(),
		placemark_render_data_less_by_xyz_distance(eye));

      are_render_placemarks_new_ = false;
    }
    mutex_.unlock();
  }
  
  bool terrain_billboard_placemarks::on_event_self(qgl_scene_view& qgl,
						   QEvent* e,
						   const projective_map3d_t& P,
						   const rigid_body_map3d_t& V) {
    bool result = false;

    // Handle double click
    double viewport_width  = qgl.width();
    double viewport_height = qgl.height();

    bool is_double_click = (e->type() == QEvent::MouseButtonDblClick);
    bool is_single_click = (e->type() == QEvent::MouseButtonPress);
    bool is_mouse_move   = (is_handling_mouse_move_ && (e->type() == QEvent::MouseMove));
    if (is_double_click || is_single_click || is_mouse_move) {
      QMouseEvent *me = dynamic_cast<QMouseEvent *>(e);      
      double pointer_x = me->x();
      double pointer_y = qgl.height()-1-me->y();

      matrix4x4d_t PV = (P*V).as_matrix();
      matrix4x4d_t C = V.inverse().as_matrix();
      point3d_t    camera_eye = point3d_t(C(0,3), C(1,3), C(2,3));
      
      mutex_.lock();
      {

	// otherwise use last rendering order
	if (are_render_placemarks_new_) {
	  placemarks_update_locations_xyz(qgl, camera_eye);
	}

	// Pick
	for (std::size_t i=0; i<render_placemarks_.size() && !result; ++i) {
	  const std::size_t pl_idx        = render_placemarks_[i].id();
	  const int32_t     pl_fadestatus = render_placemarks_[i].fadestatus();
	  const point3d_t&  pl_pos        = render_placemarks_[i].render_location_xyz();

	  if (pl_fadestatus == 100) { 
	    // Project on screen and check if picked
	    const vector4d_t posh = PV * vector4d_t(pl_pos[0], pl_pos[1], pl_pos[2], 1.0);

	    const point2d_t  pl_lo = point2d_t((0.5+0.5*posh[0]/posh[3])*viewport_width, 
					       (0.5+0.5*posh[1]/posh[3])*viewport_height);	
	    const vector2d_t pl_wh = billboard_pixel_extent(pl_idx, pl_pos, viewport_width, viewport_height);
	    const point2d_t  pl_hi  = pl_lo + vector2d_t(pl_wh[0]*viewport_width, pl_wh[1]*viewport_height);
	    
	    // Fully visible
	    if ((pl_lo[0] <= pointer_x && pointer_x <= pl_hi[0]) &&
		(pl_lo[1] <= pointer_y && pointer_y <= pl_hi[1])) {
	      // clicked!
	      point2d_t pl_uv = render_placemarks_[i].location_uv();
	      std::pair<bool, double> r = qgl.terrain()->current_representation_ground_elevation_from_uv(pl_uv);
	      std::size_t alt_offset = 500;
	      point3d_t actual_pl_pos = qgl.terrain()->xyz_from_uvh(point3d_t(pl_uv[0], pl_uv[1], r.second+alt_offset));

	      if (is_double_click) {
		on_event_double_click(qgl, pl_idx, actual_pl_pos, P, V);
	      } else if (is_single_click) {
		on_event_single_click(qgl, pl_idx, actual_pl_pos, P, V);
	      } else if (is_mouse_move) {
		on_event_mouse_move(qgl, pl_idx, actual_pl_pos, P, V);
	      }		
	      result = true;
	    } 
	  }
	}
      }
      mutex_.unlock();    
    }

    return result;
  }

  void terrain_billboard_placemarks::on_event_double_click(qgl_scene_view& qgl,
							   std::size_t /*pl_idx*/,
							   const point3d_t& pl_xyz,
							   const projective_map3d_t& /*P*/,
							   const rigid_body_map3d_t& /*V*/) {
    //std::cerr << "Placemark #" << pl_idx << " double click!" << std::endl;
    //    std::cerr << "terrain_billboard_placemarks::on_event_double_click() pos:" << pl_xyz << std::endl;
    qgl.goto_location(pl_xyz);
  }

  void terrain_billboard_placemarks::on_event_single_click(qgl_scene_view& /*qgl*/,
							   std::size_t /*pl_idx*/,
							   const point3d_t& /*pl_xyz*/,
							   const projective_map3d_t& /*P*/,
							   const rigid_body_map3d_t& /*V*/) {
    //std::cerr << "Placemark #" << pl_idx << " single click!" << std::endl;
  }

  void terrain_billboard_placemarks::on_event_mouse_move(qgl_scene_view& /*qgl*/,
							 std::size_t /*pl_idx*/,
							 const point3d_t& /*pl_xyz*/,
							 const projective_map3d_t& /*P*/,
							 const rigid_body_map3d_t& /*V*/) {
    //std::cerr << "Placemark #" << pl_idx << " move over" << std::endl;
  }

  void terrain_billboard_placemarks::render_self(qgl_scene_view& qgl,
						 occupancy_map_t& occupancy_map,
						 const projective_map3d_t& P,
						 const rigid_body_map3d_t& V,
						 const point3d_t& C) {
    double viewport_width = qgl.width();
    double viewport_height  = qgl.height();

    matrix4x4d_t PV = (P*V).as_matrix();
    matrix4x4d_t IV = V.inverse().as_matrix();
    point3d_t    camera_eye = point3d_t(IV(0,3), IV(1,3), IV(2,3));
    vector3d_t   camera_x   = vector3d_t(IV(0,0), IV(0,1), IV(0,2)).ok_normalized();
    vector3d_t   camera_y   = vector3d_t(IV(1,0), IV(1,1), IV(1,2)).ok_normalized();
    vector3d_t   b_up    = camera_y;
    vector3d_t   b_right = camera_x;
    vector3d_t   b_look  = b_right.cross(b_up);
    matrix4x4d_t billboard_matrix;
    billboard_matrix(0,0) = b_right[0]; billboard_matrix(0,1) = b_right[1]; billboard_matrix(0,2) = b_right[2]; billboard_matrix(0,3) = 0.0;
    billboard_matrix(1,0) = b_up[0];    billboard_matrix(1,1) = b_up[1];    billboard_matrix(1,2) = b_up[2];    billboard_matrix(1,3) = 0.0;
    billboard_matrix(2,0) = b_look[0];  billboard_matrix(2,1) = b_look[1];  billboard_matrix(2,2) = b_look[2];  billboard_matrix(2,3) = 0.0;
    billboard_matrix(3,0) = 0.0;        billboard_matrix(3,1) = 0.0;        billboard_matrix(3,2) = 0.0;        billboard_matrix(3,3) = 1.0;

    mutex_.lock();
    {
      // Recompute xyz positions and sort placenames by distance to eye
      placemarks_update_locations_xyz(qgl, camera_eye);
      
      bool is_transparent = !is_causing_occlusion_ && !is_checking_occlusion_;

      // Render front to back using fade-in/fade-out, or back to front if transparent
      glPushAttrib(GL_ALL_ATTRIB_BITS);
      glDisable(GL_LIGHTING);
      glEnable(GL_DEPTH_TEST);
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      for (std::size_t i=0; i<render_placemarks_.size(); ++i) {
	std::size_t ii = (is_transparent ? render_placemarks_.size()-1-i : i);

	const std::size_t pl_idx        = render_placemarks_[ii].id();
	const int32_t     pl_fadestatus = render_placemarks_[ii].fadestatus();
	const point3d_t&  pl_pos        = render_placemarks_[ii].render_location_xyz();
        
        const vector2d_t  pl_wh = billboard_pixel_extent(pl_idx, pl_pos, viewport_width, viewport_height);
        
	// Update fade status and alpha value
        if (pl_fadestatus == 0) {
          // Invisible, test whether to change to fade in
          if (occupancy_map_test(occupancy_map,
                                 PV,
                                 pl_pos,
                                 pl_wh)) {
            // Visible, change status to fade in
            render_placemarks_[ii].set_fadestatus(1);
          }
        } else {
          // Visible, fade in, or fade out
          if (pl_fadestatus == 100) {
            // Fully visible
            bool unoccluded = occupancy_map_test_and_set(occupancy_map,
                                                         PV,
                                                         pl_pos,
                                                         pl_wh);
            if (!unoccluded) {
              // Change to fade out
              render_placemarks_[ii].set_fadestatus(-99);
            }
          } else if (pl_fadestatus>0) {
            // Continue fading in without check and without occluding other objects
            render_placemarks_[ii].set_fadestatus(std::min(pl_fadestatus + 5, 100));
          } else {
            // Continue fading out without check and continue occluding other objects
            occupancy_map_set(occupancy_map,
                              PV,
                              pl_pos,
                              pl_wh);
            render_placemarks_[ii].set_fadestatus(std::max(pl_fadestatus + 5, 0));
          }
        }

        double alpha = std::abs(double(render_placemarks_[ii].fadestatus()))/100.0; 

        //std::cerr << i << " => " << alpha << std::endl;

	// Render
        if (alpha>0.0) {

          glPushMatrix();
          {
	    const vector3d_t pl_pos_opengl = pl_pos - C;
	    
            glTranslated(pl_pos_opengl[0], pl_pos_opengl[1], pl_pos_opengl[2]);
            glMultMatrixd(billboard_matrix.to_pointer()); // Undo view rotation
            
            double perspective_scale = apparent_scale_at_distance(P, (pl_pos - camera_eye).two_norm());
            double scale_factor = 2.0/viewport_height/perspective_scale;
            glScaled(scale_factor, scale_factor, 1.0); // Undo perspective scaling and transform in pixel coords
            // Here, we work in pixel coords

            billboard_render(pl_idx, pl_pos, alpha);
	  
	  }
          glPopMatrix();

        }
      }
      glPopAttrib();
    }
    mutex_.unlock();
  }

  void terrain_billboard_placemarks::async_update_render_placemarks(const std::vector<placemark_render_data_t>& update_placemarks,
								    const projective_map3d_t& /*P*/,
								    const rigid_body_map3d_t& /*V*/,
								    const point3d_t& /*G*/) {
    // Update current render set
    mutex_.lock();
    std::vector<placemark_render_data_t> old_render_placemarks = render_placemarks_;
    mutex_.unlock();

    std::set<std::size_t> update_placemarks_set;
    for (std::size_t i=0; i<update_placemarks.size(); ++i) update_placemarks_set.insert(update_placemarks[i].id());

    std::set<std::size_t> old_render_placemarks_set;
    std::vector<placemark_render_data_t> new_render_placemarks;

    // Traverse old ones and update visibility status
    for (std::size_t i=0; i<old_render_placemarks.size(); ++i) {
      placemark_render_data_t x = old_render_placemarks[i];
      old_render_placemarks_set.insert(x.id());
      if (x.fadestatus() == 0) {
        // was invisible, keep it only if still in working set
        if (update_placemarks_set.find(x.id()) != update_placemarks_set.end()) {
          new_render_placemarks.push_back(x); // FIXME
        }
      } else if (x.fadestatus() == 100) {
        // visible, keep it if still in working set, otherwise fade out
        if (update_placemarks_set.find(x.id()) != update_placemarks_set.end()) {
          new_render_placemarks.push_back(x); // KEEP IT VISIBLE
        } else {
	  x.set_fadestatus(-99);
          new_render_placemarks.push_back(x); // START FADE OUT
        }          
      } else if (x.fadestatus() > 0) {
        // continue fade in
        new_render_placemarks.push_back(x);
      } else {
        // continue fade out
        new_render_placemarks.push_back(x);
      }
    }  

    // Traverse new ones and add newly appearing objs in invisible set
    for (std::size_t i=0; i<update_placemarks.size(); ++i) {
      placemark_render_data_t x = update_placemarks[i];
      if (old_render_placemarks_set.find(x.id()) == old_render_placemarks_set.end()) {
        // Newly appearing obj
	x.set_fadestatus(0);
        new_render_placemarks.push_back(x);
      }
    }
      
    mutex_.lock();
    render_placemarks_ = new_render_placemarks;
    are_render_placemarks_new_ = true;
    mutex_.unlock();
    
    // std::cerr << "FIXME: " << new_render_placemarks.size() << " VISIBLE PLACEMARKS " << std::endl;
  }

  void terrain_billboard_placemarks::async_update_self(const projective_map3d_t& P,
						       const rigid_body_map3d_t& V,
						       const point3d_t& G) {
    matrix4x4d_t IV = V.inverse().as_matrix();
    point3d_t    eye = point3d_t(IV(0,3), IV(1,3), IV(2,3));
    double       eye_altitude = altitude(eye);
    
    std::vector<placemark_render_data_t> candidates;
    ++update_counter_;

    if ((update_counter_%10) == 0) {
      if ((eye_altitude > min_distance_) &&
	  (eye_altitude < max_distance_)) {
	async_update_render_placemarks_candidates_in(candidates, P, V, G);
      }
      
      async_update_render_placemarks(candidates, P, V, G);
    }

    cbdam::background_thread::cpu_yield();
  }
}
