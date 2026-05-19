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
#ifndef CBDAM_CAMERA_CONTROLLER_BASE_HPP
#define CBDAM_CAMERA_CONTROLLER_BASE_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/camera.hpp>
#include <sl/rigid_body_map.hpp>
#include <sl/projective_map.hpp>
#include <sl/linear_map_factory.hpp>
#include <sl/fixed_size_vector.hpp>
#include <sl/fixed_size_point.hpp>
#include <sl/clock.hpp>


namespace cbdam {

  /**
   * class camera_controller_base  manage The user input
   * through mouse and keyboard to modify the state
   * of the modelview matrix of the object camera.
   * A ptr to the camera object is passed in the constructor
   * this controller rotate the object on its center, and allow 
   * zooming from it.
   */
  class camera_controller_base {
  public:
    enum MOUSE_STATUS { CC_MB_RELEASED = 0, CC_MB_LEFT = 1, CC_MB_MIDDLE = 2, CC_MB_RIGHT = 4 };
    enum KEYBOARD_STATUS { CC_K_RELEASED = 0, CC_K_FORWARD = 1, CC_K_BACKWARD = 2, CC_K_LEFT = 4, CC_K_RIGHT = 8 };
    typedef sl::fixed_size_point<2, int> point2i;
    
  public:

    /**
     * camera_controller_base constructor reset variables.
     */
    camera_controller_base(camera* camera_ptr);

    /**
     *  Destructor: does not perform anything, there is nothing to release
     */
    virtual ~camera_controller_base();


    /////////////////////////////////////////////////////////////////
    // Camera_Controller_Bases management

    /**
     * increase rotation factor, which is used to map
     * mouse movement to planet rotation
     */
    void increase_rotation_factor();

    /**
     * decrease rotation factor, which is used to map
     * mouse movement to planet rotation
     */
    void decrease_rotation_factor();

    /**
     * set dimensions of the rendering window
     */
    void set_window_size(int w, int h);

    /**
     *  Set initial angles to 0,0 and position to 0,0, min_dist
     *  and viewspeed to 0.3
     */
    virtual void reset();

    /**
     * set the minimum distance from center of the object
     * that can be reached
     */
    void set_radius(camera::value_t radius);

    /**
     *  Set initial distance from the center of the sphere
     */
    virtual void set_distance(camera::value_t distance);

    /**
     * reset rotation to identity
     */
    virtual void reset_rotation() = 0;

    /**
     * called to update camera view
     */
    virtual void idle_update() = 0;

    //////////////////////////////////////////////////////////////////
    // User Input

    /**
     *  update stored cursor position
     */
    void mouse_move(const point2i& pos );

    /**
     *  Update stored cursor position and and button_state
     */
    void mouse_pressed(const point2i& pos, MOUSE_STATUS button);
  
    /**
     * update stored cursor position and and button_state
     **/
    void mouse_released(const point2i& pos, MOUSE_STATUS button);

    /**
     *  Manage key event 
     */
    void key_pressed(KEYBOARD_STATUS key_status);

    /**
     * update keyboard state
     */
    void key_released(KEYBOARD_STATUS key_status);

    /**
     * the position of the camera if the object would be centered
     */
    virtual camera::point3_t camera_position() const = 0;

    camera::value_t distance() const;

    camera::value_t rotation_factor() const;

  protected:
    /**
     * find a unit vector on the sphere corresponding to the position
     * of the mouse on the window
     */
    camera::vector3_t project_mouse(const point2i& pos) const;

    /**
     *  Update the camera view in the camera_ptr
     */
    virtual void update_camera_view() const = 0;

  protected:
    camera*		m_camera_ptr;			//< camera contains current rotation to be updated
    sl::real_time_clock m_clock;			//< used to compute increments independent from cpu speed
    camera::vector3_t	m_projected_center;		//< center of the trackball sphere
    camera::value_t	m_distance;			//< object placet at -distance along z
    camera::value_t	m_radius;			//< minimum distance from the center of the object
    point2i            	m_cursor_position;		//< old cursor position
    point2i		m_window_size;			//< used to map cursor position into a trackball sphere
    int		 	m_mouse_status;			//< mouse status used by idle function 
    int			m_keyboard_status;		//< keyboard status used by idle function
    camera::value_t     m_rotation_factor;
  };

} // namespace cbdam

#endif // CBDAM_CAMERA_CONTROLLER_BASE_IPP

#ifndef CBDAM_CAMERA_CONTROLLER_BASE_IPP
#define CBDAM_CAMERA_CONTROLLER_BASE_IPP

namespace cbdam {

  inline void camera_controller_base::set_distance(camera::value_t distance) {
    m_distance = distance;
    update_camera_view();
  }

  inline void camera_controller_base::set_radius(camera::value_t radius) {
    m_radius = radius;
  }

  inline void camera_controller_base::increase_rotation_factor() {
    m_rotation_factor *=2.0;
  }

  inline void camera_controller_base::decrease_rotation_factor() {
    m_rotation_factor *= 0.5;
 }
  
  inline void camera_controller_base::mouse_pressed(const point2i& pos, MOUSE_STATUS button ) {
    if ( m_mouse_status == CC_MB_RELEASED ) {
      m_cursor_position = pos;
      m_projected_center = project_mouse( pos );
    }
    
    // multiple key could be pressed togheter
    m_mouse_status |= button;
  }
  
  inline void camera_controller_base::mouse_released(const point2i& pos, MOUSE_STATUS button ) {
    m_mouse_status &= !button;
    if ( m_mouse_status == CC_MB_RELEASED ) {
      m_cursor_position = pos;
    }
  }

  inline void camera_controller_base::key_pressed(KEYBOARD_STATUS key_status){
    m_keyboard_status |= key_status;
  }

  inline void camera_controller_base::key_released(KEYBOARD_STATUS key_status){
    m_keyboard_status &= !key_status;
  }

  inline void camera_controller_base::mouse_move(const point2i& pos ) {
    m_cursor_position = pos;
  }

  inline void camera_controller_base::set_window_size(int w, int h) {
    m_window_size = point2i( w, h);
    m_projected_center = project_mouse( point2i( 0, 0 ) );
  }

  inline camera::vector3_t camera_controller_base::project_mouse(const point2i& pos) const {
    camera::vector3_t v( (2.0*pos[0]- m_window_size[0]) / m_window_size[0],
                         (m_window_size[1] - 2.0*pos[1]) / m_window_size[1],
                         0.0f );
    camera::value_t d = v.two_norm();
    d = (d<1.0) ? d : 1.0;
    v[ 2 ] = sqrt(1.001 - d*d);
    v.ok_normalized(); // Still need to normalize, since we only capped d, not v.
    return v;
  }

  inline camera::value_t camera_controller_base::distance() const {
    return m_distance;
  }

  inline camera::value_t camera_controller_base::rotation_factor() const {
    return m_rotation_factor;
  }

} // namespace cbdam 

#endif // CBDAM_CAMERA_CONTROLLER_BASE_IPP
