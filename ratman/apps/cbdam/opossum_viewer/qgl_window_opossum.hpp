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
#ifndef OPOSSUM_VIEWER_QGL_WINDOW_OPOSSUM_H
#define OPOSSUM_VIEWER_QGL_WINDOW_OPOSSUM_H

#include <qgl_window_base.hpp>
//Added by qt3to4:
#include <QMouseEvent>
#include <QTimerEvent>
#include <QKeyEvent>
//#include <vic/opossum/scene_renderer.hpp> 
#include <opossum/scene_renderer.hpp> // changed for compatibility with old opossum
#include <qgl.h>

typedef struct matrix44 {
  double c[4][4];
} matrix44_t;

typedef struct point3 {
  double c[3];
} point3_t;

/** 
 * qgl_window_opossum class, derived from qgl_window_base
 * is a  Qt OpenGL window which display a opossum::planet
 * using opossum::renderer with a minimal
 * keyboard interface
 */
class qgl_window_opossum : public qgl_window_base
{

  Q_OBJECT

    public:
  /**
   * Default ctor: recalls base class qgl_base_window ctor 
   */
  qgl_window_opossum( QWidget* parent, const char* name );

  /// Destructor recalls makeCurrent()
  virtual ~qgl_window_opossum();

  /**
   * Load planet data
   */
  bool init(const char* input_name_height, 
	    const char* input_name_color, 
	    const char* input_name_buildings);

 signals:
  void stop_rendering();

 protected:

  /**
   * Initialize clear colors and start timer
   */
  virtual void initializeGL();

  /**
   * Draw the planet.
   */
  virtual void paintGL();

  /**
   * Reset the projection matrix
   */
  virtual void resizeGL( int w, int h );

  // mouse events: not handled.
  virtual void mouseMoveEvent ( QMouseEvent * e );
  virtual void mousePressEvent ( QMouseEvent * e );
  virtual void mouseReleaseEvent ( QMouseEvent * e );

  /**
   * keyboard event: used for movement and flag setting.
   */
  virtual void keyPressEvent ( QKeyEvent * e );

  /**
   * keyboard event: not handled
   */
  virtual void keyReleaseEvent ( QKeyEvent * e );

  /**
   * Execute redraw each timer event.
   */
  virtual void timerEvent ( QTimerEvent *e );

  /**
   * protected copy ctor
   */
  qgl_window_opossum(const qgl_window_opossum& rhs );
  
  /**
   * Clear buffers.
   */
  void init_frame();

  /**
   * Draw a rectangle containing statistic data
   */
  void draw_statistics();

  /**
   * Draw a string in the opengl buffer
   */
  void output_string(float x, float y, const char *format, ...);

  /**
   * Print help to the standard output.
   */
  void print_commands();

  /**
   * Insert one tree over unprojected xy screen coordinates
   */
  void seed_one_tree(int x, int y);

  /**
   * Set the projection matrix to perspective.
   */
  void set_projection_matrix(int w, int h);

  /**
   * Reset position to 0,0, 2*planet radius, looking to -z.
   */
  void set_initial_position();

  /**
   * Update the position
   */
  void update_position();

  /**
   * Update rotation matrix with incremental rotations. 
   */
  void update_rotation(const double& delta_rx, 
		       const double& delta_ry, 
		       const double& delta_rz, 
		       matrix44_t& rotation) const;

  /**
   * Compute a modelview matrix.
   */
  void compute_model_view(const double& yaw, 
			  const double& pitch, 
			  const point3_t& position,
			  matrix44_t& modelview) const;


  /**
   * Compute perspective matrix. y_fov is expressed in radians,
   */
  matrix44_t perspective(double y_fov, 
                         double aspect, 
                         double v_near, 
                         double v_far) const;
  /**
   * Compute a inverse of a rigid body map
   */
  void invert_rigid_body_map(const matrix44_t& src, 
			     matrix44_t& dst) const;

  /**
   * Invert m and return its translation value.
   * Get position from m, when m is the opengl modelview matrix.
   */
  point3_t position(const matrix44_t& m) const;

  /**
   * Compute matrix multiplication dst = src * dst
   */
  matrix44_t matrix_multiply(const matrix44_t& src,
                             const matrix44_t& dst) const;

  /**
   * Compute matrix of rotation about x( 0 ), y( 1 ) or 
   * z( 2 ) axis of angle rad, expressed in radians.
   */
  matrix44_t matrix_rotation(int axis, 
                             const double& rad_theta ) const;

  /** 
   * Compute translation matrix
   */
  matrix44_t matrix_translation(const double& x,
                                const double& y,
                                const double& z) const;

  /**
   * Set matrix to identity
   */
  void matrix_to_identity(matrix44_t& lhs) const;

  std::pair<bool, point3_t> current_graph_intersection_from_cursor_position(int x, int y) const;

  //data members	
  opossum::scene_handle		m_scene_handle;
  opossum::scene_renderer	m_renderer;
  point3_t	m_camera_position;
  double	m_yaw;
  double	m_pitch;
  double	m_speed;

  matrix44_t	m_projection;		//< opengl matrix index scheme ( access data as c.[ x ][ y ] )
  matrix44_t	m_modelview;		//< opengl matrix index scheme ( access data as c.[ x ][ y ] )

  double	m_pixel_tolerance;
  double	m_fps;
  double	m_frame_duration;
  double        m_scene_width;

  int		m_last_key_pressed;
  bool          m_draw_terrain;
  bool          m_draw_buildings; 
  std::pair<bool,point3_t> m_terrain_intersection;
};

#endif // OPOSSUM_VIEWER_QGL_WINDOW_OPOSSUM_H
