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
#include <GL/glew.h>

#include <vic/cbdam/base/camera_controller_vtrackball.hpp>
#include <vic/cbdam/base/camera_controller_flight.hpp>
#include <vic/cbdam/base/cbdam_diamond_fetcher.hpp>
#include <vic/cbdam/base/geoimage_quad_fetcher.hpp>
#include <vic/cbdam/base/dummy_geoimage_quad_fetcher.hpp>

//#include <q3filedialog.h>
//Added by qt3to4:
#include <QMouseEvent>
#include <QTimerEvent>
#include <QKeyEvent>
#include <QFileDialog>
#include <glutil.h>	// for output_string
#include <sl/fixed_size_point.hpp>
#include <sl/affine_map.hpp>
#include <sl/clock.hpp>
#include <iostream>
#include <fstream>
#include <cassert>
#include "qgl_window_cbdam.hpp"
#include "glutil.h"	// for output_string

#ifndef _WIN32
#include <unistd.h>	// sleep function
#endif

#define FULLSCREEN_WORKING 0

static bool thread_running = false;

void warning_function(void* /* context */, const char*msg) {
  std::cerr << msg << std::endl;
}

sl::real_time_clock g_clock;
sl::real_time_clock g_speed_clock;

cbdam::uint32_t decore_texture_function(void*  /* context */,
					const double* tm_g2l,
					const double* /* tm_l2g */,
					double magnification) {
  float x_pos = (float)g_clock.elapsed().as_microseconds()/(50*1000000.0f);
  if ( 20*(x_pos-1) > 200 ) {
    g_clock.restart();
  }
  glEnable( GL_BLEND );
  glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable( GL_LINE_SMOOTH );

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0.0f, 1.0f, 0.0f, 1.0f );
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glLineWidth(0.5*magnification);
  float f = 0.0f;
  float l = 1.0f;
  glBegin( GL_LINE_STRIP );
  glVertex2f( f, f );
  glVertex2f( f, l );
  glVertex2f( l, l );
  glVertex2f( l, f );
  glVertex2f( f, f );
  glEnd();

  glLoadMatrixd( tm_g2l );
  glLineWidth(0.5*magnification);
  glutil_printf( 1.0-x_pos, 0.45, 0.1, "%s", "dynamic texturing"); 
  //glutil_printf( 1.0-x_pos, 0.45, 0.1, "%s", str); 
  glLineWidth( 1 );

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glDisable( GL_BLEND );
  glDisable( GL_LINE_SMOOTH );

  static cbdam::uint32_t valid_frame_count = 5;
  return valid_frame_count;
}

qgl_window_cbdam::qgl_window_cbdam(QWidget* parent)
    : qgl_window_base( parent) {
  m_terrain_model = 0;
  m_renderer = 0;
  //  m_renderer->set_dynamic_decoration_function((cbdam::decore_texture_function_t)decore_texture_function, 0);
   
  QWidget::setFocusPolicy(Qt::StrongFocus); // enable keyboard events
  m_pixel_tolerance = 2.0;
  m_fps = 30.0; // FIXME!!!
  m_statistics_mode = 0;
  m_y_fov = 30*(3.14/180.0);
  m_planet_radius = 0;

  m_mean_fps = m_fps;
  m_frame_count = 0;
  m_elapsed_time = 0;
  m_light_direction = (cbdam::vector4_t(0.25f, 0.25f, 1.0f, 0.0f)).ok_normalized();
                                       
  m_camera_controller = 0;

  m_building_renderer_enabled = true;

  g_clock.restart();
  g_speed_clock.restart();
}

qgl_window_cbdam::~qgl_window_cbdam() {
  m_terrain_model->update_stop();

  m_renderer->release_graphics();
  delete m_camera_controller;
  delete m_renderer;
  delete m_terrain_model;
} 

void qgl_window_cbdam::initializeGL() {
  std::cerr << "initialize GL\n";
  std::size_t x_bytes = 72 * 1024 * 1024;
  std::cerr << "set texture cache capacity "  << sl::human_readable_size(x_bytes) << std::endl;
  m_renderer->set_texture_cache_capacity(x_bytes);
  m_renderer->init_opengl();
  set_initial_position();

  // set redraw timer to obtain m_fps frames per second
  startTimer( (int)(1000.0 / m_fps) );

  glClear(GL_ACCUM_BUFFER_BIT);
}

void qgl_window_cbdam::init_frame() {
  //  glClearColor(  0.36, 0.43, 0.64, 0.0);
  glClearColor(0.5f, 0.76f, 0.9f,  0.0f);
  glClearDepth( 1.0f );
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
}

void qgl_window_cbdam::enable_poligon_stipple() {
  static GLubyte halftone[] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55, 
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55};
  glPolygonStipple( halftone );
    
  glEnable( GL_POLYGON_STIPPLE );
}

void qgl_window_cbdam::paintGL(){
  init_frame();
  set_projection_matrix( width(), height() );
  sl::real_time_clock clock;
  clock.restart();

  const cbdam::camera::projective_map_t& P = m_camera.projection();
  cbdam::camera::rigid_body_map_t V =  m_camera.view();
  sl::point3d center = m_camera.position(); 
  center = m_terrain_model->uvh_xyz_transform()->xyz_on_ground(center);

  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadMatrixd(P.to_pointer());
  glMatrixMode(GL_MODELVIEW);

  glPushMatrix();
  sl::affine_map3d VM = V * sl::linear_map_factory3d::translation(center[0], center[1], center[2]); 
  glLoadMatrixd(VM.to_pointer());
  //  std::cerr << "camera V" << std::endl << V << std::endl;

  // lighting
  if (m_renderer->is_shading_enabled()) {
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_POSITION, m_light_direction.to_pointer());
    glLightfv(GL_LIGHT0, GL_AMBIENT,  sl::point4f(0.1f, 0.1f, 0.1f, 1.0f).to_pointer());
    glLightfv(GL_LIGHT0, GL_DIFFUSE,  sl::point4f(0.9f, 0.9f, 0.9f, 1.0f).to_pointer());
    glLightfv(GL_LIGHT0, GL_SPECULAR, sl::point4f(0.0f, 0.0f, 0.0f, 1.0f).to_pointer());
    glLighti(GL_LIGHT0, GL_SPOT_CUTOFF, 180);
	
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE); 
  } else {
    // No lighting
    glDisable(GL_LIGHTING);
    glDisable(GL_COLOR_MATERIAL);
  }
 
  m_renderer->draw(P,V,center);

  if (m_building_renderer.scene_pointer() != 0 && m_building_renderer_enabled) {
    // FIXME
    // Handle center in building renderer!!!
    glPushMatrix();
    glTranslated(-center[0], -center[1], -center[2]);
    //    cbdam::point3d_t p = m_building_hierarchy.node(0).bbox().center();
    m_building_renderer.render(P.to_pointer(),V.to_pointer());
    glPopMatrix();
  }

  if (m_current_intersection.first) {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
    glColor3f(1.0f, 0.2f, 0.2f);
    glPointSize(4);
    glBegin(GL_POINTS);
    glVertex3d(m_current_intersection.second[0]-center[0],
	       m_current_intersection.second[1]-center[1],
	       m_current_intersection.second[2]-center[2]);
    glEnd();
  }

  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  if ( m_statistics_mode > 0 ) {
    glFinish();
  }

  m_elapsed_time += clock.elapsed().as_microseconds();
  ++m_frame_count;
  if (m_elapsed_time > 100000 || m_frame_count > 25) {
    // update fps every 1/4 of second
    m_mean_fps = (float)m_frame_count * 1000000.0f / m_elapsed_time;
    m_frame_count = 0;
    m_elapsed_time = 0;

    cbdam::point3d_t position = m_camera.position();
    double distance = (position - m_old_position).two_norm();
    m_old_position = position;
    //  m_speed = (distance/1000.0f) / (g_speed_clock.elapsed().as_microseconds()/(1000000.0f*3600.0f)); // Km/h
    //  m_speed = (distance/1000.0f) / (g_speed_clock.elapsed().as_microseconds()/(1000000.0f)); // Km/s
    m_speed = (distance*1000.0f) / (float)(g_speed_clock.elapsed().as_microseconds()); // Km/s

    g_speed_clock.restart();
  }

  if ( m_statistics_mode > 0 ) {
    draw_statistics();
  }
}
 
void qgl_window_cbdam::resizeGL( int w, int h ) {
  m_renderer->set_screen_tolerance( m_pixel_tolerance / height() );
  m_building_renderer.screen_tolerance() = m_pixel_tolerance / height();

  // used to map mouse movement to trackball
  m_camera_controller->set_window_size( width(), height() );

  // redefine projection matrix in the camera
  // and load in opengl
  set_projection_matrix(w, h );

  // set viewport
  glMatrixMode( GL_PROJECTION );
  glViewport( 0, 0, (GLint)w, (GLint)h );
  glMatrixMode( GL_MODELVIEW );
  glLoadIdentity();

  glClear(GL_ACCUM_BUFFER_BIT);
}


void qgl_window_cbdam::set_projection_matrix(int w, int h) {
  // recompute near and far planes to include all the visible data
  float aspect_ratio = ( h==0 ) ? 1.0 :  (float)w / (float)h;
  cbdam::point3d_t position = m_camera.position();

  if (m_terrain_model->is_planar()) {
    float p_far = as_vector( position ).two_norm() + m_planet_radius;
    float p_near = p_far / 1000;
    m_camera.set_projection( m_y_fov, aspect_ratio, p_near, p_far);
  } else {
    // Get far plane as far to see at least highest peek over horizon
    // Compute the distance from the viewpoint of the farther visible point on the planet.
    // Suppos Rmin = radius, Rmax = radius + max height
    // Compute distance from viewpoint of the intersection on the Rmax sphere
    // of the ray passing from view point and tangent to the inner sphere
    // D = sqrt( D^2 - Rmin^2 ) + sqrt( Rmax^2 - Rmin^2 )
    float distance_two = as_vector( position ).two_norm_squared(); // distance from sphere center
    const float radius_two = m_planet_radius * m_planet_radius;

    // delta_radius = sqrt( max_radius^2 - min_radius^2 )
    float p_far = std::sqrt(distance_two - radius_two) * 1.1; 
    float p_near = p_far / 10000;
    m_camera.set_projection( m_y_fov, aspect_ratio, p_near, p_far);
  } 
}

void qgl_window_cbdam::timerEvent ( QTimerEvent * /* e */ ) {
  m_camera_controller->idle_update();
  updateGL();
}

void qgl_window_cbdam::mouseMoveEvent ( QMouseEvent * e ) { 
  if ( e->modifiers() & Qt::ShiftModifier ) {
    m_current_intersection = current_graph_intersection_from_cursor_position( e->pos().x(), e->pos().y());
    if (m_current_intersection.first) {std::cerr << "ray  intersection at " << m_current_intersection.second << "\n";}
  } else {
    if (  e->buttons() & (Qt::LeftButton | Qt::MidButton | Qt::RightButton )) {
      sl::fixed_size_point<2,int> pos( e->pos().x(), e->pos().y() );
      m_camera_controller->mouse_move( pos );
    }
  }
}

void qgl_window_cbdam::mousePressEvent ( QMouseEvent * e ) {
  // pass position and mouse button to the camera_controller
  // which apply changes to the camera
  sl::fixed_size_point<2,int> pos( e->pos().x(), e->pos().y() );
  switch( e->button() ) {
  case  Qt::LeftButton:
    m_camera_controller->mouse_pressed( pos, cbdam::camera_controller_vtrackball::CC_MB_LEFT );
    break;
  case  Qt::MidButton:
    m_camera_controller->mouse_pressed( pos, cbdam::camera_controller_vtrackball::CC_MB_MIDDLE );
    break;
  case Qt::RightButton:
    m_camera_controller->mouse_pressed( pos, cbdam::camera_controller_vtrackball::CC_MB_RIGHT );
    break;
  default:
    break;
  }
}

void qgl_window_cbdam::mouseReleaseEvent ( QMouseEvent * e ) {
  sl::fixed_size_point<2,int> pos( e->pos().x(), e->pos().y() );
  switch( e->button() ) {
  case  Qt::LeftButton:
    m_camera_controller->mouse_released( pos, cbdam::camera_controller_vtrackball::CC_MB_LEFT );
    break;
  case  Qt::MidButton:
    m_camera_controller->mouse_released( pos, cbdam::camera_controller_vtrackball::CC_MB_MIDDLE );
    break;
  case Qt::RightButton:
    m_camera_controller->mouse_released( pos, cbdam::camera_controller_vtrackball::CC_MB_RIGHT );
    break;
  default:
    break;
  }
}

void qgl_window_cbdam::print_commands() {
  warning_function( 0,  "+------------------------- help cbdam commands ------------------+." );
  warning_function( 0,  "mouse:." );
  warning_function( 0,  "  left\t - rotate planet." );
  warning_function( 0,  "  right\t - tilt camera up/down." );
  warning_function( 0,  "keyboard:." );
  warning_function( 0,  "  h\t - print this help." );
  warning_function( 0,  "  w\t - moves forward." );
  warning_function( 0,  "  s\t - moves backward." );
  warning_function( 0,  "  a\t - turn left." );
  warning_function( 0,  "  d\t - turn right." );
  warning_function( 0,  "  q\t - reset position." );
  warning_function( 0,  "  space\t - change texture layer." );
  warning_function( 0,  "  b\t - enable / disable draw bounding volumes." );
  warning_function( 0,  "  p\t - enable / disable patch color." );
  warning_function( 0,  "  c\t - enable / disable color." );
  warning_function( 0,  "  n\t - enable / disable wireframe." );
  warning_function( 0,  "  f\t - enable / disable statistics." );
  warning_function( 0,  "  e\t - enable / disable elevation map." );
  warning_function( 0,  "  l\t - enable / disable shading." );
  warning_function( 0,  "  o\t - enable / disable buildings occlusion culling." );
  warning_function( 0,  "  r\t - reset/print network stats." );
  warning_function( 0,  "  k\t - toggle between vbo/direct rendering.");
  warning_function( 0,  "  g\t - enable / disable fog.");
  warning_function( 0,  "  v\t - enable / disable adaptive tolerance.");
  warning_function( 0,  "  z-x\t - increase / decrease error tolerance." );
  warning_function( 0,  "  ]-[\t - increase / decrease rotation speed." );
  warning_function( 0,  "+--------------------------------------------------------------+." );
}

void qgl_window_cbdam::keyPressEvent( QKeyEvent *e )
{   
  static std::size_t active_color_layer_idx_ = 0;
  int key = e->key();
  if ( !e->isAutoRepeat() ) {
    switch( key ) {
    case Qt::Key_Escape:
      emit stop_rendering();
      break;
    case Qt::Key_4 :
      std::cerr << "STEREO DISABLED" << std::endl;
#if 0
      m_renderer->set_stereo_enabled( !m_renderer->is_stereo_enabled() );
      if ( m_renderer->is_stereo_enabled() ) {
#if FULLSCREEN_WORKING
	parentWidget()->showFullScreen();
#else
        parentWidget()->reparent(parentWidget()->parentWidget(), Qt::WStyle_Customize | Qt::WStyle_NoBorderEx, QPoint(0,0));
        parentWidget()->setGeometry(-1,0,2*1024+1,768);
        parentWidget()->show();
#endif
        std::cerr <<  "stereo draw\t\t\tenabled." << std::endl;
      } else {
#if FULLSCREEN_WORKING
        parentWidget()->showNormal();
#else
        parentWidget()->reparent(parentWidget()->parentWidget(), Qt::WStyle_Customize | Qt::WStyle_NormalBorder, QPoint(0,0));
        parentWidget()->resize(800, 600);
        parentWidget()->show();
#endif
	std::cerr <<  "stereo draw\t\t\tdisabled." << std::endl;
      }
#endif
      break;  
    case Qt::Key_H :
      print_commands();	
      break;
    case Qt::Key_L :
      m_renderer->set_shading_enabled(!m_renderer->is_shading_enabled());	
      std::cerr << "shading " << m_renderer->is_shading_enabled() << std::endl;
      break;
    case Qt::Key_G :
      std::cerr << "FOG DISABLE" << std::endl;
      break;
    case Qt::Key_V :
      m_renderer->set_adaptive_tolerance_enabled(!m_renderer->is_adaptive_tolerance_enabled());	
      std::cerr << std::endl << "adaptive tolerance " << m_renderer->is_adaptive_tolerance_enabled() << std::endl;
      break;
    case Qt::Key_Q :
      set_initial_position();
      break;
    case Qt::Key_T :      
      break;
    case Qt::Key_N :
      m_renderer->set_wireframe_enabled( !m_renderer->is_wireframe_enabled() );
      break;
    case Qt::Key_F :
      m_statistics_mode++;
      if (m_statistics_mode>2) m_statistics_mode = 0;
      break;
    case Qt::Key_B :
      m_renderer->set_draw_bounding_volumes_enabled(!m_renderer->is_draw_bounding_volumes_enabled());
      break;
    case Qt::Key_C :
      m_renderer->set_color_enabled(!m_renderer->is_color_enabled());
      break;
     case Qt::Key_P :
      m_renderer->set_patch_color_enabled(!m_renderer->is_patch_color_enabled());
      break;
    case Qt::Key_E :
      m_renderer->set_elevation_map_enabled(!m_renderer->is_elevation_map_enabled());
      break;
    case Qt::Key_0 :
      m_renderer->set_draw_enabled(!m_renderer->is_draw_enabled());
      break;
    case Qt::Key_R :
      if (thread_running) {
	m_terrain_model->update_stop();
      } else {
	m_terrain_model->update_start();
      }
      thread_running = !thread_running;
      std::cerr << "thread_running = " << thread_running << std::endl;
     break;
    case Qt::Key_O :
      m_building_renderer.set_occlusion_culling_enabled(!m_building_renderer.is_occlusion_culling_enabled());
     break;
    case Qt::Key_Space :
      if (m_terrain_model->overlay_color_layer_count()>0) {
	++active_color_layer_idx_;
	if (active_color_layer_idx_ >= m_terrain_model->overlay_color_layer_count()) {
	  active_color_layer_idx_ = 0;
	}

	for(std::size_t i = 0; i < m_terrain_model->overlay_color_layer_count(); ++i) {
	  m_terrain_model->set_overlay_color_layer_active(i, i == active_color_layer_idx_);
	}

	//	m_renderer->clear_texture_cache();
	std::cerr << "Set active overlay layer " << active_color_layer_idx_ << "/" << m_terrain_model->overlay_color_layer_count() << std::endl;
      }
      break;
    case Qt::Key_U :
      m_building_renderer_enabled = !m_building_renderer_enabled;
      break;
    default:
      break;
    }
  }


  // key autorepeat or not: manage repeatable keys
  switch( key ) {
  case Qt::Key_Z :
    //    m_pixel_tolerance *= 2.0;
    m_pixel_tolerance += 0.1;
    if (m_pixel_tolerance > 1024) {
      m_pixel_tolerance = 1024;
    }
    m_renderer->set_screen_tolerance( m_pixel_tolerance / height() );
    m_building_renderer.screen_tolerance() =  m_pixel_tolerance / height();
    break;
  case Qt::Key_X :
    //    m_pixel_tolerance *= 0.5;
    m_pixel_tolerance -= 0.1;
    if ( m_pixel_tolerance < 0.1 )
      m_pixel_tolerance = 0.1;
    m_renderer->set_screen_tolerance( m_pixel_tolerance / height() );
    m_building_renderer.screen_tolerance() =  m_pixel_tolerance / height();
    break;
  case Qt::Key_W :
    m_camera_controller->key_pressed( cbdam::camera_controller_vtrackball::CC_K_FORWARD );
    break;
  case Qt::Key_S :
    m_camera_controller->key_pressed( cbdam::camera_controller_vtrackball::CC_K_BACKWARD );
    break;
  case Qt::Key_A :
    m_camera_controller->key_pressed( cbdam::camera_controller_vtrackball::CC_K_LEFT );
    break;
  case Qt::Key_D :
    m_camera_controller->key_pressed( cbdam::camera_controller_vtrackball::CC_K_RIGHT );
    break;
  case Qt::Key_BracketRight :
    m_camera_controller->increase_rotation_factor();
    break;
  case Qt::Key_BracketLeft :
    m_camera_controller->decrease_rotation_factor();
    break;
  default:
    QWidget::keyPressEvent( e );
    break;
  }
}

void qgl_window_cbdam::keyReleaseEvent ( QKeyEvent * e ){
  if ( !e->isAutoRepeat () )
    switch( e->key() ) {
    case Qt::Key_W :
      m_camera_controller->key_released( cbdam::camera_controller_vtrackball::CC_K_FORWARD );
      break;
    case Qt::Key_S :
      m_camera_controller->key_released( cbdam::camera_controller_vtrackball::CC_K_BACKWARD );
      break;
    case Qt::Key_A :
      m_camera_controller->key_released( cbdam::camera_controller_vtrackball::CC_K_LEFT );
      break;
    case Qt::Key_D :
      m_camera_controller->key_released( cbdam::camera_controller_vtrackball::CC_K_RIGHT );
      break;
    default:
      e->ignore();  
      break;     
    } else {
      e->ignore();  
    }
}

void qgl_window_cbdam::insert_color_layer(const std::string& id, cbdam::geoimage_quad_fetcher* fetcher, 
					  std::size_t first_level, std::size_t last_level, double min_altitude, double max_altitude, 
					  bool is_base_layer, bool is_active) {
  if (m_terrain_model && m_terrain_model->is_connected()) {
    if (is_base_layer) {
      m_terrain_model->insert_base_color_layer(id, fetcher, first_level, last_level,  min_altitude,  max_altitude, is_active);
    } else {
      m_terrain_model->insert_overlay_color_layer(id, fetcher, first_level, last_level, min_altitude, max_altitude, is_active);
    }
  } else {
    SL_TRACE_OUT(-1) << "trying to insert texture layer with no geometry loaded" << std::endl;
  }
}

bool qgl_window_cbdam::open(const std::string& height_url) {
  bool result = false;

  std::string file_name = height_url + ".data";
  cbdam::cbdam_diamond_fetcher* geometry_fetcher = new  cbdam::cbdam_diamond_fetcher(file_name);
  geometry_fetcher->connect();
  if (geometry_fetcher->is_connected()) {
    m_terrain_model = new cbdam::terrain_model(geometry_fetcher);
    
    if (m_terrain_model->is_connected()) {  
      m_renderer = new cbdam::terrain_model_renderer(m_terrain_model);
      
      if (m_terrain_model->is_planar()) {
	std::cerr << "planar" << std::endl;
	const cbdam::planar_coordinate_transform* geo_xform = 
	  dynamic_cast<const cbdam::planar_coordinate_transform*>(m_terrain_model->uvh_xyz_transform());
	m_camera_controller = new cbdam::camera_controller_flight(&m_camera);
	m_planet_radius = geo_xform->bounding_rectangle().diagonal().two_norm()/std::sqrt(2.0);  
      } else {
	const cbdam::spherical_coordinate_transform* geo_xform = 
	  dynamic_cast<const cbdam::spherical_coordinate_transform*>(m_terrain_model->uvh_xyz_transform());
	m_planet_radius = geo_xform->radius();
	std::cerr << "planet radius " << m_planet_radius << std::endl;
	m_camera_controller = new cbdam::camera_controller_vtrackball(&m_camera);
      } 
      
      std::cerr << "update_start" << std::endl;
      m_terrain_model->update_start();
      thread_running = true;
      std::cerr << "update_started" << std::endl;
      
      print_commands();
      result = true;
    } else {
      std::cerr << "failed to load terrain model " << height_url << std::endl;
    }
  } else {
    std::cerr << "unable to connect fetcher to " << height_url << std::endl;
  }

  return result;
}

bool qgl_window_cbdam::init_buildings(const char* fname) {
  m_building_hierarchy.open_scene(fname);
  if (!m_building_hierarchy.last_operation_success()) {
    return false;
  } else {
    m_building_renderer.set_scene_pointer(&m_building_hierarchy);
    std::cerr << "Read building bsp with " << m_building_hierarchy.size() << " nodes\n";
    return true;
  }
}

void qgl_window_cbdam::set_initial_position() {
  // set offset value used to compute proper far projection plane
  float aspect_ratio = ( height()==0 ) ? 1.0 :  (float)width() / (float)height();
  m_camera_controller->set_radius(m_planet_radius);
  m_camera_controller->set_distance(m_planet_radius/(2.0f*tan(m_y_fov/2.0)*aspect_ratio));
  m_camera_controller->reset_rotation();
  const cbdam::planar_coordinate_transform* geo_xform = 
    dynamic_cast<const cbdam::planar_coordinate_transform*>(m_terrain_model->uvh_xyz_transform());
  if (geo_xform != 0) {
    // planar
    cbdam::camera_controller_flight* cc = dynamic_cast<cbdam::camera_controller_flight*>(m_camera_controller);
    if (cc != 0) {
      cbdam::point2d_t p = geo_xform->bounding_rectangle().center();
      cbdam::camera::vector3_t pos(p[0], p[1], m_planet_radius/(2.0f*tan(m_y_fov/2.0)*aspect_ratio));
      cc->set_position(pos);
      std::cerr << "set position " << pos[0] << " "  << pos[1] << " "  << pos[2] << std::endl;
    }
  }
}

void qgl_window_cbdam::draw_rectangle(float delta, float pos_y, double percent) {
  glEnable( GL_BLEND );
  glColor4f( 0.0f, 1.0f, 0.0f, 0.7f );
  float pos_x = 4*delta;
  float width  = (int)(10 * delta * percent);
  float height = delta / 2;

  glBegin( GL_QUADS );
  glVertex2f( pos_x, pos_y );
  glVertex2f( pos_x + width, pos_y );
  glVertex2f( pos_x + width, pos_y + height );
  glVertex2f( pos_x,  pos_y + height );
  glEnd();
  glDisable( GL_BLEND );
  glColor4f( 0.0f, 0.0f, 0.0f, 0.0f );
}

void qgl_window_cbdam::draw_statistics() {
  float k = 7;
  float view_w = width() * k;
  float view_h = height() * k;
  float delta = 200;
  // disable texture if enabled 
  glPushAttrib(GL_ENABLE_BIT);
  glDisable(GL_CULL_FACE);
  // disable texture
  //  glDisable(GL_TEXTURE_RECTANGLE_ARB);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, view_w, 0, view_h );
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glPushMatrix();
  glLoadIdentity();

  // antialiasing for the lines
  //  glEnable(GL_LINE_SMOOTH);

  glColor3f( 0.0f, 0.0f, 0.0f );

  float pos_x = delta / 4;
  float pos_y = delta + delta / 4;
  float delta_x = delta * 6;
  float delta_y = delta;
  float h = delta / 2.2;

  glLineWidth( 2 );
  //  glutil_begin_screen_coordinates_system();

  float rendered_triangles = m_renderer->stat_rendered_triangles() + m_building_renderer.stat_rendered_triangles();

  if (m_statistics_mode == 1) {

    pos_y = view_h-1*delta_y;
    pos_x = delta/4;
    glColor3f( 0.0f, 0.0f, 0.0f );
#if 0
    glutil_printf( pos_x+1*k, pos_y-1*k, h, "Hz: %.0f", (m_mean_fps));
    pos_x += 2*delta_x/3;
#endif
    glutil_printf( pos_x+1*k, pos_y-1*k, h, "Triangles/pixel: %.1f", rendered_triangles / (float)(width()*height()));

    pos_y = view_h-1*delta_y;
    pos_x = delta/4;
    glColor3f( 1.0f, 1.0f, 1.0f );
#if 0
    glutil_printf( pos_x+0*k, pos_y-0*k, h, "Hz: %.0f", (m_mean_fps));
    pos_x += 2*delta_x/3;
#endif
    glutil_printf( pos_x+0*k, pos_y-0*k, h, "Triangles/pixel: %.1f", rendered_triangles / (float)(width()*height()));

  } else {
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

    glColor3f( 0.0f, 0.0f, 0.0f );

    glutil_printf( pos_x, pos_y, h, "Pixel tol %2.1f", m_pixel_tolerance );
    pos_x += delta_x;

    glutil_printf( pos_x, pos_y, h, "Fps: %5.1f", m_mean_fps);
    pos_x += delta_x;

    glutil_printf( pos_x, pos_y, h, "Mtri/s %4.1f", rendered_triangles * m_mean_fps * 1E-6 );
    pos_x += delta_x;

    // get current frame statistic data
    glutil_printf(  pos_x, pos_y, h, "KTri/f %4.1f", rendered_triangles * 1E-3 );
    pos_x += delta_x;

    glutil_printf( pos_x, pos_y, h, "New/f %d", m_renderer->stat_frame_vbo_creation_count());
    // next row    
    pos_x = delta/4;    
    pos_y -= delta_y;

    glutil_printf( pos_x, pos_y, h, "Patches/f %d", m_renderer->stat_frame_vbo_creation_count()+ m_renderer->stat_frame_vbo_reuse_count());
    pos_x += delta_x;

    glutil_printf( pos_x, pos_y, h, "Tri/pix %4.2f", rendered_triangles / (float)(width()*height()));
    pos_x += delta_x;

    if (m_building_renderer.scene_pointer() != 0) {
      glutil_printf( pos_x, pos_y, h, "Builds %d", m_building_renderer.stat_rendered_objects());
      pos_x += delta_x;
    
      glutil_printf( pos_x, pos_y, h, "Querie %d", m_building_renderer.stat_issued_occlusion_queries());
      pos_x += delta_x;
    }

    cbdam::point3d_t viewpoint = m_camera.position();
    float height = m_terrain_model->is_planar() ? viewpoint[2] / 1000.0f : (viewpoint.as_vector().two_norm() - m_planet_radius)/1000.0f;
    glutil_printf( pos_x, pos_y, h, "H Km %4.1f", height);
    pos_x += delta_x;
    glutil_printf( pos_x, pos_y, h, "Km/s %4.1f", m_speed);
    pos_x += delta_x;

    //    cbdam::point3d_t uvh_viewpoint = m_terrain_model->uvh_from_xyz(viewpoint);
    //    std::cerr << "viewpoint " << viewpoint[0] << " " << viewpoint[1] <<  " " << viewpoint[2] << ", uv " << uvh_viewpoint[0] <<  " " << uvh_viewpoint[1] << std::endl;
    //    res = m_terrain_model->current_representation_ground_elevation_from_uv(cbdam::point2d_t(uvh_viewpoint[0], uvh_viewpoint[1]));
    std::pair<bool, double> res = m_terrain_model->current_representation_ground_elevation_from_xyz(viewpoint);
    if (res.first) {
      glutil_printf( pos_x, pos_y, h, "elv m %4.1f", res.second);	
      pos_x += delta_x;
    }
  }

  glLineWidth( 1 );

  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopAttrib();
}

std::pair<bool, cbdam::point3d_t> qgl_window_cbdam::current_graph_intersection_from_cursor_position(int x, int y) const {
  double x_pos,y_pos,z_pos;
  GLint xywh[4];
  glGetIntegerv( GL_VIEWPORT, xywh );
  double view[16];
  double proj[16];
  const double* camera_view = m_camera.view().to_pointer();
  const double* camera_proj = m_camera.projection().to_pointer();
  for(int i = 0; i < 16; ++i) {
    view[i] = camera_view[i];
    proj[i] = camera_proj[i];
  }
  int res = gluUnProject(x, xywh[3] - 1 - y, 1.0, view, proj, xywh, &x_pos, &y_pos, &z_pos);
  if (res) {  
    // test absolute intersection
    cbdam::point3d_t extremity(x_pos, y_pos, z_pos);
    cbdam::point3d_t origin = m_camera.position();
    // FIXME I3D
    //    std::pair<bool, double> res = m_terrain_model->current_representation_intersection(origin, extremity);
    std::pair<bool, double> res = m_terrain_model->current_representation_nearest_intersection_from_xyz(origin, extremity);
    if (res.first) {
      // convert t into r(t) = o + d * t
      return std::make_pair(true, origin + (extremity - origin) * res.second);
    } else {
      return std::make_pair(false, cbdam::point3d_t(0,0,0));
    }
  } else {
    std::cerr << "unable to unproject " << x << ", " << y << std::endl;
    return std::make_pair(false, cbdam::point3d_t(0,0,0));
  }
}

const cbdam::cbdam_diamond_fetcher* qgl_window_cbdam::elevation_fetcher() const {
  assert(m_terrain_model->is_connected());
  return m_terrain_model->elevation_layer()->fetcher();
}



