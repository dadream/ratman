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
#include <qgl_window_opossum.hpp>
//Added by qt3to4:
#include <QMouseEvent>
#include <QTimerEvent>
#include <QKeyEvent>
#include <glutil.h>
#include <iostream>
#include <cmath>
#include <cassert>
#include <stdlib.h>
#include <stdarg.h>
#include <qdatetime.h>

static QTime m_global_clock;

void warning_function(void* /* context */, const char*msg) {
  std::cerr << msg << std::endl;
}

qgl_window_opossum::qgl_window_opossum( QWidget* parent, const char* name)
    : qgl_window_base( parent, name ) {
  QWidget::setFocusPolicy(Qt::StrongFocus); // enable keybord events
  int i;
  m_pixel_tolerance = 3.0;

  m_renderer.set_geometry_cache_capacity(40*1024*1024);
  m_yaw = 0.0;
  m_pitch = 0.0;
  m_speed = 0.0;
  for( i = 0; i < 3; i++ ) {
    m_camera_position.c[i] = 0.0;
  }
  m_scene_width = 0.0;
  m_fps = 50;
  
  m_draw_terrain = true;
  m_draw_buildings = false;

  m_fps = 50;

  point3_t pos;
  pos.c[0] = 0; 
  pos.c[1] = 0;
  pos.c[2] = 0;
  m_terrain_intersection = std::make_pair(false, pos);
}

qgl_window_opossum::~qgl_window_opossum() {

}

void qgl_window_opossum::initializeGL() {
  m_renderer.init_opengl();

  // set texture parameters only if texture will be needed
  m_renderer.set_shading_enabled(false);

  // center camera over the center of bvolume watching down.
  set_initial_position();

  // redraw  to obtain m_fps frames per sec
  startTimer( (int)(1000.0 / m_fps) );
}

void qgl_window_opossum::init_frame() {
  glClearColor(0.5f, 0.76f, 0.9f,  0.0f);
  glClearDepth(1.0f);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void qgl_window_opossum::paintGL() {  
  // measure performances
  QTime clock;
  clock.restart();
  init_frame();

  // renderer doesn't care of the opengl projection and view matrices,
  // but uses directly the two matrices passed as parameter
  glMatrixMode(GL_PROJECTION);
  glLoadMatrixd(reinterpret_cast<GLdouble*>(&m_projection));
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixd(reinterpret_cast<GLdouble*>(&m_modelview));
#if 1
  m_renderer.render(m_draw_terrain, m_draw_buildings);
#else
  float l = -m_scene_width/2;
  float h = m_scene_width/2;
  glColor3f(1.0f, 0.7f, 0.5f);
  glBegin(GL_QUADS);
  glVertex3f(l, l, -10);
  glVertex3f(h, l, -10);
  glVertex3f(h, h, -10);
  glVertex3f(l, h, -10);
  glEnd();
#endif

  if (m_terrain_intersection.first) {
    std::cerr << "intersection at " << m_terrain_intersection.second.c[0] << ", "
	      << m_terrain_intersection.second.c[1] << ", " << m_terrain_intersection.second.c[2] << "\r";

    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.0f, 0.0f);
    glDisable(GL_DEPTH_TEST);
    glPointSize(5);
    glBegin(GL_POINTS);
    glVertex3f(m_terrain_intersection.second.c[0],
	       m_terrain_intersection.second.c[1],
	       m_terrain_intersection.second.c[2]);
    glEnd();
    glPointSize(1);
  }

  // measure rendering performances (frame duration expressed in sec
  glFinish();
  m_frame_duration = clock.elapsed() * 1E-3;

  draw_statistics();
}

void qgl_window_opossum::resizeGL( int w, int h ) {
  // used for refinement: screen tolerance as percent of height
  m_renderer.set_screen_tolerance( m_pixel_tolerance / height() );

  set_projection_matrix( w, h);
}

void qgl_window_opossum::set_projection_matrix(int w, int h) {
  double aspect_ratio = ( h==0 ) ? 1 :  ((double)w / (double)h);
  double distance = std::sqrt(m_camera_position.c[0] * m_camera_position.c[0] +
                              m_camera_position.c[1] * m_camera_position.c[1] +
                              m_camera_position.c[2] * m_camera_position.c[2]);
    
  double p_far = distance +m_scene_width;
  double p_near = p_far / 1000;
  double y_fov = 3.1415927 / 6.0;

  // set viewport and projection matrix
  glMatrixMode( GL_PROJECTION );
  glViewport( 0, 0, (GLint)w, (GLint)h );
  m_projection = perspective( y_fov,  aspect_ratio, p_near, p_far );
  glLoadMatrixd( reinterpret_cast<GLdouble*>( &m_projection ) );
  glMatrixMode( GL_MODELVIEW );
}

void qgl_window_opossum::timerEvent ( QTimerEvent * /* e */ ) {
  update_position();
  updateGL();
}

void qgl_window_opossum::mouseMoveEvent ( QMouseEvent * e ) { 
  m_terrain_intersection = current_graph_intersection_from_cursor_position( e->pos().x(), e->pos().y());
}

void qgl_window_opossum::mousePressEvent ( QMouseEvent * e ) {
  switch( e->button() ) {
  case  Qt::LeftButton: 
    break;
  case  Qt::MidButton:
    break;
  case Qt::RightButton:
    break;
  default:
    break;
  }
}

void qgl_window_opossum::mouseReleaseEvent ( QMouseEvent * e ) {
  switch( e->button() ) {
  case  Qt::LeftButton:
    break;
  case  Qt::MidButton:
    break;
  case Qt::RightButton:
    break;
  default:
    break;
  }
}

void qgl_window_opossum::print_commands() {
  warning_function( 0, "+-----------------keyboard commands ------------------+." );
  warning_function( 0, "  Left - Right\t- rotate about vertical axis." );
  warning_function( 0, "  Up - Down\t- moves forward / backward axis." );
  warning_function( 0, "  Home - End\t- tilt over planet surface." );
  warning_function( 0, "  PgUp - PgDwn \t\t- moves up / down." );
  warning_function( 0, "  Space\t\t- reset position." );
  warning_function( 0, "  c\t- enable / disable color." );
  warning_function( 0, "  t\t- enable / disable textures." );
  warning_function( 0, "  0\t- enable / disable terrain drawing." );
  warning_function( 0, "  b\t- enable / disable buildings." );
  warning_function( 0, "  w\t- enable / disable wireframe." );
  warning_function( 0, "  p\t- enable / disable patch color." );
  warning_function( 0, "  e\t- enable / disable elevation map." );
  warning_function( 0, "  o\t- enable / disable buildings occlusion culling." );
  warning_function( 0, "  f\t- enable / disable fog." );
  warning_function( 0, "  z\t- increase error tolerance." );
  warning_function( 0, "  x\t- decrease error tolerance." );
  warning_function( 0, "  ]\t- increase movement speed." );
  warning_function( 0, "  [\t- decrease movement speed." );
  warning_function( 0, "  mouse click + mouse move: cast ray to terrain.");
  warning_function( 0, "  h\t- print this help." );
  warning_function( 0, "+-----------------------------------------------------+." );
}

void qgl_window_opossum::keyPressEvent( QKeyEvent *e ) {   
  m_last_key_pressed = e->key();
  if ( !e->isAutoRepeat() ) {
    // manage status flags
    switch( m_last_key_pressed ) {
    case Qt::Key_Escape :
      emit stop_rendering();
      break;
    case Qt::Key_H :
      print_commands();	
      break;
    case Qt::Key_Space :
      set_initial_position();
      break;
    case Qt::Key_T :
      std::cerr << "dynamic decoration no more supported" << std::endl;
      break;
    case Qt::Key_W :
      m_renderer.set_wireframe_enabled( !m_renderer.is_wireframe_enabled() );
      break;
    case Qt::Key_P :
      m_renderer.set_patch_color_enabled(!m_renderer.is_patch_color_enabled());
      break;
    case Qt::Key_C :
      m_renderer.set_color_enabled(!m_renderer.is_color_enabled());
      break;
    case Qt::Key_E :
      m_renderer.set_elevation_map_enabled(!m_renderer.is_elevation_map_enabled());
      break;
    case Qt::Key_0 :
      m_draw_terrain = !m_draw_terrain;
      break;
    case Qt::Key_B :
      m_draw_buildings = !m_draw_buildings;
      break;
    case Qt::Key_O :
      m_renderer.set_occlusion_culling_enabled(!m_renderer.is_occlusion_culling_enabled());      
    case Qt::Key_F :
      m_renderer.set_fog_enabled(!m_renderer.is_fog_enabled());      
      break;
    case Qt::Key_S :
      m_renderer.set_shading_enabled(!m_renderer.is_shading_enabled());      
      std::cerr << "SHADING " << m_renderer.is_shading_enabled() << std::endl;
      break;
    default:
      break;
    }
  }


  // key autorepeat or not: manage repeatable keys
  switch( m_last_key_pressed ) {
  case Qt::Key_Z :
    m_pixel_tolerance+= 0.1;
    m_renderer.set_screen_tolerance( m_pixel_tolerance / height() );
    break;
  case Qt::Key_X :
    m_pixel_tolerance-= 0.1;
    if ( m_pixel_tolerance < 0 )
      m_pixel_tolerance = 0;
    m_renderer.set_screen_tolerance( m_pixel_tolerance / height() );
    break;
  case Qt::Key_BracketLeft :
    m_speed /= 1.5;
    break;
  case Qt::Key_BracketRight :
    m_speed *= 1.5;
    break;
  default:
    QWidget::keyPressEvent( e );
    break;
  }
}

void qgl_window_opossum::keyReleaseEvent ( QKeyEvent * e ){
  // set to null last key pressed (used by update position)
  m_last_key_pressed = 0;

  if ( !e->isAutoRepeat () ) {
    switch( e->key() ) {
    default:
      e->ignore();  
      break;     
    } 
  } else {
    e->ignore();  
  }
}

bool qgl_window_opossum::init(const char* input_name_height, 
			      const char* input_name_color, 
			      const char* input_name_buildings) {
  std::size_t batch_count = 4;
  std::size_t timeout_s = 60;
  m_scene_handle.open(input_name_height, input_name_color, input_name_buildings, batch_count, timeout_s);
  if (m_scene_handle.is_open()) {
    m_renderer.set_scene(m_scene_handle);
    warning_function( 0, "data loaded." );
    m_renderer.set_screen_tolerance( m_pixel_tolerance / height() );
    m_scene_width = m_scene_handle.root_side_length();
    m_speed = m_scene_width / 100.0;
    print_commands();

    return true;
  } else {
    std::cerr << "unable to open " << input_name_height << std::endl;
    return false;
  }
}

void qgl_window_opossum::set_initial_position() {
  double y_fov = 3.1415927 / 6.0;
  m_pitch = 0.0;
  m_yaw = 0.0;
  m_camera_position.c[ 0 ] = 0;
  m_camera_position.c[ 1 ] = 0;
  m_camera_position.c[ 2 ] = m_scene_width * 0.5 / tan(y_fov/2);

  std::cerr << "height " <<  m_camera_position.c[ 2 ]  << std::endl;
  // compute modelview matrix
  compute_model_view( m_yaw, m_pitch, m_camera_position, m_modelview );
}

void qgl_window_opossum::update_position() {
  double elapsed_time = m_global_clock.elapsed(); // as millisec
  m_global_clock.restart();

  // handle rotation and traslation, with speed proportional to 
  // the distance from the planet surface.
  double dist_percent = m_camera_position.c[2] / (10.0 * m_scene_width);
  if ( dist_percent < 0.1 )
    dist_percent = 0.1;

  // change this param to move slower/faster with pageup/pagedown keys
  const double acceleration = 0.0001 * m_speed;
  const double angle_conversion = 0.005;
  double delta_movement = 0.0;
  double delta_rotation = 0.0;
  double delta_tilt = 0.0;
  double delta_height = 1.0;
  double time_factor = elapsed_time * 0.1; // m_frame_duration * 100;

  switch( m_last_key_pressed ) {
  case Qt::Key_Up :
    delta_movement = dist_percent ;
    break;
  case Qt::Key_Down :
    delta_movement = -dist_percent;
    break;
  case Qt::Key_Left :
    delta_rotation = angle_conversion;
    break;
  case Qt::Key_Right :
    delta_rotation = -angle_conversion;
    break;
  case Qt::Key_PageUp :
    delta_height = (1+acceleration*time_factor);
    break;
  case Qt::Key_PageDown :
    delta_height = (1-acceleration*time_factor);
    break;
  case Qt::Key_Home :
    delta_tilt = angle_conversion;
    // clamp to 90 deg expressed in rad
    if ( m_pitch + delta_tilt > 1.5708 ) {
      delta_tilt = 0;
    }
    break;
  case Qt::Key_End :
    delta_tilt = -0.017450;			
    // clamp to 0 deg
    if ( m_pitch < 0 )
      delta_tilt = 0;
    break;
  default:
    break;
  }

  // decouple updates from time
  delta_tilt *= time_factor;
  delta_movement *= time_factor;
  delta_rotation *= time_factor;
  
  // update angles and position
  m_yaw += delta_rotation;
  m_pitch += delta_tilt;

  double cy, sy;
  cy = cos( m_yaw );
  sy = sin( m_yaw );
  m_camera_position.c[0] += -m_speed * sy * delta_movement;
  m_camera_position.c[1] += m_speed * cy * delta_movement;
  m_camera_position.c[2] = m_camera_position.c[2] * delta_height;
  std::pair<bool, double> res = m_scene_handle.current_graph_elevation_from_absolute(m_camera_position.c[0], m_camera_position.c[1]);
  if (res.first) {
    // there has been a hit, for collision detection set desired height offset
    double collision_detection_offset = 20;
    m_camera_position.c[2] = std::max(res.second + collision_detection_offset, m_camera_position.c[2]);
  } else if (m_camera_position.c[2] < 1) {
    m_camera_position.c[2] = 1;
  }

  // compute new modelview matrix
  compute_model_view( m_yaw, m_pitch, m_camera_position, m_modelview );

  // update projection matrix to keep always data inside near and far plane
  set_projection_matrix( width(), height() );
}

void qgl_window_opossum::compute_model_view(const double& yaw, 
                                            const double& pitch, 
                                            const point3_t& position,
                                            matrix44_t& modelview) const {
  matrix44_t rot_pitch = matrix_rotation( 0, -pitch );
  matrix44_t rot_yaw = matrix_rotation( 2, -yaw ); 
  matrix44_t translation=matrix_translation( -position.c[0], -position.c[1], -position.c[2] );
  modelview = matrix_multiply( rot_pitch, matrix_multiply( rot_yaw, translation ));
  return;
}

matrix44_t qgl_window_opossum::perspective(double y_fov, 
                                           double aspect, 
                                           double v_near, 
                                           double v_far) const {
  const double zero = 0.0;
  const double one  = 1.0;
  const double two  = 2.0;
  const double cot = one/tan(y_fov/two);

  matrix44_t p;
  double m[4][4]= {{cot/aspect,     zero,    zero,                                            zero},  
                   {zero,            cot,    zero,                                            zero},
                   {zero,            zero,    -(v_far + v_near)/(v_far - v_near),             -one},
                   {zero,            zero,   -(two * v_far * v_near)/(v_far - v_near),        zero} };
  for(int y = 0; y < 4; ++y) {
    for(int x = 0; x < 4; ++x) {
      p.c[x][y] = m[x][y];
    }
  }
  return p;
}

point3_t qgl_window_opossum::position(const matrix44_t& m) const {
  // get pos inverting matrix, p =  as R^t * (-T)
  const int dimension = 3;
  int i, j;
  double tmp;
  point3_t pos;
  for (i=0; i<dimension; i++) {
    tmp = 0;
    for (j=0; j<dimension; j++) {
      tmp -= m.c[ i ][ j ] * m.c[ dimension ][ j ];
    }
    pos.c[ i ] = tmp;
  }
  return pos;
}

matrix44_t qgl_window_opossum::matrix_multiply(const matrix44_t& lhs, 
                                               const matrix44_t& rhs) const {
  matrix44_t res;
  double tmp;
  int y,x,i;
  for( y = 0; y < 4; y++ )
    for( x = 0; x < 4; x++ ) {
      // compute product row * column  (use opengl transposed order)
      tmp = 0;
      for( i = 0; i < 4; i ++ ) 
	tmp += lhs.c[ i ][ y ] * rhs.c[ x ][ i ];
      res.c[ x ][ y ] = tmp;
    }
  return res;
}

matrix44_t qgl_window_opossum::matrix_translation(const double& x,
                                                  const double& y,
                                                  const double& z) const {
  // build identity matrix
  matrix44_t m;
  matrix_to_identity( m );

  // set translation (use opengl transposed order)
  m.c[ 3 ][ 0 ] = x;
  m.c[ 3 ][ 1 ] = y;
  m.c[ 3 ][ 2 ] = z;

  return m;
}

matrix44_t qgl_window_opossum::matrix_rotation(int axis, 
                                               const double& rad_theta ) const {
  double sin_theta = sin( rad_theta );
  double cos_theta = cos( rad_theta );
  matrix44_t m;
  matrix_to_identity( m );

  // m.c[ x ][ y ]
  switch( axis ) {
  case 0 :
    // rotation about x axis
    m.c[ 1 ][ 1 ] = cos_theta;
    m.c[ 1 ][ 2 ] = sin_theta;
    m.c[ 2 ][ 1 ] = -sin_theta;
    m.c[ 2 ][ 2 ] = cos_theta;
    break;
  case 1 :
    // rotation about y axis
    m.c[ 0 ][ 0 ] = cos_theta;
    m.c[ 0 ][ 2 ] = -sin_theta;
    m.c[ 2 ][ 0 ] = sin_theta;
    m.c[ 2 ][ 2 ] = cos_theta;
    break;
  case 2 :
    // rotation about z axis
    m.c[ 0 ][ 0 ] = cos_theta;
    m.c[ 0 ][ 1 ] = sin_theta;
    m.c[ 1 ][ 0 ] = -sin_theta;
    m.c[ 1 ][ 1 ] = cos_theta;      
    break;
  default:
    warning_function( 0, "qgl_window_opossum::rotation wrong axis selected." );
    break;
  }
  return m;
}

void qgl_window_opossum::matrix_to_identity(matrix44_t& lhs) const {
  int y,x;
  for( y = 0; y < 4; y++ )
    for( x = 0; x < 4; x++ ) {
      if ( x == y )
	lhs.c[ y ][ x ] = 1.0;
      else
	lhs.c[ y ][ x ] = 0.0;
    }
}

void qgl_window_opossum::invert_rigid_body_map(const matrix44_t& src, 
                                               matrix44_t& dst) const {
  // invert rotation (transpose)
  int i, j;
  const int dimension = 3;
  for (i=0; i<dimension; i++) {
    for (j=0; j<dimension; j++) {
      dst.c[ i ][ j ] = src.c[ j ][ i ];
    }
    dst.c[ i ][ dimension ] = 0;
  }
  dst.c[ dimension ][ dimension ] = 1;

  // invert translation: multiply R^-1 * T(-t)
  double tmp;
  for (i=0; i<dimension; i++) {
    tmp = 0;
    for (j=0; j<dimension; j++) {
      tmp -= dst.c[ j ][ i ] * src.c[ dimension ][ j ];
    }
    dst.c[ dimension ][ i ] = tmp;
  }
}

void qgl_window_opossum::draw_statistics() {
  float k = 7;
  float view_w = width() * k;
  float view_h = height() * k;
  float delta = 200;
  // disable texture if enabled 
  GLboolean texture_enabled;
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_CULL_FACE);
  glGetBooleanv( GL_TEXTURE_2D, &texture_enabled );
  if ( texture_enabled )
    glDisable( GL_TEXTURE_2D  );

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, view_w, 0, view_h );
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  // draw a semitrasparent rectangle
  glColor4f( 0.9f, 0.9f, 0.9f, 0.8f );
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBegin( GL_QUADS );
  glVertex2f( 0, 0 );
  glVertex2f( view_w, 0 );
  glVertex2f( view_w, delta * 2 );
  glVertex2f( 0, delta * 2 );
  glEnd();

  // antialiasing for the lines
  glEnable(GL_LINE_SMOOTH);

  glColor3f( 0.0f, 0.0f, 0.0f );
 
  float pos_x = delta / 4;
  float pos_y = delta + delta / 4;
  float delta_x = delta * 5;
  float h = delta / 2.5;

  glLineWidth( 2 );
  glutil_printf( pos_x, pos_y, h, "Pixel tol %2.1f", m_pixel_tolerance );
  pos_x += delta_x;

  // qtime accuracy is > 0.01 sec. so 
  // rendering times lower than 0.01 are not handled.
  if ( m_frame_duration == 0 ) {
    m_frame_duration = 0.01;
  }
  
  glutil_printf( pos_x, pos_y, h, "Fps %2.1f", 1.0 / m_frame_duration );
  pos_x += delta_x;

  glutil_printf( pos_x, pos_y, h, "MTri/s %2.1f",  m_renderer.stat_frame_rendered_triangles() * 1E-6 / m_frame_duration );
  pos_x += delta_x;

  glutil_printf( pos_x, pos_y, h, "Ktri %4.1f", m_renderer.stat_frame_rendered_triangles() * 1E-3 );
  pos_x = delta / 4;
  pos_y = delta / 4;

  glutil_printf( pos_x, pos_y, h, "VBO/f %d", m_renderer.stat_frame_rendered_vbo());
  pos_x += delta_x;

  glutil_printf( pos_x, pos_y, h, "H Km %4.1f", m_camera_position.c[2]/1000.0);
  pos_x += delta_x;

  std::pair<bool, double> res = m_scene_handle.current_graph_elevation_from_absolute(m_camera_position.c[0], m_camera_position.c[1]);
  if (res.first) {
    glutil_printf( pos_x, pos_y, h, "elev m %3.1f", res.second);
    pos_x += delta_x;
  }

  if (m_renderer.is_occlusion_culling_enabled()) {
    glutil_printf( pos_x, pos_y, h, "Occlusion enabled");
  }
  pos_x += delta_x;

  glLineWidth( 1 );

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  // disable antialiasing
  glDisable( GL_BLEND );
  glDisable( GL_LINE_SMOOTH );

  // enable texture
  if ( texture_enabled )
    glEnable( GL_TEXTURE_2D  );

  glPopAttrib();
  glMatrixMode(GL_MODELVIEW);
}

std::pair<bool, point3_t> qgl_window_opossum::current_graph_intersection_from_cursor_position(int x, int y) const {
  double x_pos,y_pos,z_pos;
  GLint xywh[4];
  glGetIntegerv( GL_VIEWPORT, xywh );
  int unproj_res = gluUnProject(x, xywh[3] - 1 - y, 1.0, 
				reinterpret_cast<const GLdouble*>(&m_modelview),
				reinterpret_cast<const GLdouble*>(&m_projection), 
				xywh, &x_pos, &y_pos, &z_pos);
  if (unproj_res) {  
    // test absolute intersection
    std::pair<bool, double> res = m_scene_handle.current_graph_intersection_from_absolute(m_camera_position.c[0], m_camera_position.c[1], m_camera_position.c[2],
											  x_pos, y_pos, z_pos);
    if (res.first) {
      // convert t into r(t) = o + d * t
      // : origin + (extremity - origin).normalized() * t
      double t = res.second;
      point3_t dir;
      point3_t pos;
      dir.c[0] = x_pos - m_camera_position.c[0];
      dir.c[1] = y_pos - m_camera_position.c[1];
      dir.c[2] = z_pos - m_camera_position.c[2];
      double len = sqrt(dir.c[0]*dir.c[0] + dir.c[1]*dir.c[1] + dir.c[2]*dir.c[2]);
      if (len == 0) len = 1;
      for(int i = 0; i < 3; ++i) {
	dir.c[i] /= len;
	pos.c[i] = m_camera_position.c[i] + dir.c[i] * t;
      }
      return std::make_pair(true, pos);
    } else {
      point3_t pos;
      pos.c[0] = 0; 
      pos.c[1] = 0;
      pos.c[2] = 0;
      return std::make_pair(false, pos);
    }
  } else {
    std::cerr << "unable to unproject " << x << ", " << y << std::endl;
    point3_t pos;
    pos.c[0] = 0; 
    pos.c[1] = 0;
    pos.c[2] = 0;
    return std::make_pair(false, pos);
  }
}



