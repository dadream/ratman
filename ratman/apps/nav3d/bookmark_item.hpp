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
#ifndef BOOKMARK_ITEM_HPP
#define BOOKMARK_ITEM_HPP

#include <QTreeWidgetItem>
#include <vic/ratman/ratman.hpp>

/**
 * Items present in the bookmark_dialog
 */
class bookmark_item : public QTreeWidgetItem {

public:

  bookmark_item(std::size_t entry,
		QTreeWidget *parent,
		int type=Type);

  inline std::size_t entry() const {
    return entry_;
  }

  inline void set_entry(std::size_t en){
    entry_=en;
  }

protected:
  
  std::size_t entry_;

};

#endif

