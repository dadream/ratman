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
#include "bookmarks_dialog.hpp"
#include "bookmark_item.hpp"

#include <QCheckBox>
#include <QFile>
#include <QTextStream>

  bookmarks_dialog::bookmarks_dialog(QWidget *parent, QString filename) : 
    QDialog(parent){
    ui.setupUi(this);

    bookmark_text_ = "Bookmark";
    file_path_ = filename;
 
    connect(treeWidget(), SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)), this, SLOT(handling_doubleclicking(QTreeWidgetItem *, int)));
    connect(treeWidget(), SIGNAL(itemClicked(QTreeWidgetItem *, int)), this, SLOT(handling_selected(QTreeWidgetItem *, int)));
    connect(ui.placenameEdit, SIGNAL( textChanged(QString) ), this, SLOT(set_current_description(QString)));
    connect(ui.insertButton, SIGNAL( pressed() ), this, SIGNAL( insert_bookmark() ) );
    connect(ui.modifyButton, SIGNAL( pressed() ), this, SLOT( modify_name() ) );
    connect(ui.deleteButton, SIGNAL( pressed() ), this, SLOT( delete_bookmark() ) );
    connect(this, SIGNAL(current_nameplace(QString)),ui.placenameEdit, SLOT(setText(QString)) );

    bookmarks_ = new ratman::bookmarks();
    bookmarks_->clear_container();
 
    inizialized_ = false;
    inizialize_placenames();
}

  bookmarks_dialog::~bookmarks_dialog(){
    if (bookmarks_) delete bookmarks_;
    bookmarks_ = 0;
  }


  void bookmarks_dialog::insert(const ratman::point3d_t pos, const ratman::point3d_t o) {
    QString description = bookmark_text_;
    bookmarks_->insert(description,pos,o);
    bookmarks_->insert_xml_node(description, pos, o);
    reload_tree();
    bookmarks_->save_kml(file_path_);
    emit modified_bookmarks();
  }

void bookmarks_dialog::reload_tree() {
  treeWidget()->clear();
  treeWidget()->setColumnCount(1);
  int col = 0;
  for (std::size_t i=0; i<bookmarks_->count();i++){
    bookmark_item* bookmark  =  new bookmark_item(i,treeWidget());
    bookmark->setText(col,bookmarks_->name(i));
    treeWidget()->setItemWidget(bookmark,0,0);
    treeWidget()->setCurrentItem(bookmark,col);
    emit current_nameplace(treeWidget()->currentItem()->text(col));
    treeWidget()->expandItem(bookmark);
  }
}

  void bookmarks_dialog::handling_doubleclicking(QTreeWidgetItem *item, int /*col*/) {
    bookmark_item* bookmark = dynamic_cast<bookmark_item*>(item);
    if (bookmark) {
      emit item_doubleclicked(bookmark);
    }
  }

  void bookmarks_dialog::handling_selected(QTreeWidgetItem *item, int col) {
    bookmark_item* bookmark = dynamic_cast<bookmark_item*>(item);
    treeWidget()->setCurrentItem(bookmark,col);
    emit current_nameplace(treeWidget()->currentItem()->text(col));
  }

  void bookmarks_dialog::inizialize_placenames(){
    bookmarks_->load_kml(file_path_);
    reload_tree();
    emit modified_bookmarks();
  }

  void bookmarks_dialog::set_current_description(QString s){
    bookmark_text_ = s;
  }

  void bookmarks_dialog::modify_name(){
    int col = 0;
    bookmark_item* bookmark = dynamic_cast<bookmark_item*>(treeWidget()->currentItem());
    bookmark->setText(col,bookmark_text_);
    if(bookmark->entry()<bookmarks_->count()){
      bookmarks_->set_name(bookmark_text_,bookmark->entry());
      bookmarks_->save_kml(file_path_);
    }
    emit modified_bookmarks();
  }

  void bookmarks_dialog::delete_bookmark(){
    bookmark_item* bookmark = dynamic_cast<bookmark_item*>(treeWidget()->currentItem());
    bookmarks_->delete_bookmark(bookmark->entry());
    bookmarks_->save_kml(file_path_);
    reload_tree();
    emit modified_bookmarks();
  }


void  bookmarks_dialog::closeEvent(QCloseEvent *event){
  emit bookmarks_dialog_close();
  event->accept();
}
