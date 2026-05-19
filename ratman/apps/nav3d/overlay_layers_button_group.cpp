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
#include "overlay_layers_button_group.hpp"

#include <vic/ratman/terrain_renderable.hpp>
#include <vic/cbdam/base/terrain_model.hpp>
#include <vic/cbdam/base/terrain_model_renderer.hpp>

#include <QAbstractButton>
#include <cassert>

overlay_layers_button_group::overlay_layers_button_group(ratman::terrain_renderable* terrain, QWidget *parent) :
    QButtonGroup(parent),
    terrain_(terrain){
  assert(terrain_);
  setExclusive(false);
  connect(this, SIGNAL(buttonClicked(int)), this, SLOT(handling_clicked(int)));
}

void overlay_layers_button_group::handling_clicked(int i) {
  QAbstractButton* b=button(i);
  if(b && b->isCheckable()) {
    //    std::cerr << "set overlay active " << b->isChecked() << std::endl;
    terrain_->terrain_layer()->model()->set_overlay_color_layer_active(i,b->isChecked());
    //    emit is_active(i);
  }
}
