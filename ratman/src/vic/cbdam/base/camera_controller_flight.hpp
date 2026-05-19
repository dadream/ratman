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
#ifndef CBDAM_CAMERA_CONTROLLER_FLIGHT_HPP
#define CBDAM_CAMERA_CONTROLLER_FLIGHT_HPP

#include <vic/cbdam/base/camera_controller_base.hpp>


namespace cbdam {

  /**
   * class camera_controller_flight  manage The user input
   * through mouse and keyboard to modify the state
   * of the modelview matrix of the object camera.
   * A ptr to the camera object is passed in the constructor
   * this controller rotate the object on its center, and allow 
   * zooming from it.
   */
  class camera_controller_flight : public camera_controller_base {
  public:
    /**
     * camera_controller_flight constructor reset variables.
     * Set yaw, pitch  to 0,
     */
    camera_controller_flight(camera* camera_ptr);

    /**
     *  Destructor: does not perform anything, there is nothing to release
     */
    virtual ~camera_controller_flight();

    /**
     *  Reset to intial pos/rot
     */
    virtual void reset();

    /**
     * reset rotation to identity
     */
    virtual void reset_rotation();

    /**
     *  Set initial position
     */
    virtual void set_distance(camera::value_t distance);
    
    /**
     * called to update camera view
     */
    virtual void idle_update();

    /**
     * the position of the camera if the object would be centered
     */
    virtual camera::point3_t camera_position() const;

    void set_position(camera::vector3_t& x);

  protected:
    camera::vector3_t delta_position(const camera::value_t& dt, const camera::vector3_t& direction) const;
    
    camera::value_t delta_yaw(const camera::value_t& dt, const camera::vector3_t& direction) const;

    camera::value_t delta_pitch(const camera::value_t& dt, const camera::vector3_t& direction) const;

    camera::value_t weight_from_height() const;
    
    /**
     *  Compute the camera view from passed parameters
     */
    camera::rigid_body_map_t compute_camera_view() const;
    
     /**
     *  Update the camera view in the camera_ptr
     */
     virtual void update_camera_view() const;

  protected:
    camera::value_t     m_pitch;
    camera::value_t     m_pitch_max;
    camera::value_t     m_pitch_min;
    camera::value_t     m_yaw;
    camera::vector3_t   m_position;
  };

} // namespace cbdam

#endif // CBDAM_CAMERA_CONTROLLER_FLIGHT_IPP

#ifndef CBDAM_CAMERA_CONTROLLER_FLIGHT_IPP
#define CBDAM_CAMERA_CONTROLLER_FLIGHT_IPP

namespace cbdam {

  inline void camera_controller_flight::update_camera_view() const {
    m_camera_ptr->set_view(compute_camera_view());
  }

  inline camera::point3_t camera_controller_flight::camera_position() const {
    return as_point(m_position);
  }

} // namespace cbdam 

#endif // CBDAM_CAMERA_CONTROLLER_FLIGHT_IPP
