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
#ifndef QGL_SCENE_VIEW_HPP
#define QGL_SCENE_VIEW_HPP

#include <GL/glew.h>

#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#endif

#include <QGLWidget>
#include <QMouseEvent>

#include <vic/ratman/camera_controller.hpp>
#include <vic/ratman/camera_animation.hpp>

#include <vector>

namespace cbdam {
  class terrain_model;
}

namespace ratman {
  
  class decorated_terrain_view;

  /**
   * Main opengl window.
   * Draws the terrain and all the layers, and manages 
   * user interaction to move the camera.
   */
  class qgl_scene_view : public QGLWidget {

    Q_OBJECT

  public:
    typedef qgl_scene_view this_t;
    typedef QGLWidget super_t;

  protected:
    decorated_terrain_view*       terrain_view_;
    aabox3d_t                     camera_uvh_bbox_;

    ratman::camera_controller	  camera_controller_;
    ratman::camera_animation	  camera_animation_;
    point3d_t			  previous_eye_position_;
    sl::real_time_clock		  speed_clock_;
    double			  speed_km_h_;
    bool upgraded_tilt_value_;
    std::vector<QCursor *> cursors_;
    std::size_t current_cursor_;
    bool animation_is_active_;

    QMouseEvent last_mouse_move_event_;
  public:
    qgl_scene_view(ratman::decorated_terrain_view* terrain_view,
		   const aabox3d_t& camera_uvh_bbox,
		   const oriented_position& reset_position,
		   QWidget *parent = 0);
    virtual ~qgl_scene_view();

    const cbdam::coordinate_transform* uvh_xyz_transform() const; 

    const ratman::camera_controller &camera_controller() const {
      return camera_controller_;
    }

    ratman::camera_controller &camera_controller() {
      return camera_controller_;
    }
    
    point3d_t current_WGS84_lonlat_position() const;
 
    /// The visible portion of the terrain in degrees (approximation based purely on altitude)
    std::pair<point2d_t,double> current_WGS84_lonlat_center_radius() const;

    std::pair<bool, ratman::point3d_t> terrain_intersection_from_cursor_position(int x, int y) const;

    void update_position_labels();

    bool is_good_cursor(std::size_t i) const {
      return i < cursors_.size();
    }
    
    void set_current_cursor(std::size_t i) {
      assert(is_good_cursor(i));
      current_cursor_=i;
      setCursor(*(cursors_[i]));
    }
    
    std::size_t current_cursor() const {
      return current_cursor_;
    }

    const ratman::decorated_terrain_view* terrain_view() const {
      return terrain_view_;
    }

    const cbdam::terrain_model* terrain() const;

    double north_angle(const rigid_body_map3d_t& V) const;

    oriented_position constrained_position(const oriented_position& op);

protected:
    oriented_position constrained_ground_position(const oriented_position& op);

    oriented_position constrained_eye_position(const oriented_position& op);

public slots:

    virtual void handling_event(void *data);

    void zoom_in_pressed();
    void zoom_in_released();
    void zoom_out_pressed();
    void zoom_out_released();
    void set_zoom_factor(int value);
    void reset_zoom_factor();

    void right_pressed();
    void right_released();
    void left_pressed();
    void left_released();
    void forward_pressed();
    void forward_released();
    void backward_pressed();
    void backward_released();

    void reset_north_pressed();
    void reset_tilt_pressed();

    void yaw_ccw_pressed();
    void yaw_ccw_released();
    void yaw_cw_pressed();
    void yaw_cw_released();

    void tilt_up_pressed();
    void tilt_up_released();
    void tilt_down_pressed();
    void tilt_down_released();
    void set_tilt(int value);
    void reset_tilt();
    void goto_location(const point3d_t& target_pos);
    void goto_oriented_location(const point3d_t& target_pos, const point3d_t& orientation);

    void fly_from_to_oriented_location(const oriented_position& init_pos, const oriented_position& target_pos);
    void go_towards_location(const point3d_t& target_pos);

    void reset();

    signals:
    void signal_animation_begin();
    void signal_animation_end();

    void signal_tilt_reset(int value);
    void signal_zoom_reset(int value);
    void signal_latitute_changed(QString str);
    void signal_longitude_changed(QString str);
    void signal_elevation_changed(QString str);
    void signal_quota_changed(QString str);
    void signal_speed_changed(QString str);
    void signal_yaw_changed(QString str);
    void signal_tilt_changed(QString str);

  protected:
    virtual void timerEvent ( QTimerEvent *e );

    virtual void initializeGL();
    virtual void paintGL();
    virtual void resizeGL(int width, int height);

    virtual bool event(QEvent *e);

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent * e);
    virtual void keyPressEvent(QKeyEvent * e);
    virtual void keyReleaseEvent(QKeyEvent * e);
    virtual void mouseDoubleClickEvent(QMouseEvent* e);
    virtual void wheelEvent(QWheelEvent *e);
  };

}
#endif //QGL_SCENE_VIEW_HPP
