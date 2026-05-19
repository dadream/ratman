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
#include <vic/ratman/osg_item3d.hpp>
#include <vic/ratman/qgl_scene_view.hpp>
#include <vic/cbdam/base/terrain_model.hpp>
#include <vic/cbdam/base/terrain_model_renderer.hpp>

#include <terrain_renderable.hpp>

#include <QApplication>
#include <QMessageBox>
#include <cassert>

#include <osg/CameraNode>
#include <osg/AutoTransform>
#include <osg/PositionAttitudeTransform>

namespace ratman {

    osg_item3d::osg_item3d(decorated_terrain_view* scene, const std::string& name,
                           sl::point3d location, sl::point3d rotation, sl::point3d scale,
                           const std::string& file) :
    terrain_item3d(scene, name, file) {

      sl:: point3d uvh = sl::point3d(location); // replace with location_ memeber variable

      modelmatrix_ = terrain_layer()->model()->local_to_global_from_WGS84_lonlat(uvh).as_matrix();

      //modelmatrix_ = osg::Matrixd(terrain_layer()->model()->local_to_global_from_WGS84_lonlat(uvh).as_matrix().to_pointer());
      //osg::Matrixd scalematrix;
      //scalematrix.makeScale(osg::Vec3d(10000, 10000, 10000)); // scale_
      //modelmatrix_ *= scalematrix;

      // MAKE SCALE
      modelmatrix_ *= sl::matrix4d(scale[0], 0, 0, 0,
                                   0, scale[1], 0, 0,
                                   0, 0, scale[2], 0,
                                   0, 0, 0, 1
                                  );

      // MAKE ROTATION
      const double PI = 3.14159265358979323846264338327;

      double h = rotation[0]*PI/180;
      double p = rotation[1]*PI/180;
      double r = rotation[2]*PI/180;
      // rotation on the X axis
      modelmatrix_ *= sl::matrix4d(1, 0, 0, 0,
                                   0, cos(h), sin(h), 0,
                                   0, -sin(h), cos(h), 0,
                                   0, 0, 0, 1
                                  );
      // rotation on the Y axis
      modelmatrix_ *= sl::matrix4d(cos(p), 0, -sin(p), 0,
                                   0, 1, 0, 0,
                                   sin(p), 0, cos(p), 0,
                                   0, 0, 0, 1
                                  );

      // rotation on the Z axis
      modelmatrix_ *= sl::matrix4d(cos(r), sin(r), 0, 0,
                                   -sin(r), cos(r), 0, 0,
                                   0, 0, 1, 0,
                                   0, 0, 0, 1
                                  );

      // Rescale normals
      osg::ref_ptr<osg::StateSet> stateset = new osg::StateSet();
      stateset->setMode(GL_NORMALIZE, osg::StateAttribute::ON);

      //mt_ = new osg::MatrixTransform();

      modelmatrix_updated_ = modelmatrix_;
      setMatrix( osg::Matrixd(modelmatrix_.to_pointer()) );
      //setMatrix(modelmatrix_);
      setStateSet(stateset.get());
    }


    osg_item3d::~osg_item3d() {
    }

    void osg_item3d::set_active(bool x) {
    mutex_.lock();
    {
      is_active_ = x;
      if (x)
        setNodeMask(0xffffffff); // Disable the node for all NodeVisitors of OSG
      else
        setNodeMask(0x0);
    }
    mutex_.unlock();
  }

  bool osg_item3d::on_select_self(const projective_map3d_t& /*P*/,
				  const rigid_body_map3d_t& /*V*/,
				  const point2d_t& /*xy*/) {
    bool result;
    //   mutex_.lock();
    {
      result = false;
    }
    // mutex_.unlock();
    return result;
  }

  void osg_item3d::render_self(qgl_scene_view& /*qgl*/,
			       occupancy_map_t& /*occupancy_map*/,
			       const projective_map3d_t& /*P*/,
			       const rigid_body_map3d_t& /*V*/,
			       const point3d_t& C) {
      sl::matrix4d m = modelmatrix_;
      m(0, 3) -= C[0];
      m(1, 3) -= C[1];
      m(2, 3) -= C[2];
      modelmatrix_updated_ = m;
  }

  void osg_item3d::async_update_self(const projective_map3d_t& /*P*/,
				     const rigid_body_map3d_t& /*V*/,
				     const point3d_t& /*G*/) {
      // Nothing to do
  }

}
