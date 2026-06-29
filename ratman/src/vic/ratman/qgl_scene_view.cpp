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
#include <QApplication>
#include <QCursor>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QPixmap>
#include <QPoint>
#include <QString>
#include <QTimerEvent>
#include <QWheelEvent>
#include <QWidget>

#include <vic/ratman/decorated_terrain_view.hpp>
#include <vic/ratman/qgl_scene_view.hpp>
#include <vic/cbdam/base/terrain_model.hpp>

#include "Icons/close_hand.xpm"
#include "Icons/open_hand.xpm"
#include "Icons/rot_3d.xpm"
#include <sl/clock.hpp>

#ifdef _WIN32
  #undef min
  #undef max
#endif

namespace ratman {

  qgl_scene_view::qgl_scene_view(ratman::decorated_terrain_view* terrain_view, 
				 const aabox3d_t& camera_uvh_bbox,
				 const oriented_position& reset_position,
				 QWidget *parent)
    : QGLWidget(parent), 
      terrain_view_(terrain_view), 
      camera_uvh_bbox_(camera_uvh_bbox),
      camera_controller_(uvh_xyz_transform()),
      last_mouse_move_event_(QEvent::None, QPoint(0,0), Qt::NoButton, Qt::NoButton, Qt::NoModifier)
  {

    QWidget::setFocusPolicy(Qt::StrongFocus); // enable keyboard events
    setMouseTracking(true); // enable tracking without button press
    cursors_.push_back(new QCursor(QPixmap(open_hand_xpm),16,16));
    cursors_.push_back(new QCursor(QPixmap(close_hand_xpm),16,16));
    cursors_.push_back(new QCursor(QPixmap(rot_3d_xpm),16,16));
    current_cursor_=0;

    animation_is_active_ = false;
    
    camera_controller_.set_initial_oriented_position(reset_position);
    camera_controller_.reset();

    // Initial animation
    const ratman::oriented_position start_pos = constrained_position(camera_controller_.get_oriented_position());
    ratman::oriented_position end_pos(start_pos);	// set same parameters of start pos
    end_pos.set_distance_from_target(0.5*end_pos.distance_from_target());
    //std::cerr << "animation from " << std::endl << start_pos << std::endl << " to " << std::endl << end_pos << std::endl;
    previous_eye_position_ = point3d_t(0.0, 0.0, 0.0);
    speed_clock_.restart();
    speed_km_h_ = 0.0;

    goto_location(end_pos.position_xyz());
  }

  qgl_scene_view::~qgl_scene_view() {
    makeCurrent();
  }

  const cbdam::coordinate_transform* qgl_scene_view::uvh_xyz_transform() const {
    return terrain()->uvh_xyz_transform();
  }

  const cbdam::terrain_model* qgl_scene_view::terrain() const {
    assert(terrain_view());
    assert(terrain_view()->terrain_layer());
    
    return terrain_view()->terrain_layer()->model();
  }

  oriented_position qgl_scene_view::constrained_position(const oriented_position& op) {
    oriented_position result = op;
    
    // 1. ground constraint
    point3d_t p0_ground_xyz = op.ground_target_xyz();
    point3d_t p0_ground_uvh = uvh_xyz_transform()->uvh_from_xyz(p0_ground_xyz);

    const double min_u = camera_uvh_bbox_[0][0];
    const double max_u = camera_uvh_bbox_[1][0];
    const double min_v = camera_uvh_bbox_[0][1];
    const double max_v = camera_uvh_bbox_[1][1];

    point3d_t p0_constrained_uvh = point3d_t(sl::median(p0_ground_uvh[0], min_u, max_u),
					     sl::median(p0_ground_uvh[1], min_v, max_v),
					     p0_ground_uvh[2]);
    if (!p0_constrained_uvh.is_epsilon_equal(p0_ground_uvh, 0.0001)) {
      //      std::cerr << "constrained " << p0_constrained_uvh[0] << " " << p0_constrained_uvh[1] << " " << p0_constrained_uvh[2] << " "
      //		<<  p0_ground_uvh[0] << " " << p0_ground_uvh[1] << " " << p0_ground_uvh[2] << " " << std::endl;
      point3d_t p0_constrained_xyz = uvh_xyz_transform()->xyz_from_uvh(p0_constrained_uvh);
      result.move_ground_position_to_xyz(p0_constrained_xyz);
      //      std::cerr << "ground constraint from " << p0_ground_uvh[0] << ", " << p0_ground_uvh[1] << ", " << p0_ground_uvh[2] << " TO " << p0_constrained_uvh[0] << ", " << p0_constrained_uvh[1] << ", " << p0_constrained_uvh[2] 
      //		<< " XYZ POS " << result.position_xyz()[0] << ", " << result.position_xyz()[1] << ", " << result.position_xyz()[2] << std::endl;
    }

    // 2. eye constraint
    // 1. Use uvh box 
    // xyz -> uvh
    point3d_t p0_orig_xyz = result.position_xyz();
    point3d_t p0_orig_uvh = uvh_xyz_transform()->uvh_from_xyz(p0_orig_xyz);

    // Find terrain elevation
    std::pair<bool, double> r = terrain_view_->terrain_layer()->model()->current_representation_ground_elevation_from_uv(point2d_t(p0_orig_uvh[0],
																   p0_orig_uvh[1]));
    const double terrain_h = (!r.first) ? 0.0 : r.second;

    //    std::cerr << " H = " << terrain_h << std::endl;

    // Determine coordinate range
    const double delta_h = 200.0; // m FIXME
    const double min_h = std::max(camera_uvh_bbox_[0][2], terrain_h+delta_h);
    const double max_h = std::max(camera_uvh_bbox_[1][2], min_h);

    p0_constrained_uvh = point3d_t(sl::median(p0_orig_uvh[0], min_u, max_u),
				   sl::median(p0_orig_uvh[1], min_v, max_v),
				   sl::median(p0_orig_uvh[2], min_h, max_h));
    if (p0_constrained_uvh != p0_orig_uvh) {
      // Apply constraint

      // Convert back to xyz
      const point3d_t p0_constrained_xyz = uvh_xyz_transform()->xyz_from_uvh(p0_constrained_uvh);
      
      // Convert to "Oriented position"
      // Camera at p0_constrained looking at ground_target
      
      result.set_position_xyz(p0_constrained_xyz);

      if (p0_constrained_uvh[2] == p0_orig_uvh[2]) {
	// constraint has been applied to uv, hence stop movement
	camera_controller_.stop_movement();
      }
    }

    return result;
  }

  void qgl_scene_view::timerEvent ( QTimerEvent * /* e */ ) {
    update_position_labels();
    updateGL();
  }

  void qgl_scene_view::initializeGL() {
    //std::cerr << "############# TIMER  START" << std::endl;
    startTimer( (int)(1000.0 / 25.0) );
    set_current_cursor(0);
  }

  void qgl_scene_view::paintGL() {
    sl::real_time_clock stat_clock_full;
    sl::real_time_clock stat_clock;

    double stat_time_paint_full;
    double stat_time_paint_pre;
    double stat_time_paint_draw;

    
    stat_clock_full.restart();

    stat_clock.restart();
    //  std::cerr << "############# PAINTGL START" << std::endl;
    if (camera_animation_.is_active()) {
      if (!animation_is_active_) {
	emit signal_animation_begin();
	animation_is_active_ = true;
      }
      camera_controller_.set_oriented_position(camera_animation_.current_position_xyz());
    } else {
      if (animation_is_active_) {
	emit signal_animation_end();
	animation_is_active_ = false;
      }

      camera_controller_.idle_update();
    }
    
    // --------- FIXME -- constraint
    const oriented_position op = camera_controller_.get_oriented_position();
    oriented_position op_constrained = constrained_position(op);

    // update speed
    point3d_t current_eye = op_constrained.position_xyz();
    speed_km_h_ = (current_eye - previous_eye_position_).two_norm() / (speed_clock_.elapsed().as_milliseconds()/3600.0);
    previous_eye_position_ = current_eye;
    speed_clock_.restart();

    if (op != op_constrained) {
      camera_controller_.set_oriented_position(op_constrained);
      if (animation_is_active_) {
	// FIXME
	// FIXME std::cerr << "Adding track point!" << std::endl;
	// FIXME camera_animation_.add_track_point(camera_animation_.current_time(), op_constrained); 
      }
    }

    // -------- Mouse over?
    if ((camera_controller_.mouse_status() == camera_controller::MS_RELEASED) &&
        (last_mouse_move_event_.type() == QEvent::MouseMove)) {
      terrain_view_->on_event(*this,&last_mouse_move_event_);
    }
    
    stat_time_paint_pre = stat_clock.elapsed().as_milliseconds();

    stat_clock.restart();
    terrain_view_->set_cameraV(camera_controller_.camera_view(),
			       camera_controller_.get_oriented_position().ground_target_xyz());

    terrain_view_->render(*this);

    stat_time_paint_draw = stat_clock.elapsed().as_milliseconds();

    stat_time_paint_full = stat_clock_full.elapsed().as_milliseconds();

    if (stat_time_paint_full > 30) {
      SL_TRACE_OUT(1) <<
	"FRAME: " <<
	" full: " << stat_time_paint_full <<
	" pre: " << stat_time_paint_pre <<
	" draw: " << stat_time_paint_draw <<
	std::endl;
    }
  }

  void qgl_scene_view::resizeGL(int width, int height) {
    glViewport( 0, 0, (GLint)width, (GLint)height );
    terrain_view_->set_viewport(width, height);
  }

  bool qgl_scene_view::event(QEvent *e) {
    bool result = false;
    if (camera_controller_.mouse_status() != camera_controller::MS_RELEASED) {
      // Focus camera controller
      result = super_t::event(e); // Will go to camera controller
    } else {
      // No focus
      if (e->type() == QEvent::MouseMove) {    
        QMouseEvent* mouse_event = dynamic_cast<QMouseEvent*>(e);
        if (mouse_event->button() == Qt::NoButton) {
          // Move without press - will be handled by timer
          last_mouse_move_event_ = *dynamic_cast<QMouseEvent*>(e);
          result = true;
        } 
      }
      if (!result) {
        result=terrain_view_->on_event(*this,e);
        if (!result) {
          result = super_t::event(e);
        }
      }
    }
    return result;
  }

  void qgl_scene_view::mousePressEvent(QMouseEvent* e) {
    ratman::camera_controller::MOUSE_STATUS button = ratman::camera_controller::MS_RELEASED;
    switch(e->button()) {
    case Qt::LeftButton:
      button = ratman::camera_controller::MS_LEFT;
      set_current_cursor(1);
      break;
    case Qt::MidButton:
      button = ratman::camera_controller::MS_MIDDLE;
      break;
    case Qt::RightButton:
      button = ratman::camera_controller::MS_RIGHT;
      set_current_cursor(2);
      break;
    default:
      break;
    }
  
    camera_animation_.stop();
    camera_controller_.mouse_press(e->pos().x(), e->pos().y(), button);
  }

  void qgl_scene_view::mouseDoubleClickEvent(QMouseEvent* e) {
    if (e->button() == Qt::LeftButton) {
      std::pair<bool, ratman::point3d_t> res = terrain_intersection_from_cursor_position(e->pos().x(), e->pos().y());
      if (res.first) {
	//std::cerr << "intersection at " << res.second[0] << ", "  << res.second[1] << ", "  << res.second[2] << std::endl;
	go_towards_location(res.second);
      } else {
	//std::cerr << "missing intersection\n";
      }
    }
  }
  
  void qgl_scene_view::mouseMoveEvent(QMouseEvent* e) {
    camera_controller_.mouse_move(e->pos().x(), e->pos().y());
  }

  void qgl_scene_view::mouseReleaseEvent (QMouseEvent* e) {
    ratman::camera_controller::MOUSE_STATUS button = ratman::camera_controller::MS_RELEASED;
    switch(e->button()) {
    case Qt::LeftButton:
      button = ratman::camera_controller::MS_LEFT;
      break;
    case Qt::MidButton:
      button = ratman::camera_controller::MS_MIDDLE;
      break;
    case Qt::RightButton:
      button = ratman::camera_controller::MS_RIGHT;
      break;
    default:
      break;
    }
    camera_controller_.mouse_release(button);
    set_current_cursor(0);
  }

  void qgl_scene_view::wheelEvent(QWheelEvent *e) {
    camera_controller_.wheel_tick(e->delta());
  }


  void qgl_scene_view::keyPressEvent(QKeyEvent* e) {
    int key = e->key();
    switch(key) {
    case Qt::Key_W:
      camera_controller_.key_press(ratman::camera_controller::K_ZOOMIN);
      break;
    case Qt::Key_S:
      camera_controller_.key_press(ratman::camera_controller::K_ZOOMOUT);
      break;
    case Qt::Key_Right:
      camera_controller_.key_press(ratman::camera_controller::K_RIGHT);
      break;
    case Qt::Key_Left:
      camera_controller_.key_press(ratman::camera_controller::K_LEFT);
      break;
    case Qt::Key_Up:
      camera_controller_.key_press(ratman::camera_controller::K_FORWARD);
      break;
    case Qt::Key_Down:
      camera_controller_.key_press(ratman::camera_controller::K_BACKWARD);
      break;
    case Qt::Key_N:
      camera_controller_.key_press(ratman::camera_controller::K_YAW_CCW);
      break;
    case Qt::Key_M:
      camera_controller_.key_press(ratman::camera_controller::K_YAW_CW);
      break;
    case Qt::Key_PageUp:
      camera_controller_.key_press(ratman::camera_controller::K_TILT_UP);
      break;
    case Qt::Key_PageDown:
      camera_controller_.key_press(ratman::camera_controller::K_TILT_DOWN);
      break;
    case Qt::Key_Space:
      reset();
      break;
    case Qt::Key_L:
      terrain_view_->terrain_layer()->set_shading_enabled(!terrain_view_->terrain_layer()->is_shading_enabled());
      break;
    default:
      break;
    }
  }

  void qgl_scene_view::keyReleaseEvent(QKeyEvent* e) {
    int key = e->key();
    switch(key) {
    case Qt::Key_W:
      camera_controller_.key_release(ratman::camera_controller::K_ZOOMIN);
      break;
    case Qt::Key_S:
      camera_controller_.key_release(ratman::camera_controller::K_ZOOMOUT);
      break;
    case Qt::Key_Right:
      camera_controller_.key_release(ratman::camera_controller::K_RIGHT);
      break;
    case Qt::Key_Left:
      camera_controller_.key_release(ratman::camera_controller::K_LEFT);
      break;
    case Qt::Key_Up:
      camera_controller_.key_release(ratman::camera_controller::K_FORWARD);
      break;
    case Qt::Key_Down:
      camera_controller_.key_release(ratman::camera_controller::K_BACKWARD);
      break;
    case Qt::Key_N:
      camera_controller_.key_release(ratman::camera_controller::K_YAW_CCW);
      break;
    case Qt::Key_M:
      camera_controller_.key_release(ratman::camera_controller::K_YAW_CW);
      break;
    case Qt::Key_PageUp:
      camera_controller_.key_release(ratman::camera_controller::K_TILT_UP);
      break;
    case Qt::Key_PageDown:
      camera_controller_.key_release(ratman::camera_controller::K_TILT_DOWN);
      break;
    default:
      break;
    }
  }


  void qgl_scene_view::zoom_in_pressed(){
    //std::cerr << "ZOOM IN PR" << std::endl;
    camera_animation_.stop();
    camera_controller().key_press(ratman::camera_controller::K_ZOOMIN);
  }

  void qgl_scene_view::zoom_in_released(){
    //std::cerr << "ZOOM IN REL" << std::endl;
    camera_controller().key_release(ratman::camera_controller::K_ZOOMIN);
  }

  void qgl_scene_view::zoom_out_pressed(){
    //std::cerr << "ZOOM OUT PR" << std::endl;
    camera_animation_.stop();
    camera_controller().key_press(ratman::camera_controller::K_ZOOMOUT);
  }

  void qgl_scene_view::zoom_out_released(){
    //std::cerr << "ZOOM OUT REL" << std::endl;
    camera_controller().key_release(ratman::camera_controller::K_ZOOMOUT);
  }

  void qgl_scene_view::right_pressed(){
    camera_animation_.stop();
    //std::cerr << "RIGHT PR" << std::endl;
    camera_controller().key_press(ratman::camera_controller::K_RIGHT);
  }

  void qgl_scene_view::right_released(){
    //std::cerr << "RIGHT REL" << std::endl;
    camera_controller().key_release(ratman::camera_controller::K_RIGHT);
  }

  void qgl_scene_view::left_pressed(){
    camera_animation_.stop();
    //std::cerr << "LEFT PR" << std::endl;
    camera_controller().key_press(ratman::camera_controller::K_LEFT);
  }

  void qgl_scene_view::left_released(){
    //std::cerr << "LEFT REL" << std::endl;
    camera_controller().key_release(ratman::camera_controller::K_LEFT);
  }

  void qgl_scene_view::forward_pressed(){
    camera_animation_.stop();
    //std::cerr << "FORWARD PR" << std::endl;
    camera_controller().key_press(ratman::camera_controller::K_FORWARD);
  }

  void qgl_scene_view::forward_released(){
    //std::cerr << "FORWARD REL" << std::endl;
    camera_controller().key_release(ratman::camera_controller::K_FORWARD);
  }

  void qgl_scene_view::backward_pressed(){
    camera_animation_.stop();
    //std::cerr << "BACKWARD PR" << std::endl;
    camera_controller().key_press(ratman::camera_controller::K_BACKWARD);
  }

  void qgl_scene_view::backward_released(){
    //std::cerr << "BACKWARD REL" << std::endl;
    camera_controller().key_release(ratman::camera_controller::K_BACKWARD);
  }

  void qgl_scene_view::yaw_ccw_pressed(){
    camera_animation_.stop();
    //std::cerr << "YAW_CCW PR" << std::endl;
    camera_controller().key_press(ratman::camera_controller::K_YAW_CCW);
  }

  void qgl_scene_view::yaw_ccw_released(){
    //std::cerr << "YAW_CCW REL" << std::endl;
    camera_controller().key_release(ratman::camera_controller::K_YAW_CCW);
  }

  void qgl_scene_view::yaw_cw_pressed(){
    camera_animation_.stop();
    //std::cerr << "YAW_CW PR" << std::endl;
    camera_controller().key_press(ratman::camera_controller::K_YAW_CW);
  }

  void qgl_scene_view::yaw_cw_released(){
    //std::cerr << "YAW_CW REL" << std::endl;
    camera_controller().key_release(ratman::camera_controller::K_YAW_CW);
  }

  void qgl_scene_view::tilt_up_pressed(){
    camera_animation_.stop();
    //std::cerr << "TILT_UP PR" << std::endl;
    camera_controller().key_press(ratman::camera_controller::K_TILT_UP);
  }

  void qgl_scene_view::tilt_up_released(){
    //std::cerr << "TILT_UP REL" << std::endl;
    camera_controller().key_release(ratman::camera_controller::K_TILT_UP);
  }

  void qgl_scene_view::tilt_down_pressed(){
    camera_animation_.stop();
    //std::cerr << "TILT_DOWN PR" << std::endl;
    camera_controller().key_press(ratman::camera_controller::K_TILT_DOWN);
  }

  void qgl_scene_view::tilt_down_released(){
    //std::cerr << "TILT_DOWN REL" << std::endl;
    camera_controller().key_release(ratman::camera_controller::K_TILT_DOWN);
  }

  void qgl_scene_view::set_tilt(int value) {
    camera_animation_.stop();
    //std::cerr << "TILT_SLIDER " << value << std::endl;
    if(camera_controller().tilt_slider_factor() != value) {
      //std::cerr << "TILT_SLIDER CHANGED" << std::endl;    
      camera_controller().set_tilt_slider_factor(value);
    }
  }

  void qgl_scene_view::reset_tilt() {
    //std::cerr << "RESET TILT_SLIDER " << std::endl;
    set_tilt(0);
    emit signal_tilt_reset(0);
  }

  void qgl_scene_view::set_zoom_factor(int value) {
    camera_animation_.stop();
    if(camera_controller().zoom_slider_factor() != value) {
      camera_controller().set_zoom_slider_factor(value);
    }
  }

  void qgl_scene_view::reset_zoom_factor() {
    //  std::cerr << "RESET ZOOM_SLIDER " << std::endl;
    set_zoom_factor(0);
    emit signal_zoom_reset(0);
  }

  void qgl_scene_view::reset(){
    camera_animation_.stop();
    //  std::cerr << "RESET" << std::endl;

    const ratman::oriented_position start_pos = constrained_position(camera_controller_.get_oriented_position());
    camera_controller_.reset();
    ratman::oriented_position end_pos = camera_controller_.get_oriented_position();
    
    if (rad2deg(start_pos.yaw()) > 180.0) {
      end_pos.set_yaw(deg2rad(360.0));
    } else {
      end_pos.set_yaw(deg2rad(0.0));
    }

    // Enforce constraints
    end_pos = constrained_position(end_pos);
    
    //camera_animation_.compute_track_from_to(start_pos, end_pos, 7000);
    camera_animation_.begin_track();
    camera_animation_.add_track_point(0,    start_pos);
    camera_animation_.add_track_point(100,  start_pos);
    camera_animation_.add_track_point(4900, end_pos);
    camera_animation_.add_track_point(5000, end_pos);
    camera_animation_.end_track();
   
    camera_animation_.start();
  }

  void qgl_scene_view::reset_north_pressed() {
    //  std::cerr << "NORTH PRESSED" << std::endl;
    camera_animation_.stop();
    const ratman::oriented_position start_pos = constrained_position(camera_controller_.get_oriented_position());
    ratman::oriented_position end_pos = start_pos;

    point3d_t point_on_ground = end_pos.local_to_global_xform() * point3d_t(0.0, 0.0, 0.0);
    rigid_body_map3d_t north_frame = uvh_xyz_transform()->xyz_local_to_global_from_xyz(point_on_ground);
    end_pos.set_local_to_global_xform(north_frame);

    if (rad2deg(start_pos.yaw()) > 180.0) {
      end_pos.set_yaw(deg2rad(360.0));
    } else {
      end_pos.set_yaw(deg2rad(0.0));
    }
    // Enforce constraints
    end_pos = constrained_position(end_pos);
    // Compute animation and start it
    camera_animation_.begin_track();
    camera_animation_.add_track_point(0.0, start_pos);
    camera_animation_.add_track_point(fabs(start_pos.yaw() - end_pos.yaw()) * 2000.0, end_pos);
    camera_animation_.end_track();
    camera_animation_.start();
  }

  void qgl_scene_view::reset_tilt_pressed() {
    //  std::cerr << "RESET TILT PRESSED" << std::endl;
    camera_animation_.stop();
    const ratman::oriented_position start_pos = constrained_position(camera_controller_.get_oriented_position());
    ratman::oriented_position end_pos = start_pos;
    end_pos.set_tilt(0.0);
    // Enforce constraints
    end_pos = constrained_position(end_pos);
    // Compute animation and start it
    camera_animation_.compute_track_from_to(start_pos, end_pos, fabs(start_pos.tilt()) * 2000.0f);
    camera_animation_.start();
  }

  void qgl_scene_view::go_towards_location(const point3d_t& target_pos) {
    const ratman::oriented_position start_pos = constrained_position(camera_controller_.get_oriented_position());
    ratman::oriented_position end_pos(start_pos);	// set same parameters of start pos
    
    end_pos.set_ground_target_xyz(target_pos); // projected on ground
    end_pos.set_tilt(0.0);

    double start_h = uvh_xyz_transform()->altitude_from_xyz(start_pos.position_xyz());
    double target_h = uvh_xyz_transform()->altitude_from_xyz(target_pos);

    // heuristic: we want to go down toward end pos, but not to far from starting height
    end_pos.set_distance_from_target(std::max(start_h * 0.25, 
					      target_h + 400));   // End height + 500m


    // Enforce constraints
    end_pos = constrained_position(end_pos);

    // Compute travel distance and time
    const double h_travel = 0.5*(start_h+target_h);
    const double h_max = 500000; 
    const double h_factor = std::min(1.0, h_travel/h_max);
    const double desired_speed_m_s = 3000/15 + 3000000/15*h_factor; // m/s
    const double dist_m = end_pos.position_xyz().distance_to(start_pos.position_xyz()); 
    double time_s = sl::median(4.0, 10.0, dist_m/desired_speed_m_s);

    // Compute animation and start it
    //std::cerr << "go from " << std::endl << start_pos << std::endl << "to" << std::endl << end_pos << std::endl; 
    camera_animation_.compute_track_from_to(start_pos, end_pos, time_s*1000);
    camera_animation_.start();
  }

  void qgl_scene_view::goto_location(const point3d_t& target_pos) {
    const ratman::oriented_position start_pos = constrained_position(camera_controller_.get_oriented_position());
    ratman::oriented_position end_pos(start_pos);	// set same parameters of start pos
    
    end_pos.set_ground_target_xyz(target_pos); // projected on ground
    end_pos.set_tilt(0.0);

    double start_h = uvh_xyz_transform()->altitude_from_xyz(start_pos.position_xyz());
    double target_h = uvh_xyz_transform()->altitude_from_xyz(target_pos);

    // heuristic: we want to go down toward end pos, but not to far from starting height
    end_pos.set_distance_from_target(target_h + 100);   // End height + 500m

    // Enforce constraints
    end_pos = constrained_position(end_pos);

    // Compute travel distance and time
    const double h_travel = 0.5*(start_h+target_h);
    const double h_max = 500000; 
    const double h_factor = std::min(1.0, h_travel/h_max);
    const double desired_speed_m_s = 3000/15 + 3000000/15*h_factor; // m/s
    const double dist_m = end_pos.position_xyz().distance_to(start_pos.position_xyz()); 
    
    double time_s = sl::median(4.0, 20.0, dist_m/desired_speed_m_s);

    // Compute animation and start it
    camera_animation_.compute_track_from_to(start_pos, end_pos, time_s*1000);
    camera_animation_.start();
  }

  void qgl_scene_view::goto_oriented_location(const point3d_t& target_pos, const point3d_t& orientation){
    // FIXME USE goto_location
    goto_location(target_pos);
    return;

    const ratman::oriented_position start_pos = constrained_position(camera_controller_.get_oriented_position());
    ratman::oriented_position end_pos(start_pos);	// initialize with the same parameters of start pos
    
   
    end_pos.set_ground_target_xyz(target_pos); // projected on ground
    end_pos.set_distance_from_target(orientation[0]);
    end_pos.set_tilt(orientation[1]);
    end_pos.set_yaw(orientation[2]);

    double start_h = uvh_xyz_transform()->altitude_from_xyz(start_pos.position_xyz());
    double target_h = uvh_xyz_transform()->altitude_from_xyz(target_pos);

    // Enforce constraints
    //end_pos = constrained_position(end_pos);

    //std::cerr << "qgl end oriented position "<<end_pos <<std::endl;

    // Compute travel distance and time
    const double h_travel = 0.5*(start_h+target_h);
    const double h_max = 500000; 
    const double h_factor = std::min(1.0, h_travel/h_max);
    const double desired_speed_m_s = 3000/15 + 3000000/15*h_factor; // m/s
    const double dist_m = end_pos.position_xyz().distance_to(start_pos.position_xyz()); 
    
    double time_s = sl::median(0.0, 12.0, dist_m/desired_speed_m_s);

    // Compute animation and start it
    std::cerr << "qgl_scene_view::goto_oriented_location()" << std::endl;
    std::cerr << "START" << std::endl << start_pos << std::endl;
    std::cerr << "END" << std::endl << end_pos << std::endl;
    std::cerr << "h_travel " << h_travel << " h_factor " << h_factor << " desired_speed_m_s " << desired_speed_m_s << " dist_m " << dist_m << " time_s " << time_s << std::endl;
    camera_animation_.compute_fly_from_to(start_pos, end_pos, time_s*1000);
    camera_animation_.start();
  }

  void qgl_scene_view::fly_from_to_oriented_location(const oriented_position& init_pos, const oriented_position& target_pos){
    const double desired_speed_m_s = 280.0; //approx 1000 Km/h
    const double dist_m = target_pos.position_xyz().distance_to(init_pos.position_xyz()); 
    double time_ms = 1000.0 * sl::median(1.0, 16.0, dist_m/desired_speed_m_s); 
    
    camera_animation_.compute_fly_from_to(init_pos,target_pos,time_ms);
    camera_animation_.start();
  }

  void qgl_scene_view::update_position_labels() {
    ratman::point3d_t pos = current_WGS84_lonlat_position();
    QString lat_str;
    lat_str.setNum(pos[1], 'f', 6); // LAT
    lat_str += QApplication::translate("MainWindow", "\302\260", 0);
    
    QString lon_str;
    lon_str.setNum(pos[0], 'f', 6); // LON
    lon_str += QApplication::translate("MainWindow", "\302\260", 0);
    
    QString ele_str;
    {
      double z = pos[2]; // ELV
      if (z>= 1000.0) {
	ele_str.setNum(z/1000.0, 'f', 2);
	ele_str += " Km";
      } else {
	ele_str.setNum(z, 'f', 0);
	ele_str += " m";
      }
    }
    
    QString quota_str;
    {
      // ALTITUDE
      double z = camera_controller_.camera_altitude();
      if (z>= 1000.0) {
	quota_str.setNum(z/1000.0, 'f', 2);
	quota_str += " Km";
      } else {
	quota_str.setNum(z, 'f', 0);
	quota_str += " m";
      }
    }

    QString speed_str;
#if 0
    speed_km_h_ = camera_controller_.current_speed_m_s()*3600.0/1000.0;
#endif
    speed_str.setNum(speed_km_h_, 'f', 0);
    speed_str += " Km/h";

    QString yaw_str;
    double yaw = rad2deg(camera_controller_.get_oriented_position().yaw());
    yaw_str.setNum(yaw, 'f', 1);
    yaw_str += QApplication::translate("MainWindow", "\302\260", 0);
    
    QString tilt_str;
    tilt_str.setNum(rad2deg(camera_controller_.get_oriented_position().tilt()) - 90.0, 'f', 1);
    tilt_str += QApplication::translate("MainWindow", "\302\260", 0);
    
    emit signal_latitute_changed(lat_str);
    emit signal_longitude_changed(lon_str);
    emit signal_elevation_changed(ele_str);
    emit signal_quota_changed(quota_str);
    emit signal_speed_changed(speed_str);
    emit signal_yaw_changed(yaw_str);
    emit signal_tilt_changed(tilt_str);
  }

  void qgl_scene_view::handling_event(void * /*data*/) {
    // DO NOTHING
  }

  ratman::point3d_t qgl_scene_view::current_WGS84_lonlat_position() const {
    point3d_t camera_position_xyz = camera_controller_.camera_position_xyz();
    point3d_t camera_position_uvh = terrain_view_->terrain_layer()->model()->uvh_from_xyz(camera_position_xyz);
    // Project on terrain
    //std::pair<bool, double> r = std::make_pair(false,0.0);

    std::pair<bool, double> r = terrain_view_->terrain_layer()->model()->current_representation_ground_elevation_from_uv(point2d_t(camera_position_uvh[0],
												       camera_position_uvh[1]));
    if (!r.first) {r.second = 0;}
    point3d_t ground_position_uvh = point3d_t(camera_position_uvh[0],
					      camera_position_uvh[1],
					      r.second);
    // Transform to WGS84
    return terrain_view_->terrain_layer()->model()->WGS84_lonlat_from_uvh(ground_position_uvh);
  }

  std::pair<ratman::point2d_t,double> qgl_scene_view::current_WGS84_lonlat_center_radius() const {
    point3d_t camera_position_xyz = camera_controller_.get_oriented_position().ground_target_xyz(); // FIXME camera_position_xyz();
    point3d_t camera_position_WGS84_lonlat = terrain_view_->terrain_layer()->model()->WGS84_lonlat_from_xyz(camera_position_xyz);
    
    // Estimate range in degrees
    double h = camera_controller_.get_oriented_position().distance_from_target(); // FIXME camera_controller_.camera_altitude();
    const double R = 6370000; // FIXME Hardcoded Earth radius - to make it work also for planar 
    const double half_dim = sl::median(0.01,90.0,rad2deg(0.5*h/R)); 

    return std::make_pair(point2d_t(camera_position_WGS84_lonlat[0],
				    camera_position_WGS84_lonlat[1]),
			  half_dim);
  }

  std::pair<bool, ratman::point3d_t> qgl_scene_view::terrain_intersection_from_cursor_position(int x, int y) const {
    double x_pos,y_pos,z_pos;
    GLint xywh[4];
    xywh[0]=0;
    xywh[1]=0;
    xywh[2]=width();
    xywh[3]=height();
    int res = gluUnProject(x, xywh[3] - 1 - y, 1.0, terrain_view_->cameraV().to_pointer(), terrain_view_->cameraP().to_pointer(), xywh, &x_pos, &y_pos, &z_pos);
    point3d_t extremity(x_pos, y_pos, z_pos);

    if (res) {  
      // test absolute intersection
      ratman::point3d_t origin = camera_controller_.camera_position_xyz();
      //std::cerr << "RAY from " << origin[0] << " " << origin[1] << " " << origin[2] << " TO " << extremity[0] << " " << extremity[1] << " " << extremity[2] << std::endl;
      std::pair<bool, double> res = terrain_view_->terrain_layer()->model()->current_representation_nearest_intersection_from_xyz(origin,
																  extremity);
      if (res.first) {
        // convert t into r(t) = o + d * t
        return std::make_pair(true, origin + (extremity - origin) * res.second);
      } else {
        return std::make_pair(false, point3d_t(0,0,0));
      }
    } else {
      SL_TRACE_OUT(-1) << "unable to unproject " << x << ", " << y << std::endl;
      return std::make_pair(false, point3d_t(0,0,0));
    }
  }

  double qgl_scene_view::north_angle(const rigid_body_map3d_t& V) const {
    matrix4x4d_t VI = V.inverse().as_matrix();

    point3d_t    camera_eye_xyz    = point3d_t(VI(0,3), VI(1,3), VI(2,3));
    vector3d_t   camera_x_axis_xyz = vector3d_t(VI(0,0), VI(1,0), VI(2,0)).ok_normalized();
    vector3d_t   camera_y_axis_xyz = vector3d_t(VI(0,1), VI(1,1), VI(2,1)).ok_normalized();
    vector3d_t   camera_z_axis_xyz = vector3d_t(VI(0,2), VI(1,2), VI(2,2)).ok_normalized();
    vector3d_t   north_axis_xyz = terrain()->xyz_north_from_xyz(camera_eye_xyz);
    vector3d_t   north_axis_xyz_prime = (north_axis_xyz - (north_axis_xyz.dot(camera_z_axis_xyz))*camera_z_axis_xyz).ok_normalized(); 
    
    return -std::atan2(camera_y_axis_xyz.dot(north_axis_xyz_prime),
		       camera_x_axis_xyz.dot(north_axis_xyz_prime));
  }
}




#if 0


  oriented_position qgl_scene_view::constrained_position(const oriented_position& op) {
#if 1
    // FIXME RESTORE CONSTRAINT
    oriented_position result = constrained_ground_position(op);
    return constrained_eye_position(result);
#else
    return constrained_eye_position(op);
#endif
  }

  oriented_position qgl_scene_view::constrained_ground_position(const oriented_position& op) {
    point3d_t p0_ground_xyz = op.ground_target_xyz();
    point3d_t p0_ground_uvh = uvh_xyz_transform()->uvh_from_xyz(p0_ground_xyz);
    
    const double min_u = camera_uvh_bbox_[0][0];
    const double max_u = camera_uvh_bbox_[1][0];
    const double min_v = camera_uvh_bbox_[0][1];
    const double max_v = camera_uvh_bbox_[1][1];

    const point3d_t p0_constrained_uvh = point3d_t(sl::median(p0_ground_uvh[0], min_u, max_u),
						   sl::median(p0_ground_uvh[1], min_v, max_v),
						   0);
    if (!p0_constrained_uvh.is_epsilon_equal(p0_ground_uvh, 0.01)) {
      //      std::cerr << "constrained " << p0_constrained_uvh[0] << " " << p0_constrained_uvh[1] << " " << p0_constrained_uvh[2] << " "
      //		<<  p0_ground_uvh[0] << " " << p0_ground_uvh[1] << " " << p0_ground_uvh[2] << " " << std::endl;
      point3d_t p0_constrained_xyz = uvh_xyz_transform()->xyz_from_uvh(p0_constrained_uvh);
      oriented_position result = op;
      result.move_ground_position_to_xyz(p0_constrained_xyz);
      return result;
    } else {
      return op;
    }    
  }


  oriented_position qgl_scene_view::constrained_eye_position(const oriented_position& op) {
    // 1. Use uvh box 

    // xyz -> uvh
    point3d_t p0_orig_xyz = op.position_xyz();
    point3d_t p0_orig_uvh = terrain_view_->terrain_layer()->model()->uvh_from_xyz(p0_orig_xyz);

    // Find terrain elevation
    std::pair<bool, double> r = terrain_view_->terrain_layer()->model()->current_representation_ground_elevation_from_uv(point2d_t(p0_orig_uvh[0],
																   p0_orig_uvh[1]));
    const double terrain_h = (!r.first) ? 0.0 : r.second;

    //    std::cerr << " H = " << terrain_h << std::endl;

    // Determine coordinate range
    const double delta_h = 200.0; // m FIXME
    const double min_h = std::max(camera_uvh_bbox_[0][2], terrain_h+delta_h);
    const double max_h = std::max(camera_uvh_bbox_[1][2], min_h);

#if 1
    // FIXME RESTORE CONSTRAINT
    // Clamp to box
    const double min_u = camera_uvh_bbox_[0][0];
    const double max_u = camera_uvh_bbox_[1][0];
    const double min_v = camera_uvh_bbox_[0][1];
    const double max_v = camera_uvh_bbox_[1][1];
    const point3d_t p0_constrained_uvh = point3d_t(sl::median(p0_orig_uvh[0], min_u, max_u),
						   sl::median(p0_orig_uvh[1], min_v, max_v),
						   sl::median(p0_orig_uvh[2], min_h, max_h));
#else
    // FIXME
    const point3d_t p0_constrained_uvh = point3d_t(p0_orig_uvh[0], p0_orig_uvh[1],
						   sl::median(p0_orig_uvh[2], min_h, max_h));

#endif
    if (p0_constrained_uvh == p0_orig_uvh) {
      return op;
    } else {
      // Apply constraint

      // Convert back to xyz
      const point3d_t p0_constrained_xyz = terrain_view_->terrain_layer()->model()->xyz_from_uvh(p0_constrained_uvh);
      
      // Convert to "Oriented position"
      // Camera at p0_constrained looking at ground_target
      
      oriented_position result = op;
      result.set_position_xyz(p0_constrained_xyz);

      if (p0_constrained_uvh[2] == p0_orig_uvh[2]) {
	// constraint has been applied to uv, hence stop movement
	camera_controller_.stop_movement();
      }

      return result;
    }
  }


#endif
