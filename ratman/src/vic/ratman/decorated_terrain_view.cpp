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
#include <vic/ratman/decorated_terrain_view.hpp>
#include <vic/ratman/qgl_scene_view.hpp>
#include <vic/ratman/atmosphere.hpp>
#include <vic/cbdam/base/background_thread.hpp>
#include <vic/cbdam/base/terrain_model.hpp>

#include <qthread.h>

#include <GL/glew.h>
#include <cassert>


namespace ratman {

  namespace detail {
    
    /**
     * Separated update thread which takes care of updating terrain and all its decorations.
     */
    class decorated_terrain_view_update_thread: public cbdam::background_thread {
    protected:
      mutable QMutex mutex_;
      decorated_terrain_view* view_;
      bool stop_requested_;
    public:

      decorated_terrain_view_update_thread(decorated_terrain_view* view) :
          view_(view), stop_requested_(false) {
      }

      virtual ~decorated_terrain_view_update_thread() {
      }
      
      bool stop_requested() const {
        bool result;
        mutex_.lock();
        result = stop_requested_;
        mutex_.unlock();
        return result;
      }

      void request_stop() {
        mutex_.lock();
        stop_requested_ = true;
        mutex_.unlock();
      }
        
      virtual void run() {
	// set background priority
	cbdam::background_thread::run();

	std::cerr << std::endl  << std::endl << "START REAL RUN" << std::endl << std::endl << std::endl;

        while (!stop_requested()) {
	  cbdam::background_thread::cpu_long_yield();
          view_->async_update();
        }
      }
 
    };
  }
  
  // =============================================================

  decorated_terrain_view::decorated_terrain_view() :
      mutex_(QMutex::Recursive),
      terrain_(0),
      decorations_root_(this, std::string("decorations")) {
    decorations_root_.set_active(true);
    camera_fovy_ = 30.0f*3.14f/180.0f;
    viewport_width_  = 100;
    viewport_height_ = 100;
    camera_V_ = lookat3D(point3d_t(0.0, 0.0, 10000.0),
                         point3d_t(0.0, 0.0, 0.0),
                         0.0);
    update_thread_ = 0;
  }
   
  decorated_terrain_view::~decorated_terrain_view() {
    // FIXME !!!!!
  }

  void decorated_terrain_view::set_terrain_layer(terrain_renderable* x) {
    terrain_ = x;
  }

  const terrain_renderable* decorated_terrain_view::terrain_layer() const {
    return terrain_;
  }

  terrain_renderable* decorated_terrain_view::terrain_layer() {
    return terrain_;
  }
  
  const active_renderable* decorated_terrain_view::decorations_root() const {
    return &decorations_root_;
  }

  active_renderable* decorated_terrain_view::decorations_root() {
    return &decorations_root_;
  }

  void decorated_terrain_view::insert_decoration_layer(active_renderable* x) {
    decorations_root_.insert_child(x);
  }

  void decorated_terrain_view::set_cameraV(const rigid_body_map3d_t& V,
					   const point3d_t& ground_target) {
    mutex_.lock();
    camera_V_ = V;
    camera_ground_target_ = ground_target;
    mutex_.unlock();
  }
      
  rigid_body_map3d_t decorated_terrain_view::cameraV() const {
    rigid_body_map3d_t result;
    mutex_.lock();
    result = camera_V_;
    mutex_.unlock();
    return result;
  }
 
  point3d_t decorated_terrain_view::camera_ground_target() const {
    point3d_t result;
    mutex_.lock();
    result = camera_ground_target_;
    mutex_.unlock();
    return result;
  }
  
  void decorated_terrain_view::set_camera_fovy(float rad) {
    mutex_.lock();
    camera_fovy_ = rad;
    mutex_.unlock();
  }

  void decorated_terrain_view::set_viewport(int width, int height) {
    mutex_.lock();
    viewport_width_  = width;
    viewport_height_ = height;
    mutex_.unlock();
  }    

  projective_map3d_t decorated_terrain_view::cameraP() const {
    // FIXME I3D !!!! 
    // NEAR/FAR planes for spherical
    point3d_t eye = ~camera_V_ * point3d_t(0.0, 0.0, 0.0);
    double p_near, p_far;
    if (terrain_->model()->is_planar()) {
      aabox2d_t uvbox = terrain_->model()->uv_box();
      point3d_t center = point3d_t(uvbox.center()[0], uvbox.center()[1], 0.0);
      vector3d_t diagonal = vector3d_t(uvbox.diagonal()[0], uvbox.diagonal()[1], 0.0);
      p_far = (eye - center).two_norm() + diagonal.two_norm();
      p_near = p_far / 16384; // worst case zbuffer
    } else {
      // Get far plane as far to see at least highest peek over horizon
      // Compute the distance from the viewpoint of the farther visible point on the planet.
      // Suppos Rmin = radius, Rmax = radius + max height
      // Compute distance from viewpoint of the intersection on the Rmax sphere
      // of the ray passing from view point and tangent to the inner sphere
      // D = sqrt( D^2 - Rmin^2 ) + sqrt( Rmax^2 - Rmin^2 )
      const double r = terrain_->model()->radius();
      const double d = std::max(1.1*r, as_vector(eye).two_norm()); // distance from sphere center

      // delta_radius = sqrt( max_radius^2 - min_radius^2 )
      p_far = std::sqrt(d*d - r*r);
      p_near = p_far / 16384; // worst case zbuffer
    }

    projective_map3d_t result;
    mutex_.lock();
    result = perspective3D(double(camera_fovy_),
			   double(viewport_width_)/double(viewport_height_),
			   p_near,
			   p_far);
    mutex_.unlock();
    return result;
  }
    
  bool decorated_terrain_view::on_event(qgl_scene_view& qgl,
					QEvent* e) {
    mutex_.lock();
    projective_map3d_t P = cameraP();
    rigid_body_map3d_t V = cameraV();
    mutex_.unlock();

    bool result = false;
    for (std::size_t i=0; i<decorations_root_.child_count(); ++i) {
      std::size_t ii= decorations_root_.child_count()-1-i;
      if (decorations_root_.child(ii)->priority() >= 0) {
        result = decorations_root_.child(ii)->on_event(qgl, e, P, V);
	if (result) return result;
      }
    }
    //std::cerr << std::endl;

    // Render terrain
    //std::cerr << " T";
    result = terrain_->on_event(qgl, e, P, V);
    if (result) return result;										   

    //std::cerr << "RENDER: ";
    // Render all negative priority children
    for (std::size_t i=0; i<decorations_root_.child_count(); ++i) {
      std::size_t ii= decorations_root_.child_count()-1-i;
      if (decorations_root_.child(ii)->priority() < 0) {
        result = decorations_root_.child(ii)->on_event(qgl, e, P, V);
	if (result) return result;										   
      }
    }

    // If it is a mouse-move reset pointer to current to mouseover=false, children
    // will eventually set it to true
    if ((e->type() == QEvent::MouseMove) && (qgl.is_good_cursor(qgl.current_cursor()))) {
      qgl.set_current_cursor(qgl.current_cursor());
    }

    return result;
  }

  void decorated_terrain_view::render(qgl_scene_view& qgl) {
    glClearColor(0.38f, 0.65f, 0.78f, 0.0f); 
    glClearDepth( 1.0f );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glEnable(GL_LIGHTING);

    mutex_.lock();
    projective_map3d_t P = cameraP();
    rigid_body_map3d_t V = cameraV();
    mutex_.unlock();
    
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(P.to_pointer());
    glMatrixMode(GL_MODELVIEW);
    
    point3d_t eye = (~V) * point3d_t();
    
    point3d_t render_center;
    if (terrain_->model()->is_planar()) {
      render_center = point3d_t(eye[0], eye[1], 0.0);
    } else {
      render_center = as_point(as_vector(eye).ok_normalized() * terrain_->model()->radius());
    }
    
    rigid_body_map3d_t VM = V * translation3D(vector3d_t(render_center[0], render_center[1], render_center[2]));
    
    glLoadMatrixd(VM.to_pointer());
    
    active_renderable::occupancy_map_t occupancy_map(qgl.width()/8,
                                                     qgl.height()/8);
    for (std::size_t x= 0; x<occupancy_map.extent()[0]; ++x) {
      for (std::size_t y=0; y<occupancy_map.extent()[1]; ++y) {
        occupancy_map(x,y) = false; // fill
      }
    }

    for (std::size_t i=0; i<decorations_root_.child_count(); ++i) {
      if (decorations_root_.child(i)->priority() < 0) {
	decorations_root_.child(i)->render(qgl, occupancy_map, P, V, render_center);
      }
    }
    
    if (atmosphere_ && atmosphere_->is_active()) {
      glEnable(GL_LIGHT0);
      sl::vector4f sun =
	sl::vector4f(atmosphere_->light_direction()[0],
		     atmosphere_->light_direction()[1],
		     atmosphere_->light_direction()[2],
		     0.0);
      
      sl::color4f amb =
	sl::color4f(atmosphere_->sky_ambient()[0],
		    atmosphere_->sky_ambient()[1],
		    atmosphere_->sky_ambient()[2],
		    1.0);
      
      glLightfv(GL_LIGHT0, GL_POSITION,sun.to_pointer());
      glLightfv(GL_LIGHT0, GL_AMBIENT, amb.to_pointer());
      glLightfv(GL_LIGHT0, GL_DIFFUSE, atmosphere_->sun_diffuse().to_pointer());
      glLightfv(GL_LIGHT0, GL_SPECULAR, sl::color4f(0.0, 0.0, 0.0, 1.0).to_pointer());
      glLighti(GL_LIGHT0, GL_SPOT_CUTOFF, 180);
      
      // FOG
      float yaw = rad2deg(qgl.camera_controller().get_oriented_position().yaw());  
      double z = qgl.camera_controller().camera_altitude();
      
      set_camera_fovy(deg2rad(atmosphere_->fov_y()));
      
      const float beta_sl = 1e-5f * atmosphere_->turbidity();
      const float h = std::max(0.0, z);
      const float gamma = 1.0f/8000.0f;
      const float beta_h = beta_sl * exp(-gamma*h);
      
      glFogi(GL_FOG_MODE, GL_EXP);		// Fog Mode
      glFogfv(GL_FOG_COLOR, atmosphere_->fog_color(yaw).to_pointer());// Set Fog Color
      glFogf(GL_FOG_DENSITY, beta_h);	// How Dense Will The Fog Be
      glHint(GL_FOG_HINT, GL_DONT_CARE);	// Fog Hint Value
      glEnable(GL_FOG);				// Enables GL_FOG
      
      if (!terrain_->is_shading_enabled()) {
	terrain_->set_shading_enabled(true);
      }
    } else {
      glDisable(GL_FOG);
      glDisable(GL_LIGHT0);
      glDisable(GL_LIGHTING); 
      glDisable(GL_COLOR_MATERIAL);
      glColor3f(1,1,1);
      if (terrain_->is_shading_enabled()) {
	terrain_->set_shading_enabled(false);
      }
    }
    
    glColor3f(1,1,1);
    terrain_->render(qgl, occupancy_map, P, V, render_center);
    
    if (atmosphere_ && atmosphere_->is_active()) {
      glDisable(GL_FOG);
      glDisable(GL_LIGHT0);
      glDisable(GL_LIGHTING); 
      glEnable(GL_DEPTH_TEST);
#if 0
      // DEBUG
      sl::vector4f sun =
	sl::vector4f(atmosphere_->light_direction()[0],
		     atmosphere_->light_direction()[1],
		     atmosphere_->light_direction()[2],
		     0.0);
      glColor3f(1,1,1);
      glLineWidth(5);
      glBegin(GL_LINES);
      {
	glVertex3f(0, 0, 0);
	glVertex3f(10000*sun[0], 10000*sun[1], 10000*sun[2]);
      }
      glEnd();
#endif
    }
    
    // Render all non negative priority children
    for (std::size_t i=0; i<decorations_root_.child_count(); ++i) {
      if (decorations_root_.child(i)->priority() >= 0) {
	decorations_root_.child(i)->render(qgl, occupancy_map, P, V, render_center);
      }
    }
  }
  
    
  void decorated_terrain_view::async_update_start() {
    if (!update_thread_) {
      update_thread_ = new detail::decorated_terrain_view_update_thread(this);
      update_thread_->start(QThread::IdlePriority); // FIXME OVERWRITTEN TO LOWPRIORITY BY BACKGROUND_THREAD->RUN()
    }
    terrain_->model()->update_start();    
  }
  
  void decorated_terrain_view::async_update_stop() {
    //assert(terrain_);
    //assert(terrain_->model());
    if (update_thread_) {
      update_thread_->request_stop();
      update_thread_->wait();
      delete update_thread_;
      update_thread_ = 0;
    }
    terrain_->model()->update_stop();    
  }

  void decorated_terrain_view::async_update() {
    mutex_.lock();
    projective_map3d_t P = cameraP();
    rigid_body_map3d_t V = cameraV();
    point3d_t G = camera_ground_target();
    mutex_.unlock();

    if (terrain_) terrain_->async_update(P, V, G);
    decorations_root_.async_update(P, V, G);
  }
}
