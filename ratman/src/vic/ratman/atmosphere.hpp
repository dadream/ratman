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
#ifndef RATMAN_ATMOSPHERE_HPP
#define RATMAN_ATMOSPHERE_HPP

#include <GL/glew.h>
#include <vic/ratman/ratman.hpp>
#include <vic/ratman/active_renderable.hpp>
#include <vic/ratman/camera_controller.hpp>
#include <sl/affine_map.hpp>
#include <sl/rigid_body_map.hpp>
#include <sl/clock.hpp>

namespace ratman {
  
  class atmosphere: public active_renderable {
    
  protected:
    float current_theta_;
  public:
    atmosphere(decorated_terrain_view* scene, const std::string& name);
    virtual ~atmosphere();

    /// Activate/deactivate the object
    virtual void set_active(bool x);

    /// Handle picking and return true if handled
    virtual bool on_select_self(const projective_map3d_t& P,
                                const rigid_body_map3d_t& V,
                                const point2d_t& xy);
    
    /// Render the hierarchy object rooted at this
    virtual void render_self(qgl_scene_view& qgl,
                             occupancy_map_t& occupancy_map,
			     const projective_map3d_t& P,
                             const rigid_body_map3d_t& V,
			     const point3d_t& C);
    
    /// Handle picking and return true if handled
    virtual bool on_event_self(qgl_scene_view& qgl,
			       QEvent* e,
			       const projective_map3d_t& P,
			       const rigid_body_map3d_t& V);

   
public slots:

void set_azimuth(int a){
  //convert in NE coordinates
  int ne = 360-a;
  sun_azimuth_ = float(ne);
}
   
    void set_altitude(int a){
      sun_theta_ = float(a);
    }
    void set_turbidity(int a);

    float turbidity(){
      return turbidity_;
    }

    void set_fov_y(float f){
      fov_y_=f;
    }
    float fov_y(){
      return fov_y_;
    }

    void set_real_time_mode(bool b);
    
    void set_daytime(int a);
    void set_minutes(int a);
    void set_date(int a);
    
    void compute_sky();
    void set_real_sun(const float& current_lon, const float& current_lat);
   
  public:
    
    void set_skycolor(const QImage &img);
    void set_sun();

    void init_gl_draw();
    void init_gl_tex(); 
  
    void build_quads_dome(float radius, int dtheta, int dphi);
    void render_quads_skydome();

    void render_sun_disk(const matrix4x4d_t& V);

    void destroy_dome();
    
    sl::vector3f Yxy_zenit();
    sl::vector3f PF_distribution(const float& theta,const float& gamma);
     
    sl::vector3f light_direction();
    sl::color4f fog_color(float yaw);

    sl::color4f sky_ambient();
    sl::color4f sun_diffuse();


               
  protected:
    bool sun_visible_;
    bool first_frame_;
    bool real_time_mode_;
    
    std::vector<sl::point3f> sky_vertices_xyz_;
    std::vector<sl::point2f> sky_vertices_uv_;
    float dome_radius_;

    QImage current_sky_image_;
    GLuint gl_texid_[2];
   
    float sun_azimuth_;
    float sun_theta_;
    float turbidity_;

    float daytime_;
    float minutes_;
    int j_date_;
    float current_decl_;

    float fov_y_;

    float current_longitude_;
    float current_latitude_;
    sl::rigid_body_map3d current_frame_;
    
    sl::vector3f zenit_values_;
    std::vector<sl::vector3f> PF_sun_;
    
    sl::vector3f sky_ambient_;
    sl::vector3f sun_diffuse_;
    sl::vector3f sun_direction_;
    sl::vector3f horizont_diffuse_front_;
    sl::vector3f horizont_diffuse_back_;

    oriented_position last_oriented_position_;
    sl::real_time_clock clock_;

  };
  
}

#endif
