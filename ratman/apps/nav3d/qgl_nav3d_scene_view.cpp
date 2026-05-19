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
#include "qgl_nav3d_scene_view.hpp"
#include "meteo_dialog.hpp"
#include <vic/ratman/meteo_data.hpp>
#include <QDateTime>

qgl_nav3d_scene_view::qgl_nav3d_scene_view(ratman::decorated_terrain_view* terrain_view,
					   const ratman::aabox3d_t& camera_uvh_bbox,
					   const ratman::oriented_position& reset_position,
					   QWidget *parent) 
  : ratman::qgl_scene_view(terrain_view, camera_uvh_bbox, reset_position, parent) {
  meteo_info_ = new  meteo_dialog(this);
}

qgl_nav3d_scene_view::~qgl_nav3d_scene_view() {
}

void qgl_nav3d_scene_view::handling_event(void *data) {
  ratman::meteo_data* meteo = (ratman::meteo_data*) data;
  if(!meteo) return;

  QDateTime date_local = QDateTime::currentDateTime();
  QDateTime date = date_local.toUTC();
  date.setTime(QTime(meteo->hour(), meteo->min()));
  date_local = date.toLocalTime();
  QDate d=date_local.date();
  QTime t=date_local.time();
  QString text;
  QString val;
  text += "<HTML>\n";
  text += " <HEAD>\n";
  text += "   <TITLE>"+tr("Meteorological Information")+"</TITLE>";
  text += " </HEAD>\n";
  text += " <BODY>\n";
  text += "   <B>"+tr("Station")+":</B> " + meteo->station_name() + "<BR>\n";
  val.sprintf("%02d/%02d/%04d %02d:%02d",d.day(),d.month(),
	      d.year(),t.hour(),t.minute());
  text += "   <SMALL>"+tr("Weather Bulletin -")+" " + val + "</SMALL><BR>\n";
  text += "   <HR>\n";

  text += "   <TABLE>\n";
  if(meteo->temperature() != ratman::NOSIG) {
    val.sprintf("%4.1f &deg;C",meteo->temperature());
  } else {
    val = " - ";
  }
  text += "     <TR><TD><B>"+tr("Temperature")+":</B></TD><TD>" + val + "</TD></TR>\n";

  if(meteo->temperature() != ratman::NOSIG && 
     meteo->dew_point() != ratman::NOSIG &&
     meteo->wind_speed() != ratman::NOSIG) {
    val.sprintf("%4.1f &deg;C",meteo->perceived_temperature());
  } else {
    val = " - ";
  }
  text += "     <TR><TD><B>"+tr("Perceived Temperature")+":</B></TD><TD>" + val + "</TD></TR>\n";

  if(meteo->pressure() != ratman::NOSIG) {
    val.sprintf("%5.0f hPa",meteo->pressure());
  } else {
    val = " - ";
  }
  text += "     <TR><TD><B>"+tr("Pressure")+":</B></TD><TD>" + val + "</TD></TR>\n";

  if(meteo->temperature() != ratman::NOSIG) {
    val.sprintf("%4.1f %%",meteo->r_humidity());
  } else {
    val = " - ";
  }
  text += "     <TR><TD><B>"+tr("Relative humidity")+":</B></TD><TD>" + val + "</TD></TR>\n";

  if(meteo->visibility() != ratman::NOSIG) {
    if(meteo->visibility() < ratman::VISIBILITY_MAX) {
      val.sprintf("%4.0f m",meteo->visibility());
    } else {
      val = " MAX ( > 10000 m )";
    }
  } else {
    val = " - ";
  }
  text += "     <TR><TD><B>"+tr("Visibility")+":</B></TD><TD>" + val + "</TD></TR>\n";
 
  float degree=meteo->wind_degree();
  if(degree != ratman::NOSIG) {
    if(degree == ratman::VARIABLE) {
      val = tr("Variable");
    } else {
      val = meteo->get_wind_direction(degree) + " [" + meteo->get_wind_name(degree) + "]";
    }
  } else {
    val = " - ";
  }
  val += "<BR>\n";
  float degree_min=meteo->wind_degree_variations_min();
  float degree_max=meteo->wind_degree_variations_max();
  if(degree_min != ratman::NOSIG) {
    val += tr("Variable from %1 to %2").arg(meteo->get_wind_direction(degree_min)).arg(meteo->get_wind_direction(degree_max));
    val += "<BR>\n";
  }
  text += "     <TR><TD><B>"+tr("Wind Direction")+":</B></TD><TD>" + val + "</TD></TR>\n";
  
  float speed=meteo->wind_speed();
  if(speed != ratman::NOSIG) {
    QString forza;
    forza.sprintf("%d",meteo->get_wind_beaufort(speed));
    val = tr("Force") + " " + forza + " [" +  meteo->get_wind_status(speed) + "]";
  } else {
    val = " - ";
  }
  val += "<BR>\n";
  float gusting_speed=meteo->wind_gusting_speed();
  if(gusting_speed != ratman::NOSIG) {
    QString forza;
    forza.sprintf("%d",meteo->get_wind_beaufort(gusting_speed));
    val += tr("Gusting Speed Force") + " " + forza;
    val += "<BR>\n";
  }
  text += "     <TR><TD><B>"+tr("Wind Speed")+":</B></TD><TD>" + val + "</TD></TR>\n";

  int id=meteo->worst_sky_condition();
  if(id != ratman::MISSING) {
    text += "     <TR><TD><B>"+tr("Sky Conditions")+":</B></TD><TD>" + meteo->sky_conditions_label(id) + "</TD></TR>\n";
  }

  std::vector<int> phenomena = meteo->phenomena_list();
  if(phenomena.size()>0) {
    text += "     <TR><TD><B>"+tr("Phenomena")+":</B></TD><TD>";
    for(unsigned i=0; i<phenomena.size(); ++i) {
      text += meteo->phenomena_label(phenomena[i]) + "<BR>";
    } 
    text += "</TD></TR>\n";
  }
  text += "   </TABLE>\n";
  text += " </BODY>\n";
  text += "</HTML>\n";
  meteo_info_->meteo_info()->setHtml(text);
  meteo_info_->set_meteo_icon(meteo->meteo_icon_pixmap());
  meteo_info_->show();
}
