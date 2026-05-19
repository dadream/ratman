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
#ifndef NAV3D_METEO_DIALOG_HPP
#define NAV3D_METEO_DIALOG_HPP

#include <QTextEdit>
#include <QPixmap>
#include "ui_meteo_dialog.hpp"

/**
 * Dialog which presents detailed meteo informations.
 * Uses mete_dialog.ui interface.
 * Meteo information is provided by ratman::meteo_data
 */
class meteo_dialog : public QDialog {

  Q_OBJECT

public:
  meteo_dialog(QWidget *parent = 0, Qt::WindowFlags f = 0);

  QTextEdit* meteo_info() const;

  void set_meteo_icon(const QPixmap &icon) {
    ui_.meteo_icon_->setPixmap(icon);
  }

private:
  Ui::meteo_dialog ui_;
};

#endif // NAV3D_METEO_DIALOG_HPP
