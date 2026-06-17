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
#ifndef CBDAM_WINDOW_H
#define CBDAM_WINDOW_H

#include <GL/glew.h>
#include "ui_cbdam_window.hpp"
#include <vic/cbdam/base/grid_diamond_graph_incore.hpp>
#include <vic/cbdam/base/cbdam_diamond_fetcher.hpp>


class cbdam_window : public QWidget
{ 
    Q_OBJECT

public:
    cbdam_window( QWidget* parent = 0, Qt::WindowFlags fl = 0 );
    ~cbdam_window();

  bool open(const std::string& height_url);

  void insert_color_layer(const std::string& id, cbdam::geoimage_quad_fetcher* fetcher, 
			  std::size_t first_level, std::size_t last_level, double min_altitude, double max_altitude,
			  bool is_base_layer, bool is_active);

  bool init_buildings(const char* fname);

  const cbdam::cbdam_diamond_fetcher* elevation_fetcher() const;

  bool configure_verification(const std::string& script_file,
			      const std::string& output_dir,
			      bool exit_when_done,
			      bool log_state);

  bool verification_failed() const;

 signals:
    void stop_rendering();

public slots:
  void slot_stop_rendering();

private:
    Ui::cbdam_window ui;

};

#endif // CBDAM_WINDOW_H
