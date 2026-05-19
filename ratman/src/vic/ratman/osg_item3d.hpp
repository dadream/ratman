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
#ifndef RATMAN_OSG_ITEM3D_HPP
#define RATMAN_OSG_ITEM3D_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/ratman/terrain_item3d.hpp>

#include <GL/glew.h>
#include <osg/ref_ptr>
#include <osg/Matrixd>
#include <osg/MatrixTransform>

#include <sl/fixed_size_square_matrix.hpp>

namespace ratman {

  /**
  *  An implementation of item3d for OpenSceneGraph (OSG)
  */
  class osg_item3d: public terrain_item3d, public osg::MatrixTransform {
  protected:
//    osg::ref_ptr<osg::PositionAttitudeTransform> pat;
    sl::matrix4d modelmatrix_;
    sl::matrix4d modelmatrix_updated_;
//    osg::Matrixd modelmatrix_;

  public:

    osg_item3d(decorated_terrain_view* scene, const std::string& name,
	       sl::point3d location, sl::point3d rotation, sl::point3d scale,
	       const std::string& file);

    ~osg_item3d();

  public:
    /** return the name of the node's class type.*/
    virtual const char* className() const { return "ratman::osg_item3d"; }

    /// Get the most recent updated modelmatrix
    sl::matrix4d get_model_matrix() { return modelmatrix_updated_; }

    /// Activate/deactivate the object
    virtual void set_active(bool x);

    /// Handle picking and return true if handled
    virtual bool on_select_self(const projective_map3d_t& P,
			const rigid_body_map3d_t& V,
			const point2d_t& xy);

    /// Render the hierarchy object rooted at this
    virtual void render_self(qgl_scene_view& qgl,
		     occupancy_map_t& occupancy_map,
		     const projective_map3d_t& P,
		     const rigid_body_map3d_t& V,
		     const point3d_t& C);


    /// Asynchronously update the object. Called in a separate thread.
    virtual void async_update_self(const projective_map3d_t& P,
			   const rigid_body_map3d_t& V,
			   const point3d_t& G);

  };

}

#endif
