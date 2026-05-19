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
#ifndef SEARCH_RESULT_DIALOG_HPP
#define SEARCH_RESULT_DIALOG_HPP

#include "ui_search_result.hpp"

#include <vic/ratman/ratman.hpp>
#include <iostream>
#include <vector>

namespace ratman {
  class geonames_search;
}

/**
 * Dialog containing the results of a search operation.
 * It is based on the search_result.ui which contains
 * a tree widget, where all the results are inserted.
 */
class search_result_dialog : public QDialog {

  Q_OBJECT

public:
  typedef ratman::point3d_t point3d_t;
  typedef ratman::geonames_search geonames_search_t;
  typedef std::vector<geonames_search_t*> geonames_engines_t;

protected:
  
  geonames_engines_t search_engines_;

public:

  search_result_dialog(const geonames_engines_t& search_engines, QWidget *parent = 0);

  QTreeWidget* treeWidget(){
    return ui.treeWidget;
  }

  QGridLayout *gridLayout(){
    return ui.gridLayout;
  }

public slots:
 
  void search(const QString& search_text,
	      const ratman::aabox2d_t& query_box = ratman::aabox2d_t(ratman::point2d_t(-90.0,-180.0),
								     ratman::point2d_t( 90.0, 180.0)));

  void handling_doubleclicking(QTreeWidgetItem *item, int col);

signals:
  /**
   * Signal sent to the appwindow to let the program perform a goto operation
   */
  void item_doubleclicked(std::size_t engine, std::size_t entry);

private:
  Ui_searchDialog ui;
};

#endif

