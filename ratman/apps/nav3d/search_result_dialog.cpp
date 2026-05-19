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
#include "search_result_dialog.hpp"
#include "search_result_item.hpp"

#include <vic/ratman/geonames_service.hpp>

search_result_dialog::search_result_dialog(const geonames_engines_t& search_engines,
					   QWidget *parent) : 
  QDialog(parent),
  search_engines_(search_engines) {
  ui.setupUi(this);
  connect(treeWidget(), SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(handling_doubleclicking(QTreeWidgetItem *, int)));
 }


void search_result_dialog::search(const QString& search_text,
				  const ratman::aabox2d_t& query_box) {
  treeWidget()->clear();

  std::size_t result_count = 0;

  for (std::size_t local_global=0; local_global<2; ++local_global) {
    if (local_global==1 && result_count>0) {
      // Nothing to do
    } else {
      // Perform search
      for (std::size_t i=0; i<search_engines_.size(); ++i) {
	if (local_global == 0) {
	  search_engines_[i]->search(search_text.toStdString(),query_box);
	} else {
	  search_engines_[i]->search(search_text.toStdString()); // GLOBAL SEARCH
	}      
	if (search_engines_[i]->last_search_ok() &&
	    search_engines_[i]->last_search_results().size() > 0) {
	  QTreeWidgetItem* title_item  = new QTreeWidgetItem(treeWidget());
	  title_item->setText(0,search_engines_[i]->id().c_str());
	  treeWidget()->expandItem(title_item);
	  std::vector<ratman::geonames_entry> search_list=search_engines_[i]->last_search_results();
	  for (std::size_t j=0; j<search_list.size(); ++j) {
	    ++result_count;
	    search_result_item* search_item  =  new search_result_item(i,j,title_item);
	    search_item->setText(0,search_list[j].name().c_str());
	    treeWidget()->expandItem(search_item);
	  }
	}
      }
    }
  }

  setVisible(true);  
}

void search_result_dialog::handling_doubleclicking(QTreeWidgetItem *item, int /*col*/) {
  std::cerr << "Double click" << std::endl;
  search_result_item* search_item = dynamic_cast<search_result_item*>(item);
  if (search_item) {
    emit item_doubleclicked(search_item->engine(), search_item->entry());
  }
}
