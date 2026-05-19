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
#include "parameters_dialog.hpp"
#include <assert.h>
#include <vic/ratman/atmosphere.hpp>

parameters_dialog::parameters_dialog(QWidget *parent)
  : QDialog(parent)
{
  ui.setupUi(this);
  assert(parent);

  connect(ui.AzimuthSlider, SIGNAL(valueChanged(int)), this, SLOT( update_azimuth(int)));
  connect(ui.AltitudeSlider, SIGNAL(valueChanged(int)), this, SLOT(update_altitude(int)));
  connect(ui.TurbiditySlider, SIGNAL(valueChanged(int)), this, SLOT(update_turbidity(int)));

  connect(ui.TimeSlider, SIGNAL(valueChanged(int)), this, SLOT(update_daytime(int)));
  connect(ui.TimeSlider_2, SIGNAL(valueChanged(int)), this, SLOT(update_minutes(int)));

  connect( ui.dateEdit,SIGNAL(dateChanged(QDate)),this,SLOT(update_date(QDate)) );

  connect( ui.RealTimeButton,SIGNAL(toggled(bool)),this,SLOT(switch_sunmode(bool)) );
  
  connect(ui.FovSlider, SIGNAL(valueChanged(int)), this, SLOT(update_fov_y(int)));

}

void parameters_dialog::set_tool(ratman::atmosphere* a){
  atm_ = a; 
  if (atm_ == 0) {
    SL_TRACE_OUT(-1) << "SET NULL ATMOSPHERE" << std::endl;
  }
  atm_->set_daytime(ui.TimeSlider->value());
  atm_->set_minutes(ui.TimeSlider_2->value());
  atm_->set_date( ui.dateEdit->date().dayOfYear() );

  atm_->set_azimuth(ui.AzimuthSlider->value());
  atm_->set_altitude(ui.AltitudeSlider->value());
  atm_->set_turbidity(ui.TurbiditySlider->value());
  atm_->set_fov_y(ui.FovSlider->value());
}

void parameters_dialog::update_sky(){
  atm_->compute_sky();
}
void parameters_dialog::update_azimuth(int i){
  atm_->set_azimuth(i);
  atm_->compute_sky();
}
void parameters_dialog::update_altitude(int i){
  atm_->set_altitude(i);
  atm_->compute_sky();
}
void parameters_dialog::update_turbidity(int i){
  atm_->set_turbidity(i);
  atm_->compute_sky();
}
void parameters_dialog::update_daytime(int i){
  atm_->set_daytime(i);
}
void parameters_dialog::update_minutes(int i){
  atm_->set_minutes(i);
}
void parameters_dialog::update_date(QDate date){
  atm_->set_date( date.dayOfYear() );
}

void parameters_dialog::update_fov_y(int i){
  atm_->set_fov_y(i);
}


void parameters_dialog::switch_sunmode(bool b){
  if (b){
    ui.AzimuthSlider->setEnabled(false);
    ui.AltitudeSlider->setEnabled(false);
    ui.TimeSlider->setEnabled(true);
    ui.TimeSlider_2->setEnabled(true);
    ui.dateEdit->setEnabled(true);
    atm_->set_real_time_mode(true);

    update_daytime(QTime::currentTime().hour());
    update_minutes(QTime::currentTime().minute());
    update_date(QDate::currentDate ());
    ui.lcdTime->display(QTime::currentTime().hour());
    ui.lcdTime_2->display(QTime::currentTime().minute());

    ui.TimeSlider->setValue(QTime::currentTime().hour());
    ui.TimeSlider_2->setValue(QTime::currentTime().minute());
    ui.dateEdit->setDate( QDate::currentDate() );


  }else{
    ui.AzimuthSlider->setEnabled(true);
    ui.AltitudeSlider->setEnabled(true);
    ui.TimeSlider->setEnabled(false);
    ui.TimeSlider_2->setEnabled(false);
    ui.dateEdit->setEnabled(false);
    atm_->set_real_time_mode(false);
  }
}

void  parameters_dialog::closeEvent(QCloseEvent *event){
  emit sky_dialog_close();
  event->accept();
}
