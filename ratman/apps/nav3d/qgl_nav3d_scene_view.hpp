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
#ifndef QGL_NAV3D_SCENE_VIEW_HPP
#define QGL_NAV3D_SCENE_VIEW_HPP

#include <vic/ratman/qgl_scene_view.hpp>
#include <vic/ratman/decorated_terrain_view.hpp>
#include <vic/ratman/ratman.hpp>

class meteo_dialog;

/**
 * Main opengl window. Derived from ratman::qgl_scene_view: 
 * adds to qgl_scene_view only meteo stuff.
 * The behaviour of the opengl window is mostly handled by
 * the base class.
 */
class qgl_nav3d_scene_view : public ratman::qgl_scene_view {

  Q_OBJECT

protected:
  meteo_dialog* meteo_info_;

  public:
  qgl_nav3d_scene_view(ratman::decorated_terrain_view* terrain_view,
		       const ratman::aabox3d_t& camera_uvh_bbox,
		       const ratman::oriented_position& reset_position,
		       QWidget *parent = 0);
  virtual ~qgl_nav3d_scene_view();

public slots:
  virtual void handling_event(void *data);

};



#endif // QGL_NAV3D_SCENE_VIEW_HPP
