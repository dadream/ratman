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
#include "ui_opossum_window_gui.hpp"
#include "opossum_window.hpp"
#include <QVBoxLayout>
#include "qgl_window_opossum.hpp"

opossum_window::opossum_window( QWidget* parent,  const char* name, Qt::WindowFlags fl )
    : QWidget( parent, name, fl ) {
    ui.setupUi(this);
    qgl_window_ptr = new qgl_window_opossum(this,"qgl_window_opossum");
    ui.vboxLayout->addWidget(qgl_window_ptr);
   connect( qgl_window_ptr, SIGNAL( stop_rendering() ), this, SLOT( slot_stop_rendering() ) );
}


opossum_window::~opossum_window()
{
    // no need to delete child widgets, Qt does it all for us
}

bool opossum_window::init(const char* input_name_height, 
			      const char* input_name_color, 
			      const char* input_name_buildings) {
  return qgl_window_ptr->init(input_name_height, input_name_color, input_name_buildings);
}

void opossum_window::slot_stop_rendering() {
  emit stop_rendering(); 
}
