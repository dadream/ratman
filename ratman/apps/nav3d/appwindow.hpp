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
// FIXME SUPERHACK: ADDED LINE #define QT_NO_DEBUG_STREAM 1 in file
// QT src/network/qtnetworkinterface.h line 94 to compile this app

#ifndef RATMAN_APPWINDOW_HPP
#define RATMAN_APPWINDOW_HPP

#include "ui_mainwindow.hpp"
#include "xml_config_parser.hpp"

#ifndef QTSINGLEAPPLICATION
#include <vic/ratman/tcp_server.hpp>
#endif

#include <vector>

class qgl_nav3d_scene_view;
class parameters_dialog;
class search_result_dialog;
class bookmarks_dialog;
class about_dialog;

namespace ratman {
  class decorated_terrain_view;
  class terrain_renderable;
  class active_renderable;
  class snapshots;
  class geonames_search;
  class terrain_placenames;
  class bookmarks_service;
  class control_buttons;
}

/**
 * Main ratman application window, handles all connections between 
 * application and other windows, and the initialization stuff.
 * Uses mainwindow.ui interface
 */
class AppWindow : public QMainWindow {

  Q_OBJECT

public:
  AppWindow(ratman::xml_config_parser& parser, QWidget *parent = 0);
  ~AppWindow();


public slots:

  void handling_msg(const QString& msg);

  void handling_layer_checked(ratman::active_renderable* layer, bool b);

  void handling_history_prev();
  void handling_screenshot();
  void insert_bookmark();
  void update_bookmarks_service();
  void handling_history(bool b);
  void handling_controls(bool b);
  void enable_light(bool b);

#ifndef QTSINGLEAPPLICATION
  void handling_server_request();
#endif
  
  void handling_bookmarks_dialog_closed();
  void handling_sky_dialog_closed();

  void show_about();

  // Search 
  void search();
  void goto_location(std::size_t engine, std::size_t entry);
  void goto_location(double lon, double lat);
  void goto_place(QTreeWidgetItem *current_item);
  void print();

protected:

  void closeEvent(QCloseEvent *event);

  void create_treewidget_layers();
  void recursive_create_treewidget(QTreeWidget* tree_widget,
				   QTreeWidgetItem* parent,
				   ratman::active_renderable* layer);
  void create_terrain_widget(QTreeWidget* tree_widget,
                             QTreeWidgetItem* parent,
                             ratman::terrain_renderable* terrain);

private:
  Ui::MainWindow ui_;
  parameters_dialog* p_dlg_;
  search_result_dialog* search_dialog_;
  bookmarks_dialog* bookmarks_dialog_;
  about_dialog* about_dialog_;

  std::vector<ratman::geonames_search*> search_engines_;

  qgl_nav3d_scene_view *qgl_widget_;
  ratman::snapshots *snapshots_;
  ratman::control_buttons *control_buttons_;
  ratman::decorated_terrain_view *terrain_view_;
  ratman::terrain_placenames* bookmarks_layer_;
  ratman::bookmarks_service* bookmarks_engine_;

#ifndef QTSINGLEAPPLICATION
  ratman::tcp_server server_;
#endif

};

#endif // RATMAN_APPWINDOW_HPP
