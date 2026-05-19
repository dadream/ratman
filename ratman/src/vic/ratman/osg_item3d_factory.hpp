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
#ifndef RATMAN_OSG_ITEM3D_FACTORY_HPP
#define RATMAN_OSG_ITEM3D_FACTORY_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/ratman/item3d_factory.hpp>
#include <vic/ratman/osg_item3d.hpp>

#include <osg/Timer>
#include <osg/FrameStamp>
#include <osgUtil/SceneView>
#include <osg/Node>
#include <osg/NodeCallback>

namespace ratman {

  class osg_item3d_callback : public osg::NodeCallback {
    /** Implements the callback. */
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv) {
      if (nv->getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR)
      {
	if (node->className() == "ratman::osg_item3d") {
	    ratman::osg_item3d* osg_item3d = dynamic_cast< ratman::osg_item3d* >(node);
	    update(*osg_item3d);
	}
      }

      traverse(node,nv);
    }

    /** Update the node to set the modelmatrix most recent updated.*/
    void update(osg_item3d& node) {
	if (node.className() == "ratman::osg_item3d") {
	    node.setMatrix( osg::Matrixd(node.get_model_matrix().to_pointer()) );
	}
    }
  };

  /**
  *  The interface for a Factory for 3D terrain items
  */
  class osg_item3d_factory : public item3d_factory {

  protected:
    osg::Timer_t start_tick_;
    unsigned int frame_number_;
    osg::ref_ptr<osg::FrameStamp> frame_stamp_;
    osg::ref_ptr<osgUtil::SceneView> scene_viewer_;
    osg::ref_ptr<osg_item3d_callback> osg_item3d_callback_;

  public:

    osg_item3d_factory();

    virtual ~osg_item3d_factory();

  public:

    /// Creates an OpenSceneGraph 3d terrain item
    virtual osg_item3d* create_item3d(decorated_terrain_view* scene, const std::string& name,
				      sl::point3d location, sl::point3d rotation, sl::point3d scale,
				      const std::string file);

    /// Makes the OpenGL render of all the OSG scenegraph 3d items
    virtual void render_managed();
  };

}

#endif
