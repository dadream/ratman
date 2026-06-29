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
#include <GL/glew.h>
#include "appwindow.hpp"
#include <QAction>
#include <QCloseEvent>
#include <QCursor>
#include <QImage>
#include <QPainter>
#include <QPixmap>
#include <QSize>
#include <QString>
#include <QStringList>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QVBoxLayout>
#include <QWidget>

#include <QRadioButton>
#include <QCheckBox>
#include <QFileDialog>
#include <QRegExp>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>

#include <vic/ratman/compass.hpp>
#include <vic/ratman/control_buttons.hpp>
#include <vic/ratman/fixed_label.hpp>
#include <vic/ratman/atmosphere.hpp>
#include <vic/ratman/snapshots.hpp>
#include <vic/ratman/decorated_terrain_view.hpp>
#include <vic/ratman/terrain_renderable.hpp>
#include <vic/ratman/geonames_service.hpp>
#include <vic/ratman/terrain_placenames.hpp>
#include <vic/ratman/bookmarks_service.hpp>
#include <vic/ratman/copyright.hpp>
#include <vic/ratman/logo.hpp>


#ifdef ITEM3D
#include <vic/ratman/item3d_manager.hpp>
#include <vic/ratman/item3d_factory.hpp>
#endif

#include <vic/cbdam/base/terrain_model.hpp>

#include "config.hpp"

#include "layer_check_box.hpp"
#include "base_layers_button_group.hpp"
#include "overlay_layers_button_group.hpp"
#include "qgl_nav3d_scene_view.hpp"
#include "graphics/rgb.xpm"
#include "graphics/bookmark_icon.xpm"

#include "parameters_dialog.hpp"
#include "search_result_dialog.hpp"
#include "about_dialog.hpp"
#include "bookmarks_dialog.hpp"
#include "bookmark_item.hpp"
#include <iostream>
#include <cassert>

static bool get_decimal_coord(const QString& str, double& lon, double& lat) {
  std::string stru = str.toUpper().toStdString();

  bool result = false;
  if (sscanf(stru.c_str(),
	     "%lf %lf", &lon, &lat) == 2) {
    result = true;
  } else if (sscanf(stru.c_str(),
	     "%lf E %lf N", &lon, &lat) == 2) {
    result = true;
  } else if (sscanf(stru.c_str(),
	     "%lf N %lf E", &lat, &lon) == 2) {
    result = true;
  }
  return result;
}

AppWindow::AppWindow(ratman::xml_config_parser& parser, QWidget *parent) :
  QMainWindow(parent) {
  assert(parser.decorated_terrain_view());
  terrain_view_=parser.decorated_terrain_view();
  search_engines_=parser.search_engines();

  ratman::active_renderable *post_group = new ratman::active_renderable(terrain_view_,tr("Options").toStdString());
  post_group->set_priority(9999); // Render after 
  post_group->set_active(true);
  terrain_view_->insert_decoration_layer(post_group);

  ratman::active_renderable *pre_group = new ratman::active_renderable(terrain_view_,tr("Background").toStdString());
  pre_group->set_priority(-9999); // Render before
  pre_group->set_active(true);
  terrain_view_->insert_decoration_layer(pre_group);

  ratman::compass *compass = new ratman::compass(terrain_view_,tr("Compass").toStdString());
  compass->set_active(true);
  compass->set_priority(9999); 
  post_group->insert_child(compass);

  snapshots_ = new ratman::snapshots(terrain_view_,tr("Snapshot").toStdString());
  snapshots_->set_active(true);
  snapshots_->set_selectable(false);
  snapshots_->set_priority(9999); 
  post_group->insert_child(snapshots_);

  //logo
  ratman::logo* logo = new ratman::logo(terrain_view_, "logo");
  logo->set_priority(9999); // Render after
  logo->set_active(true);
  logo->set_selectable(true);
  terrain_view_->insert_decoration_layer(logo);


  // Copyright

  ratman::copyright* copyright = new ratman::copyright(terrain_view_, "copyright_group", 14.0f, 
						       ratman::vector4f_t(1.0f, 1.0f, 1.0f, 1.0f), ratman::point2f_t(250.0f,60.0f));
  copyright->set_priority(9999); // Render after 
  copyright->set_active(true);
  copyright->set_selectable(false);
  terrain_view_->insert_decoration_layer(copyright);

  // Atmosphere
  ratman::atmosphere *atmosphere = new ratman::atmosphere(terrain_view_, tr("Sky").toStdString());
  atmosphere->set_active(true);
  atmosphere->set_priority(-9999);
  pre_group->insert_child(atmosphere);

  terrain_view_->set_atmosphere(atmosphere);
  p_dlg_ = new parameters_dialog(this);
  p_dlg_->set_tool(atmosphere);

  about_dialog_  = new about_dialog(this);
 
#ifdef ITEM3D
  // Item3d Manager
  ratman::item3d_manager* manager = ratman::item3d_manager::instance();
  if ( manager->is_active() ) {
      manager->set_priority(0); //Render after all?
      manager->set_selectable(false); // Must be always hiden and not appear on the interface
      manager->set_atmosphere(atmosphere);
      terrain_view_->insert_decoration_layer(manager);
  }
#endif

  search_dialog_ = new search_result_dialog(search_engines_, this);
  search_dialog_->setVisible(false);

  bookmarks_dialog_ = new bookmarks_dialog(this,ratman::config::instance()->home_dir().append("bookmarks.kml"));
  bookmarks_dialog_->setVisible(false);
  
  bookmarks_engine_ = new ratman::bookmarks_service( "", "", bookmarks_dialog_->bookmarks_container() );
  QImage* b_icon = new QImage(bookmark_icon_xpm);
  bookmarks_layer_ = new ratman::terrain_placenames(terrain_view_,
						    tr("Bookmarks").toStdString(),
						    bookmarks_engine_,
						    18.0,
						    sl::vector4f(255, 204, 0, 255),
						    100.0,
						    100000.0,
						    50,
						    b_icon
						    );
  
  bookmarks_layer_->set_active(true);
  bookmarks_layer_->set_priority(9998); 
  post_group->insert_child(bookmarks_layer_);

  ui_.setupUi(this);
  
  cbdam::terrain_model *tm=terrain_view_->terrain_layer()->model();
  std::string srs = tm->srs();

  // QGL VIEWER
  qgl_widget_ = new qgl_nav3d_scene_view(terrain_view_, 
					 parser.camera_uvh_box(),
					 parser.camera_reset_position_uvh(),
					 ui_.centralwidget);
  qgl_widget_->setObjectName(QString::fromUtf8("renderingFrame"));
  qgl_widget_->setCursor(QCursor(static_cast<Qt::CursorShape>(2)));

  // set rendering frame policy
  qgl_widget_->setSizePolicy(ui_.renderingFrame->sizePolicy());
  qgl_widget_->setMinimumSize(QSize(640, 480));

  //sostituisco il frame usato con il designer con il frame OpenGL
  ui_.renderingFrame->hide();
  ui_.vboxLayout1->removeWidget(ui_.renderingFrame);
  ui_.vboxLayout1->insertWidget(1, qgl_widget_, 0, 0);
 
  //Sistemo i layout dei Dock Widgets
  //QVBoxLayout *layersDockLayout = new QVBoxLayout;
  //layersDockLayout->addWidget(ui_.layersTreeWidget);
  //ui_.layersDockWidgetContents->setLayout(layersDockLayout);
  
  create_treewidget_layers();
  ui_.layersdockWidget->setVisible(false);

  QAction* close_layers = ui_.layersdockWidget->toggleViewAction();
  connect(close_layers, SIGNAL(toggled(bool)), ui_.layersButton, SLOT(setChecked(bool)));


  //Additional navigation panel
  control_buttons_ = new ratman::control_buttons(terrain_view_,tr("Control Buttons").toStdString());
  control_buttons_->set_active(false);
  control_buttons_->set_priority(9999); 
  post_group->insert_child(control_buttons_);

  // CONNECTION

  connect(ui_.homeButton,SIGNAL(pressed()),qgl_widget_,SLOT(reset()));
  connect(ui_.resetButton,SIGNAL(pressed()),qgl_widget_,SLOT(reset()));
  connect(ui_.resetTiltButton,SIGNAL(pressed()),qgl_widget_,SLOT(reset_tilt_pressed()));
  connect(ui_.resetNorthButton,SIGNAL(pressed()),qgl_widget_,SLOT(reset_north_pressed()));

  connect(ui_.prevButton,SIGNAL(pressed()),this,SLOT(handling_history_prev()));
  connect(ui_.screenshotButton,SIGNAL(pressed()),this,SLOT(handling_screenshot()));
  connect(ui_.printButton,SIGNAL(pressed()),this,SLOT(print()));

  connect(ui_.layersButton,SIGNAL(toggled(bool)),ui_.layersdockWidget,SLOT(setVisible(bool))); 
  connect(ui_.skyButton,SIGNAL(toggled(bool)),p_dlg_,SLOT(setVisible(bool)));
  connect(ui_.skyButton,SIGNAL(toggled(bool)),this,SLOT(enable_light(bool)));
  connect(ui_.historyButton,SIGNAL(toggled(bool)),this,SLOT(handling_history(bool)));
  connect(ui_.bookmarksButton,SIGNAL(toggled(bool)),bookmarks_dialog_,SLOT(setVisible(bool)));
  connect(ui_.controlsButton,SIGNAL(toggled(bool)),this,SLOT(handling_controls(bool)));

  connect(ui_.aboutButton,SIGNAL(pressed()),this,SLOT(show_about()));

#ifndef QTSINGLEAPPLICATION 
  // TCP Server
  connect(&server_, SIGNAL(ready_param()), this, SLOT(handling_server_request()));
#endif

  // Search
  connect(ui_.lineEdit, SIGNAL(returnPressed()), this, SLOT(search()));
  connect(search_dialog_,  SIGNAL(item_doubleclicked(std::size_t, std::size_t)), this, SLOT(goto_location(std::size_t, std::size_t)));

  // Bookmarks
  connect(bookmarks_dialog_, SIGNAL(item_doubleclicked(QTreeWidgetItem*)), this, SLOT(goto_place(QTreeWidgetItem*)));
  connect(bookmarks_dialog_, SIGNAL(insert_bookmark()), this, SLOT(insert_bookmark()));
  connect(bookmarks_dialog_, SIGNAL(modified_bookmarks()), this, SLOT(update_bookmarks_service()));

  connect(bookmarks_dialog_, SIGNAL(bookmarks_dialog_close()), this,SLOT(handling_bookmarks_dialog_closed()));
  connect(p_dlg_, SIGNAL(sky_dialog_close()), this,SLOT(handling_sky_dialog_closed()));
 
   // LABELS
  connect(qgl_widget_,SIGNAL(signal_latitute_changed(QString)),ui_.latitudeValue,SLOT(setText(QString)));
  connect(qgl_widget_,SIGNAL(signal_longitude_changed(QString)),ui_.longitudeValue,SLOT(setText(QString)));
  connect(qgl_widget_,SIGNAL(signal_elevation_changed(QString)),ui_.elevationValue,SLOT(setText(QString)));
  connect(qgl_widget_,SIGNAL(signal_quota_changed(QString)),ui_.quoteValue,SLOT(setText(QString)));
  connect(qgl_widget_,SIGNAL(signal_speed_changed(QString)),ui_.speedValue,SLOT(setText(QString)));
  connect(qgl_widget_,SIGNAL(signal_yaw_changed(QString)),ui_.directionValue,SLOT(setText(QString)));
  connect(qgl_widget_,SIGNAL(signal_tilt_changed(QString)),ui_.inclinationValue,SLOT(setText(QString)));

  //INIZIALIZE SPECIAL FEATURES
  enable_light(true);
 
  //INIZIALIZE MAINWINDOW MESSAGES
  statusBar()->showMessage(tr("3D GEO VIEWER Ready - Press Left Mouse Button to Translate, Right Mouse Button to Rotate and Use Mouse Wheel to Zoom"),60000);
  setWindowTitle(tr("3D GEO RATMAN"));
}


AppWindow::~AppWindow() {
  std::vector<ratman::geonames_search*> search_engines_;
  for (std::size_t i=0; i<search_engines_.size(); ++i) {
    if (search_engines_[i]) delete search_engines_[i];
    search_engines_[i]=0;
  }
  search_engines_.clear();
}

void AppWindow::closeEvent(QCloseEvent *event) {
  p_dlg_->close();
  search_dialog_->close();
  bookmarks_dialog_->close();
  about_dialog_->close();
  event->accept();
 }

void AppWindow::recursive_create_treewidget(QTreeWidget* tree_widget,
					    QTreeWidgetItem* parent,
					    ratman::active_renderable* layer) {
  
  //std::cerr << "=== BEGIN: " << layer->name() << std::endl;
  if (layer->is_selectable()) {

    QString label= QString(layer->name().c_str());
  
    QTreeWidgetItem *item;
    if (parent) { 
      item = new QTreeWidgetItem(parent);
    } else {
      item  = new QTreeWidgetItem(tree_widget);
    }
    layer_check_box* checkBox = new layer_check_box(layer,0); 
    checkBox->setObjectName(label);
    checkBox->setText(label);
    if(layer->icon()) {
      QPixmap icon_pix = QPixmap::fromImage(*layer->icon());
      checkBox->setIcon(icon_pix);
    }
    if (layer->is_active()) {
      checkBox->setCheckState(Qt::Checked);
    } else {      
      checkBox->setCheckState(Qt::Unchecked);
    }
    tree_widget->setItemWidget(item,0,checkBox);
    tree_widget->expandItem(item);
    connect(checkBox,SIGNAL(is_active(ratman::active_renderable*,bool)),this,SLOT(handling_layer_checked(ratman::active_renderable*,bool))); 
    
    for (std::size_t i=0; i<layer->child_count(); ++i) {
      recursive_create_treewidget(tree_widget, item, layer->child(i));
    }
  }
  std::cerr << "=== END: " << layer->name() << std::endl;
}

void AppWindow::create_terrain_widget(QTreeWidget* tree_widget,
                                      QTreeWidgetItem* parent,
                                      ratman::terrain_renderable* terrain) {
  std::cerr << "=== BEGIN TERRAIN: " << terrain->name() << std::endl;

  assert(terrain);
  assert(terrain->model());

  cbdam::terrain_model *tm = terrain->model();
  
  QTreeWidgetItem *item_base;
  if (parent) { 
    item_base = new QTreeWidgetItem(parent);
  } else {
    item_base  = new QTreeWidgetItem(tree_widget);
  }

  // BASE LAYERS

  item_base->setText(0,tr("Base Layers"));
  tree_widget->expandItem(item_base);

  base_layers_button_group* baseButtons = new base_layers_button_group(terrain, 0);

  // image embedded in the code: rgb_xpm
  QPixmap icon_pix_base = QPixmap::fromImage(QImage(rgb_xpm));

  for (std::size_t i=0; i<tm->base_color_layer_count(); ++i) {
    QTreeWidgetItem *child = new QTreeWidgetItem(item_base);
    QRadioButton* rb = new QRadioButton(QString(tm->base_color_layer(i)->id().c_str()));
    rb->setIcon(icon_pix_base);
    rb->setChecked(i==0);
    tree_widget->setItemWidget(child, 0, rb);
    tree_widget->expandItem(child);
    baseButtons->addButton(rb, i);
  }

  // OVERLAY LAYERS

  QTreeWidgetItem *item_overlay;
  if (parent) { 
    item_overlay = new QTreeWidgetItem(parent);
  } else {
    item_overlay  = new QTreeWidgetItem(tree_widget);
  }

  item_overlay->setText(0,tr("Overlay Layers"));
  tree_widget->expandItem(item_overlay);

  overlay_layers_button_group* overlayButtons = new overlay_layers_button_group(terrain, 0);
  //  connect(overlayButtons,SIGNAL(is_active(int)),this,SLOT(handling_color_copyright(int))); 
  
  QPixmap icon_pix_overlay = QPixmap::fromImage(QImage(rgb_xpm));
  //  QImage icon = QImage(rgb_xpm);// QPixmap::fromImage(*layer->icon());
  //  QPixmap icon_pix = QPixmap::fromImage(icon);

  for (std::size_t i=0; i<tm->overlay_color_layer_count(); ++i) {
    QTreeWidgetItem *child = new QTreeWidgetItem(item_overlay);
    QCheckBox* rb = new QCheckBox(QString(tm->overlay_color_layer(i)->id().c_str()));
    rb->setCheckable(true);
    rb->setIcon(icon_pix_overlay);
    rb->setChecked(tm->is_overlay_color_layer_active(i));
    tree_widget->setItemWidget(child, 0, rb);
    tree_widget->expandItem(child);
    overlayButtons->addButton(rb, i);
  }

  //std::cerr << "=== END: " << terrain->name() << std::endl;
  
}

void AppWindow::create_treewidget_layers() {
  ui_.layersTreeWidget->headerItem()->setText(0, tr("Layers"));
  ui_.layersTreeWidget->clear();
  for (std::size_t i=0; i<terrain_view_->decorations_root()->child_count(); ++i) {
    recursive_create_treewidget(ui_.layersTreeWidget, NULL, terrain_view_->decorations_root()->child(i));
  }

  // Choose among different color layers if more than one available
    create_terrain_widget(ui_.layersTreeWidget, NULL, terrain_view_->terrain_layer());
}

void AppWindow::handling_msg(const QString& msg) {
  SL_TRACE_OUT(1) << msg.toStdString() << std::endl;
  if (!msg.isEmpty()) {
    QStringList str=msg.split(" ");
    if (str.size()==2) {
      bool ok_lon;
      double lon=str[0].toDouble(&ok_lon);
      bool ok_lat;
      double lat=str[1].toDouble(&ok_lat);
      if (ok_lon && ok_lat) {
	goto_location(lon, lat);
      }
    }
  }
}


void AppWindow::handling_layer_checked(ratman::active_renderable* layer, bool b) {
  layer->set_active(b);
}

void AppWindow::handling_history_prev(){
  qgl_widget_->camera_controller().set_oriented_position(snapshots_->go_prev_slot()->oriented_pos());
}

void AppWindow::handling_screenshot(){
  QImage snap_shot_full = qgl_widget_->grabFrameBuffer(false);

  QString file_name = QFileDialog::getSaveFileName(this, tr("Save Image"),
						   "",
						   tr("Images (*.png)"));
  if (!file_name.isEmpty()) {
    snap_shot_full.save(file_name,"png",-1); 
    QString message = QString("Image saved!");
    statusBar()->showMessage(message,8000);
  }

}

void AppWindow::insert_bookmark(){
  //TODO store the ray
  ratman::point3d_t lonlat_pos = qgl_widget_->terrain()->WGS84_lonlat_from_xyz(qgl_widget_->camera_controller().
									       get_oriented_position().
									       ground_target_xyz());

  // get terrain intersection at camera ground target lonlat.
  std::pair<bool, double> r = qgl_widget_->terrain()->current_representation_ground_elevation_from_WGS84_lonlat(ratman::point2d_t(lonlat_pos[0], lonlat_pos[1]));
  ratman::point3d_t pos = lonlat_pos;
  pos[2] += r.first ? r.second : 0;  
  //  std::cerr << "lonlat_pos " << lonlat_pos[0] << " "  << lonlat_pos[1] << " "  << lonlat_pos[2] << std::endl;
  //  std::cerr << "ground h   " << pos[2] << std::endl;

  double camera_range = qgl_widget_->camera_controller().get_oriented_position().distance_from_target();
  double camera_tilt = qgl_widget_->camera_controller().get_oriented_position().tilt();
  double camera_heading = qgl_widget_->camera_controller().get_oriented_position().yaw();
 
  //std::cerr << "ground target "<<pos<<std::endl;

  ratman::point3d_t o = ratman::point3d_t(camera_range,camera_tilt,camera_heading);
  bookmarks_dialog_->insert(pos,o);
}

void AppWindow::handling_history(bool b){
  snapshots_->set_show_tape(b);
}
void AppWindow::handling_controls(bool b){
  control_buttons_->set_active(b);
  control_buttons_->set_show(b);
}
void AppWindow::enable_light(bool b){
  terrain_view_->terrain_layer()->set_shading_enabled(b);
}

void AppWindow::search() {
  double lon=0;
  double lat=0;
  if (get_decimal_coord(ui_.lineEdit->text(), lon, lat)) {
    std::cerr << "GOTO: " << lon << " " << lat << std::endl;
    goto_location(lon, lat);
  } else {

    // FIND QUERY BOX
    std::pair<ratman::point2d_t, double> query_region = qgl_widget_->current_WGS84_lonlat_center_radius();

    ratman::aabox2d_t query_box = ratman::aabox2d_t(ratman::point2d_t(query_region.first[0]-query_region.second,
								      query_region.first[1]-query_region.second),
						    ratman::point2d_t(query_region.first[0]+query_region.second,
								      query_region.first[1]+query_region.second));
    search_dialog_->search(ui_.lineEdit->text(), query_box);
  }
}

void AppWindow::goto_location(std::size_t engine, std::size_t entry) {
  SL_TRACE_OUT(1) << std::endl;
  std::vector<ratman::geonames_entry> search_list=search_engines_[engine]->last_search_results();
  std::string name = search_list[entry].name();
  ratman::point3d_t lon_lat_alt = search_list[entry].location();
  goto_location(lon_lat_alt[0], lon_lat_alt[1]);
}

void AppWindow::goto_place(QTreeWidgetItem *current_item) {
  bookmark_item* bookmark = dynamic_cast<bookmark_item*>(current_item);

  ratman::point3d_t lon_lat_alt = bookmarks_dialog_->bookmarks_container()->location(bookmark->entry());
  ratman::point3d_t orientation = bookmarks_dialog_->bookmarks_container()->orientation(bookmark->entry()); 
  
  ratman::point3d_t lon_lat_alt_offset = lon_lat_alt;
  double alt_offset = 500.0;
  lon_lat_alt_offset[2] += alt_offset;
  
  ratman::point3d_t xyz = terrain_view_->terrain_layer()->model()->xyz_from_WGS84_lonlat(lon_lat_alt_offset);
  

  qgl_widget_->goto_oriented_location(xyz,orientation);

  QString message = QString("Go to LAT %1 N  LONG %2 E Alt %3 m")
    .arg(lon_lat_alt[1])
    .arg(lon_lat_alt[0])
    .arg(lon_lat_alt[2])
    ;
  statusBar()->showMessage(message,8000);
 
}


void AppWindow::goto_location(double lon, double lat) {
  SL_TRACE_OUT(1) << std::endl;
  ratman::point2d_t lon_lat = ratman::point2d_t(lon, lat);
  cbdam::terrain_model *tm=terrain_view_->terrain_layer()->model();
  std::pair<bool, double> value = tm-> current_representation_ground_elevation_from_WGS84_lonlat(lon_lat);
  double alt = value.first ? value.second : 0.0;
  std::size_t alt_offset = 3000;
  ratman::point3d_t lon_lat_alt = ratman::point3d_t(lon, lat, alt+alt_offset);
  ratman::point3d_t xyz = tm->xyz_from_WGS84_lonlat(lon_lat_alt);
  qgl_widget_->goto_location(xyz);
  QString message = QString("Go to LAT %1 N  LONG %2 E")
    .arg(lon_lat[1])
    .arg(lon_lat[0])
    ;
  statusBar()->showMessage(message,8000);
}

#ifndef QTSINGLEAPPLICATION
void AppWindow::handling_server_request(){
  SL_TRACE_OUT(1) << std::endl;
  if ( (server_.request_type() == ratman::tcp_server::SERVER_LONLAT ) ||
       (server_.request_type() == ratman::tcp_server::SERVER_FILE )) {
    goto_location(server_.lon(), server_.lat());
  }

}
#endif

void AppWindow::update_bookmarks_service(){
  bookmarks_engine_->load_bookmarks(bookmarks_dialog_->bookmarks_container());
  bookmarks_layer_->async_clear_cache();
}

void AppWindow::handling_bookmarks_dialog_closed(){
  ui_.bookmarksButton->setChecked(false);
}

void AppWindow::handling_sky_dialog_closed(){
   ui_.skyButton->setChecked(false);
}

void AppWindow::print(){
	QImage image = qgl_widget_->grabFrameBuffer();

	QPrinter printer(QPrinter::HighResolution);
    printer.setOutputFormat(QPrinter::NativeFormat);

    QPrintDialog dialog(&printer, this);
    if (dialog.exec() == QDialog::Accepted) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = image.size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(),size.height());
        painter.setWindow(image.rect());
        painter.drawImage(0, 0, image);
    }
}
void AppWindow::show_about(){
  about_dialog_->show();
}
