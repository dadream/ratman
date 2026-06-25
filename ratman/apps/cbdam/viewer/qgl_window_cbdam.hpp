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
#ifndef VIEWER_QGL_WINDOW_CBDAM_H
#define VIEWER_QGL_WINDOW_CBDAM_H

#include <GL/glew.h>
#include <vic/cbdam/base/camera.hpp>
#include <vic/cbdam/base/camera_controller_base.hpp>
#include <vic/cbdam/base/terrain_model.hpp>
#include <vic/cbdam/base/building_hierarchy.hpp>
#include "qgl_window_base.hpp"

//Added by qt3to4:
#include <QMouseEvent>
#include <QTimerEvent>
#include <QKeyEvent>
#include <vic/cbdam/base/terrain_model_renderer.hpp>
#include <vic/cbdam/base/building_renderer.hpp>
#include <string>
#include <vector>

/** 
 * qgl_window_cbdam class, derived from qgl_window_base
 * is a  Qt OpenGL window derived from base class qgl_window_base
 */
class qgl_window_cbdam : public qgl_window_base
{

  Q_OBJECT

    public:
  /// Default ctor: recalls base class qgl_base_window ctor 
  qgl_window_cbdam(QWidget* parent);

  /// Destructor recalls makeCurrent()
  virtual ~qgl_window_cbdam();

  bool open(const std::string& height_url);

  void insert_color_layer(const std::string& id, cbdam::geoimage_quad_fetcher* fetcher, 
			  std::size_t first_level, std::size_t last_level, double min_height, double max_height, 
			  bool is_base_layer, bool is_active);

  bool init_buildings(const char* fname);
  
  const cbdam::cbdam_diamond_fetcher* elevation_fetcher() const;

  bool configure_verification(const std::string& script_file,
			      const std::string& output_dir,
			      bool exit_when_done,
			      bool log_state);

  void set_verification_window_size(int width, int height);

  bool verification_failed() const;

 signals:
  /// signal emitted to stop application
  void stop_rendering();

  public slots:

 protected:

  /// Initialize settings with FLAT shade and enabling one light
  virtual void initializeGL();

  /// Paints the world
  virtual void paintGL();

  /// set view frustum to perspective calling set_projection_matrix
  virtual void resizeGL( int w, int h );
  
  /**
   * set the eprojection matrix, to include all the data
   * in the viewing frustum.
   */
  void set_projection_matrix(int width, int height);

  // mouse events
  virtual void mouseMoveEvent (QMouseEvent * e);
  virtual void mousePressEvent (QMouseEvent * e);
  virtual void mouseReleaseEvent (QMouseEvent * e);
  virtual void keyPressEvent (QKeyEvent * e);
  virtual void keyReleaseEvent (QKeyEvent * e);

  /// handle timer events
  virtual void timerEvent (QTimerEvent *e);

  // protected copy ctor and operator=
  qgl_window_cbdam(const qgl_window_cbdam& rhs);
  qgl_window_cbdam operator=(const qgl_window_cbdam& rhs);
  
  void init_frame();

  void draw_statistics();

  void draw_rectangle(float delta, float pos_y, double percent);

  void print_commands();

  void set_initial_position();

  void enable_poligon_stipple();

  std::pair<bool, cbdam::point3d_t> current_graph_intersection_from_cursor_position(int x, int y) const;

  struct verify_action_t {
    std::string name;
    std::vector<std::string> args;
  };

  bool load_verify_script(const std::string& script_file);

  void process_verify_actions();

  bool ensure_verify_window_size();

  bool execute_verify_action(const verify_action_t& action);

  bool capture_verify_state(const std::string& name);

  bool write_verify_state(const std::string& name, const std::string& path);

  void log_verify_event(const std::string& event, const std::string& detail) const;

  void fail_verify(const std::string& detail);

  bool apply_verify_key(const std::string& key);

  void verify_zoom(int steps, bool zoom_in);

  void verify_tilt(double degrees);

  void verify_rotate(double degrees);

  void set_verify_flight_view(const cbdam::camera::vector3_t& position);

 protected:
  //data members
  cbdam::terrain_model*                 m_terrain_model;
  cbdam::camera                         m_camera;
  cbdam::camera_controller_base*	m_camera_controller;
  cbdam::terrain_model_renderer*        m_renderer;
  cbdam::building_hierarchy             m_building_hierarchy;
  cbdam::building_renderer              m_building_renderer;
  cbdam::vector4_t                      m_light_direction;
  cbdam::point3d_t			m_old_position;
  float					m_speed;
  float                                 m_pixel_tolerance;
  float                                 m_fps;
  float                                 m_y_fov;
  int                                   m_statistics_mode;
  float                                 m_planet_radius;
  unsigned int                          m_patch_width;
  unsigned int                          m_frame_count;
  float                                 m_elapsed_time;
  float                                 m_mean_fps;
  bool					m_building_renderer_enabled;
  std::pair<bool, cbdam::point3d_t>	m_current_intersection;
  bool                                  m_verify_enabled;
  bool                                  m_verify_exit_when_done;
  bool                                  m_verify_log_state;
  bool                                  m_verify_failed;
  bool                                  m_verify_done;
  std::string                           m_verify_output_dir;
  std::vector<verify_action_t>          m_verify_actions;
  std::size_t                           m_verify_next_action;
  unsigned int                          m_verify_wait_until_frame;
  unsigned int                          m_verify_rendered_frames;
  double                                m_verify_pitch;
  double                                m_verify_yaw;
  int                                   m_verify_expected_width;
  int                                   m_verify_expected_height;
  unsigned int                          m_verify_resize_wait_frames;
};

#endif // VIEWER_QGL_WINDOW_CBDAM_H
