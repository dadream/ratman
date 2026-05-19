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
#ifndef PARAMETERS_DIALOG_HPP
#define PARAMETERS_DIALOG_HPP

#include "ui_parametersdialog.hpp"
#include <vic/ratman/atmosphere.hpp>

#include <QCloseEvent>
#include <iostream>


/**
 * Dialog which handles sky parameters.
 * Uses parametersdialog.ui interface.
 * Sky parameters are applied to ratman::atmosphere.
 */ 
class parameters_dialog : public QDialog
{
  Q_OBJECT

public:

protected:

public:
  parameters_dialog(QWidget *parent = 0);

  void set_tool(ratman::atmosphere* a);


  void  closeEvent(QCloseEvent *event);

protected slots:
  void update_sky();
  void update_azimuth(int i);
  void update_altitude(int i);
  void update_turbidity(int i);
  void update_daytime(int i);
  void update_minutes(int i);
  void update_date(QDate date);
  void switch_sunmode(bool b);
  void update_fov_y(int i);

  signals:
void sky_dialog_close();

private:
  Ui_Dialog ui;
  ratman::atmosphere* atm_;
};

#endif

