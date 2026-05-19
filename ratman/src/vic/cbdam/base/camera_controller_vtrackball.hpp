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
#ifndef CBDAM_CAMERA_CONTROLLER_VTRACKBALL_HPP
#define CBDAM_CAMERA_CONTROLLER_VTRACKBALL_HPP

#include <vic/cbdam/base/camera_controller_base.hpp>

#ifdef _WIN32
#undef min
#undef max
#endif

namespace cbdam {

  /**
   * class camera_controller_vtrackball  manage The user input
   * through mouse and keyboard to modify the state
   * of the modelview matrix of the object camera.
   * A ptr to the camera object is passed in the constructor
   * this controller rotate the object on its center, and allow 
   * zooming from it.
   */
  class camera_controller_vtrackball : public camera_controller_base {
  public:
    /**
     * camera_controller_vtrackball constructor reset variables.
     * Set yaw, pitch  to 0,
     */
    camera_controller_vtrackball(camera* camera_ptr);

    /**
     *  Destructor: does not perform anything, there is nothing to release
     */
    virtual ~camera_controller_vtrackball();


    /////////////////////////////////////////////////////////////////
    // Camera_Controller_Vtrackballs management
    
    /**
     *  Set initial angles to 0,0 and position to 0,0, min_dist
     *  and viewspeed to 0.3
     */
    virtual void reset();

    /**
     *  Set tilt angle.
     */
    void set_tilt_angle(camera::value_t tilt_angle);

    /**
     * reset rotation to identity
     */
    virtual void reset_rotation();

    /**
     * called to update camera view
     */
    virtual void idle_update();

    /**
     * Predict and set a new camera position in the camera
     * passed as parameter, depending on current status
     */
    void predict_position(camera* cam, const camera::value_t& dt_microseconds) const;

    /**
     * return current camera tilt angle
     */
    camera::value_t tilt_angle() const;

    /**
     * the position of the camera if the object would be centered
     */
    virtual camera::point3_t camera_position() const;

  protected:
    /**
     * Compute rotation matrix after dt
     */
    camera::rigid_body_map_t next_rotation_matrix(const camera::value_t& dt) const;
  
    /**
     * Compute m_distance after dt
     */
    camera::value_t next_distance(const camera::value_t& dt) const;

    /**
     * Compute next tilt_angle value after dt
     */
    camera::value_t next_tilt_angle(const camera::value_t& dt) const;

    /**
     *  Compute the camera view from passed parameters
     */
    virtual camera::rigid_body_map_t compute_camera_view(const camera::value_t& distance, const camera::value_t& tilt_angle, 
                                                         const camera::rigid_body_map_t& rotation_matrix) const;
     /**
     *  Update the camera view in the camera_ptr
     */
    virtual void update_camera_view() const;

  protected:
    camera::rigid_body_map_t m_rotation_matrix;		//< the sphere rotation about its center
    camera::value_t		m_tilt_angle;			//< tilt of the camera: rotation about the x axis, on the point of intersection between sphere and POV-spherecenter ray.
    camera::value_t		m_acceleration;			//< 1+m_acceleration multiply speed
    camera::value_t		m_tilt_factor;			//< convert mouse movement into radians for tilt
    camera::value_t 		m_minimum_rotation_angle;	//< angle under which no rotation is computed
    camera::value_t		m_tilt_angle_bound;		//< minimum bound for tilt angle
  };

} // namespace cbdam

#endif // CBDAM_CAMERA_CONTROLLER_VTRACKBALL_IPP

#ifndef CBDAM_CAMERA_CONTROLLER_VTRACKBALL_IPP
#define CBDAM_CAMERA_CONTROLLER_VTRACKBALL_IPP

namespace cbdam {

  inline void camera_controller_vtrackball::set_tilt_angle(camera::value_t tilt_angle) {
    m_tilt_angle = tilt_angle;
    update_camera_view();
  }

  inline camera::rigid_body_map_t camera_controller_vtrackball::compute_camera_view(const camera::value_t& distance, const camera::value_t& tilt_angle, 
                                                                                    const camera::rigid_body_map_t& rotation_matrix) const {
    // Matrix : 1)rotate around center 2) translate -radius 3)tilt [0-90] 4)translate -( distance-radius)

    return camera::rigid_body_map_t(camera::linear_map_factory_t::translation( camera::vector3_t( 0, 0, -(distance - m_radius ) ) ) * 
                                    camera::linear_map_factory_t::rotation( 0, tilt_angle ) * 
                                    camera::linear_map_factory_t::translation( camera::vector3_t( 0, 0, -m_radius ) ) * 
                                    rotation_matrix);
  }

  inline void camera_controller_vtrackball::update_camera_view() const {
    m_camera_ptr->set_view( compute_camera_view( m_distance, m_tilt_angle, m_rotation_matrix ) );
  }

  inline camera::value_t camera_controller_vtrackball::tilt_angle() const {
    return m_tilt_angle;
  }

  inline camera::point3_t camera_controller_vtrackball::camera_position() const {
    // inverse transformation
    // 1. Translate D - R
    camera::point3_t v(  0, 0, m_distance - m_radius );

    // 2. Rotate - tilt
    v = camera::linear_map_factory_t::rotation( 0, -m_tilt_angle ) * v;

    // 3. Translate R along Z
    v[ 2 ] += m_radius;

    // 4. Rotate -rotation matrix
    return inverse_transformation( m_rotation_matrix, v );
  }

} // namespace cbdam 

#endif // CBDAM_CAMERA_CONTROLLER_VTRACKBALL_IPP
