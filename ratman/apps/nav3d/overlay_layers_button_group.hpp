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
#ifndef OVERLAY_LAYERS_BUTTON_GROUP_HPP
#define OVERLAY_LAYERS_BUTTON_GROUP_HPP

#include <QButtonGroup>

namespace ratman {
  class terrain_renderable;
}

/**
 * Overlay layers group, which is visible in the layers window.
 * Handle multiple texture overlay layers for the terrain model.
 * Many base layers can be active at the same time.
 * When a layer is selected the proper texture layer is activated
 * in the terrain model.
 */
class overlay_layers_button_group : public QButtonGroup {

    Q_OBJECT

protected:
  ratman::terrain_renderable* terrain_;
  
public:
  overlay_layers_button_group(ratman::terrain_renderable* terrain_, QWidget *parent = 0);

signals:
  void is_active(int i);

protected slots:

  void handling_clicked(int i);

};
#endif // OVERLAY_LAYERS_BUTTON_GROUP_HPP
