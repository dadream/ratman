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
#include <vic/ratman/terrain_renderable.hpp>
#include <vic/ratman/qgl_scene_view.hpp>
#include <vic/cbdam/base/terrain_model.hpp>
#include <vic/cbdam/base/terrain_model_renderer.hpp>

#include <GL/glew.h>
#include <QApplication>
#include <QMessageBox>
#include <cassert>

namespace ratman {
  
  terrain_renderable::terrain_renderable(decorated_terrain_view* scene,
					 cbdam::terrain_model* model) :
    active_renderable(scene)
  {
    assert(model);
    model_ = model;
    renderer_ = new cbdam::terrain_model_renderer(model_);

    set_shading_enabled(false); // default value

    first_frame_ = true;
  }
      
  terrain_renderable::~terrain_renderable() {
    if (renderer_) delete renderer_;
  }
            
  const cbdam::terrain_model* terrain_renderable::model() const {
    return model_;
  }

  cbdam::terrain_model* terrain_renderable::model() {
    return model_;
  }

  const cbdam::terrain_model_renderer* terrain_renderable::renderer() const {
    return renderer_;
  }

  cbdam::terrain_model_renderer* terrain_renderable::renderer() {
    return renderer_;
  }

  bool terrain_renderable::on_select_self(const projective_map3d_t& /*P*/,
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
    
  void terrain_renderable::render_self(qgl_scene_view& /*qgl*/,
				       occupancy_map_t& /*occupancy_map*/,
				       const projective_map3d_t& P,
				       const rigid_body_map3d_t& V,
				       const point3d_t& C) {
    if (first_frame_ && !model_->is_connected()) {
      // -- No data
      QString msg = QApplication::tr("Unable to connect to terrain model server");
      QMessageBox::critical( 0, QApplication::tr("Graphics error" ), msg, QMessageBox::Abort,0 );
      qFatal(qPrintable(msg));	      
    } else {
      // Init OpenGL
      if (first_frame_) {
	SL_TRACE_OUT(1) << "INIT OPENGL!" << std::endl;
	renderer_->init_opengl();

	if (!renderer_->is_opengl_supported()) {
	  QString msg = QApplication::tr("OpenGL failed to initialized. Your graphics board or driver is not capable to run this software.");
	  QMessageBox::critical( 0, QApplication::tr("Graphics error" ), msg, QMessageBox::Abort,0 );
	  qFatal(qPrintable(msg));	      
	}
      }

      // Render
      const double tol_pixels = 1.25f;
      GLint xywh[4];
      glGetIntegerv(GL_VIEWPORT,xywh);
      double tol_screen = tol_pixels / double(xywh[3]);

      renderer_->set_screen_tolerance(tol_screen);
      renderer_->draw(P, V, C);

      // FIXME std::cerr << "TRI/F: " << renderer_->stat_rendered_triangles() << std::endl;
    }

    // Mark not first frame
    first_frame_ = false;
  }
            
  void terrain_renderable::async_update_self(const projective_map3d_t& /*P*/,
					     const rigid_body_map3d_t& /*V*/,
					     const point3d_t& /*G*/) {
    // Nothing to do
  }
 
  void terrain_renderable::set_shading_enabled(bool x) {
    renderer_->set_shading_enabled(x);
  }

  bool terrain_renderable::is_shading_enabled() const {
    return renderer_->is_shading_enabled();
  }

}
