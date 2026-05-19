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
#ifndef OPOSSUM_WINDOW_H
#define OPOSSUM_WINDOW_H

#include <QtGui>
#include "ui_opossum_window_gui.hpp"

class qgl_window_opossum;

class opossum_window : public QWidget { 

    Q_OBJECT

protected: 
    qgl_window_opossum *qgl_window_ptr;

public:
    opossum_window( QWidget* parent = 0, const char* name = 0, Qt::WindowFlags fl = 0 );
    ~opossum_window();

    bool init(const char* input_name_height, 
	      const char* input_name_color, 
	      const char* input_name_buildings);

signals:
  void stop_rendering();

public slots:
  void slot_stop_rendering();
 
private:
  Ui::opossum_window_gui ui;

};

#endif // OPOSSUM_WINDOW_H
