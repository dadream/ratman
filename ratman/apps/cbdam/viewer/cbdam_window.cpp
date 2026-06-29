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
#include "cbdam_window.hpp"
#include <QWidget>

/* 
 *  Constructs a cbdam_window which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f' 
 */
cbdam_window::cbdam_window(QWidget* parent, Qt::WindowFlags fl )
    : QWidget(parent, fl )
{
  // connect stop rendering to propagate from qgl_window_ptr to application
  ui.setupUi(this);

  // connect stop rendering to propagate from qgl_window_ptr to application
  connect(ui.qgl_window_ptr, SIGNAL( stop_rendering() ), this, SLOT( slot_stop_rendering() ) );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
cbdam_window::~cbdam_window() {
    // no need to delete child widgets, Qt does it all for us
}

bool cbdam_window::open(const std::string& height_url) {
  return ui.qgl_window_ptr->open(height_url);
}

void cbdam_window::insert_color_layer(const std::string& id, cbdam::geoimage_quad_fetcher* fetcher, 
				      std::size_t first_level, std::size_t last_level, double min_altitude, double max_altitude, 
				      bool is_base_layer, bool is_active) {
  ui.qgl_window_ptr->insert_color_layer(id, fetcher, first_level, last_level, min_altitude, max_altitude, is_base_layer, is_active);
}

bool cbdam_window::init_buildings(const char* fname) {
  return  ui.qgl_window_ptr->init_buildings(fname);
}

void cbdam_window::slot_stop_rendering() {
  // propagate stop signal to qt app
  emit stop_rendering();
}
 
const cbdam::cbdam_diamond_fetcher* cbdam_window::elevation_fetcher() const {
  return ui.qgl_window_ptr->elevation_fetcher();
}

bool cbdam_window::configure_verification(const std::string& script_file,
					  const std::string& output_dir,
					  bool exit_when_done,
					  bool log_state) {
  return ui.qgl_window_ptr->configure_verification(script_file, output_dir, exit_when_done, log_state);
}

void cbdam_window::set_verification_window_size(int width, int height) {
  if (layout()) {
    layout()->setSizeConstraint(QLayout::SetFixedSize);
  }
  setFixedSize(width, height);
  resize(width, height);
  ui.qgl_window_ptr->set_verification_window_size(width, height);
  ui.qgl_window_ptr->setFixedSize(width, height);
  ui.qgl_window_ptr->resize(width, height);
  ui.qgl_window_ptr->setGeometry(0, 0, width, height);
  ui.qgl_window_ptr->updateGeometry();
  updateGeometry();
  adjustSize();
}

bool cbdam_window::verification_failed() const {
  return ui.qgl_window_ptr->verification_failed();
}
