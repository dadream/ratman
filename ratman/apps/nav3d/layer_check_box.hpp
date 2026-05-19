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
#ifndef LAYER_CHECK_BOX_HPP
#define LAYER_CHECK_BOX_HPP

#include <QCheckBox>
#include <vic/ratman/active_renderable.hpp>

/**
 * Check box appearing in the layer window.
 * is_active is used to connect state changes to the appwindow. 
 */
class layer_check_box : public QCheckBox {

    Q_OBJECT

protected:
    ratman::active_renderable* layer_;

public:
  layer_check_box(ratman::active_renderable* layer, QWidget *parent = 0);

signals:
  void is_active(ratman::active_renderable* layer, bool b);

protected slots:

void handling_toggled(bool b);

};
#endif // LAYER_CHECK_BOX_HPP
