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
#include <vic/ratman/item3d_manager.hpp>
#include <vic/ratman/osg_item3d_factory.hpp>
#include <vic/ratman/osg_item3d.hpp>
#include <vic/ratman/qgl_scene_view.hpp>
#include <vic/cbdam/base/terrain_model.hpp>
#include <vic/cbdam/base/terrain_model_renderer.hpp>

#include <terrain_renderable.hpp>

#include <GL/gl.h>
#include <QApplication>
#include <QMessageBox>
#include <cassert>

#include <osg/Group>
#include <osg/CameraNode>
#include <osgDB/ReadFile>

namespace ratman {

    osg_item3d_factory::osg_item3d_factory() :
	item3d_factory() {
	start_tick_ = osg::Timer::instance()->tick();
	frame_stamp_ = new osg::FrameStamp();
	frame_number_ = 0;

	scene_viewer_ = new osgUtil::SceneView();
	scene_viewer_->setDefaults();
	scene_viewer_->setComputeNearFarMode( osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR );
	scene_viewer_->setLightingMode( osgUtil::SceneView::STANDARD_SETTINGS );

	osg::ref_ptr<osg::CameraNode> camera = scene_viewer_->getCamera();
	camera->setClearMask(0);    // We don't like to make a glClear() call on each frame

	osg::Group* root_group = new osg::Group();
	scene_viewer_->setSceneData(root_group);

	osg_item3d_callback_ = new osg_item3d_callback();
    }

    osg_item3d_factory::~osg_item3d_factory() {

    }

    osg_item3d* osg_item3d_factory::create_item3d(decorated_terrain_view* scene, const std::string& name ,
						  sl::point3d location, sl::point3d rotation, sl::point3d scale,
                                                  const std::string file) {

        osg::ref_ptr<osg::Node> data = osgDB::readNodeFile(file);
	if ( !data.valid() ) {
            std::cerr << "ERROR: the " << file << " file doesn't contain a valid OSG model or can not be found!" << std::endl;
            return NULL;
        }
        else {
          osg_item3d* osg_i3d = new osg_item3d(scene, name, location, rotation, scale, file);
          osg_i3d->addChild(data.get());
          osg_i3d->setUpdateCallback(osg_item3d_callback_.get());
          osg::Group* root_group = dynamic_cast<osg::Group*>(scene_viewer_->getSceneData());
          if (root_group != NULL) {
            root_group->addChild(osg_i3d);
          }
          else {
            std::cerr << "ERROR > osg_item3d_factory::create_item3d : The scene data root is not a osg::Group node!" << std::endl;
          }
          return osg_i3d;
        }
    }

    void osg_item3d_factory::render_managed() {
      glPushAttrib( GL_ALL_ATTRIB_BITS );
      glMatrixMode(GL_PROJECTION);
      glPushMatrix();
      glMatrixMode(GL_TEXTURE);
      glPushMatrix();
      glLoadIdentity();
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();

      if ( item3d_manager::instance()->is_atmosphere() ) {
        glEnable(GL_FOG);
	scene_viewer_->setLightingMode( osgUtil::SceneView::STANDARD_SETTINGS );
      } else {
	glDisable(GL_FOG);
	scene_viewer_->setLightingMode( osgUtil::SceneView::SKY_LIGHT );
      }

      GLint viewParams[4];
      glGetIntegerv( GL_VIEWPORT, viewParams );
      scene_viewer_->setViewport(viewParams[0], viewParams[1], viewParams[2], viewParams[3]);

      GLfloat glMat[16];
      osg::Matrix osgMat;

      glGetFloatv( GL_PROJECTION_MATRIX, glMat );
      osgMat.set( glMat );
      scene_viewer_->getProjectionMatrix().set( osgMat );

      glGetFloatv( GL_MODELVIEW_MATRIX, glMat );
      osgMat.set( glMat );
      scene_viewer_->getViewMatrix().set( osgMat );

      osg::Timer* timer = osg::Timer::instance();
      double time_since_start = timer->delta_s( start_tick_, timer->tick() );
      frame_stamp_->setReferenceTime( time_since_start );
      frame_stamp_->setFrameNumber( frame_number_++ );
      scene_viewer_->setFrameStamp( frame_stamp_.get() );

      scene_viewer_->update ();
      scene_viewer_->cull ();
      scene_viewer_->draw ();

      glMatrixMode(GL_MODELVIEW);
      glPopMatrix();
      glMatrixMode(GL_TEXTURE);
      glPopMatrix();
      glMatrixMode(GL_PROJECTION);
      glPopMatrix();
      glPopAttrib();

    }
}
