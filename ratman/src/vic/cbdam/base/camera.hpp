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
#ifndef CBDAM_CAMERA_HPP
#define CBDAM_CAMERA_HPP

#include <sl/rigid_body_map.hpp> 
#include <sl/projective_map.hpp>
#include <sl/linear_map_factory.hpp>
#include <sl/fixed_size_vector.hpp>
#include <sl/fixed_size_point.hpp>
#include <sl/quaternion.hpp>
#include <vic/cbdam/base/config.hpp>
#include <cassert>

namespace cbdam {

  /**
   * class camera manage projection matrix and modelview matrix.
   **/
  class camera{
  public:
    typedef double                              value_t;
    typedef sl::projective_map<3, value_t>      projective_map_t;
    typedef sl::rigid_body_map<3, value_t>      rigid_body_map_t;
    typedef sl::linear_map_factory<3, value_t>  linear_map_factory_t;
    typedef sl::fixed_size_vector<sl::column_orientation,3, value_t> vector3_t;
    typedef sl::fixed_size_point<3, value_t>    point3_t;
    
  public:
    /**
     * camera constructor reset variables.
     * set projection matrix perspective and
     * view matrix to identity.
     */
    camera();

    /**
     *  Destructor: does not perform anything, there is nothing to release
     */
    ~camera();

    /////////////////////////////////////////////////////////////////
    // Cameras management

    /**
     *  Set perspective projection, aspect_ratio = w / h
     */
    void set_projection(float fovy, float aspect_ratio, float near, float far);
  
    /**
     *  Set projection map to the one passed as argument
     */
    void set_projection(const projective_map_t& proj);

    /**
     *  Set view map to the one passed as argument
     */
    inline void set_view(const rigid_body_map_t& rbm);
  
    /**
     *  Set view matrix to identity
     */
    void reset_view();
  
    //////////////////////////////////////////////////////////////////
    // Queries

    /**
     * Return projection matrix
     */
    const projective_map_t& projection() const;
  
    /**
     *  Return modelview matrix
     */
    const rigid_body_map_t& view() const;

    /**
     *  Check goodness of projection and view matrices
     */
    bool invariant() const;

    /**
     * return current camera position
     */
    point3_t position() const;

    /**
     * return current camera rotation
     */
    rigid_body_map_t rotation() const;

  protected:
    projective_map_t 	m_projection_map;	  /// Projection matrix
    rigid_body_map_t 	m_view_map; 	  /// view matrix
  };

} // namespace cbdam

#endif // CBDAM_CAMERA_HPP

#ifndef CBDAM_CAMERA_IPP
#define CBDAM_CAMERA_IPP

namespace cbdam {

  inline camera::point3_t camera::position() const {
    // extract point of view from the inverse of the view matrix
    rigid_body_map_t inverse = m_view_map.inverse();
    sl::quaternion<value_t> q;
    camera::vector3_t p;
    inverse.factorize_to( q, p );
    return as_point(p);
  }

  inline camera::rigid_body_map_t camera::rotation() const {
    // extract rotation
    rigid_body_map_t inverse = m_view_map.inverse();
    sl::quaternion<value_t> q;
    camera::vector3_t p;
    inverse.factorize_to( q, p );
    return linear_map_factory_t::rotation( q ).inverse();
  }

  inline void camera::set_projection(const projective_map_t& proj) {
    m_projection_map = proj;
    assert( invariant() );
  }

  inline bool camera::invariant() const {
    return ( m_projection_map.invariant() && m_view_map.invariant() );
  }

  inline void camera::set_view(const rigid_body_map_t& rbm) {
    m_view_map = rbm;
    assert( invariant() );
  }
  
  inline const camera::projective_map_t& camera::projection() const{
    return m_projection_map;
  }
  
  inline const camera::rigid_body_map_t& camera::view() const {
    return m_view_map;
  }

} // namespace cbdam

#endif // CBDAM_CAMERA_IPP
