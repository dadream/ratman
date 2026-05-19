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
#ifndef BOOKMARKS_DIALOG_HPP
#define BOOKMARKS_DIALOG_HPP

#include "ui_bookmarks.hpp"
#include <vic/ratman/bookmarks.hpp>
#include <vic/ratman/ratman.hpp>

#include <QCloseEvent>
#include <iostream>
#include <vector>

/**
 * Dialog containing bookmarks, uses bookmarks.ui interface.
 * Bookmark information is derived by ratman::bookmarks.
 */
 class bookmarks_dialog : public QDialog {

    Q_OBJECT

  public:
    typedef ratman::point3d_t point3d_t;

  public:

    bookmarks_dialog(QWidget *parent = 0, QString filename = "");
    ~bookmarks_dialog();

    QTreeWidget* treeWidget(){
      return ui.treeWidget;
    }

    QGridLayout *gridLayout(){
      return ui.gridLayout;
    }

    ratman::bookmarks* bookmarks_container(){
      return bookmarks_;
    }

 
public slots:

    void insert(const ratman::point3d_t pos,const ratman::point3d_t o);
 
    void modify_name();
    void delete_bookmark();

    void set_current_description(QString s);
  
    void inizialize_placenames();
    void reload_tree();
  
    void handling_doubleclicking(QTreeWidgetItem *item, int col);
    void handling_selected(QTreeWidgetItem *item, int col);

    signals:
    void item_doubleclicked(QTreeWidgetItem *item);
    void insert_bookmark();
    void modified_bookmarks();
    void current_nameplace(QString);

    void bookmarks_dialog_close();

  
  private:
    Ui_bookmarksDialog ui;

  protected:

    void closeEvent(QCloseEvent *event);

    ratman::bookmarks* bookmarks_;
    bool inizialized_;
    QString bookmark_text_;
    QString file_path_;
 
  };



#endif
