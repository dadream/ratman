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
#include "meteo_data.hpp"
#include <math.h>
#include <QPainter>
#include <iostream>
#include <cassert>

#include "Icons/bkg.xpm"
#include "Icons/nodata.xpm"
#include "Icons/bkn_s.xpm"
#include "Icons/clr_s.xpm"
#include "Icons/few_s.xpm"
#include "Icons/gr_s.xpm"
#include "Icons/ovc_s.xpm"
#include "Icons/ra_s.xpm"
#include "Icons/sct_s.xpm"
#include "Icons/sn_s.xpm"

namespace ratman {

  const int WIND_STATUS_SIZE=13;
  const int WIND_NAME_SIZE=16;

  static float vapor_pressure(float temp) {
    return 6.11*pow(10.0,(7.5*temp/(237.7 +temp)));
  }

  static float wind_speed_kn_to_ms(float v) {
    return 0.5144444*v;
  }

  static float wct(float t, float v) {
    return 13.13+0.62*t-13.95*pow(v,0.16f)+0.4275*t*pow(v,0.16f);
  }
  
  static float humidex(float t, float td) {
    return t+0.5555*(vapor_pressure(td)-10);
  }
  

  meteo_data::meteo_data(const QString& station_name, QObject *parent) :
    QObject(parent) {

    station_name_=station_name;
    timestamp_=-1000;
    
    sky_conditions_metar_.push_back("MISSING");
    sky_conditions_metar_.push_back("CLR");
    sky_conditions_metar_.push_back("FEW");
    sky_conditions_metar_.push_back("SCT");
    sky_conditions_metar_.push_back("BKN");
    sky_conditions_metar_.push_back("OVC");
    sky_conditions_label_.push_back(tr("Missing Data"));
    sky_conditions_label_.push_back(tr("Clear Sky"));
    sky_conditions_label_.push_back(tr("Few Clouds"));
    sky_conditions_label_.push_back(tr("Scattered Clouds"));
    sky_conditions_label_.push_back(tr("Broken Clouds"));
    sky_conditions_label_.push_back(tr("Overcast"));
    
    phenomena_metar_.push_back("ANY");
    phenomena_label_.push_back(tr("Any Phenomena"));
    phenomena_metar_.push_back("DZ");
    phenomena_label_.push_back(tr("Drizzle"));
    phenomena_metar_.push_back("RA");
    phenomena_label_.push_back(tr("Rain"));
    phenomena_metar_.push_back("SN");
    phenomena_label_.push_back(tr("Snow"));
    phenomena_metar_.push_back("SG");
    phenomena_label_.push_back(tr("Snow Grains"));
    phenomena_metar_.push_back("IC");
    phenomena_label_.push_back(tr("Ice Crystals"));
    phenomena_metar_.push_back("PE");
    phenomena_label_.push_back(tr("Ice Pellets"));
    phenomena_metar_.push_back("GR");
    phenomena_label_.push_back(tr("Hail"));
    phenomena_metar_.push_back("GS");
    phenomena_label_.push_back(tr("Small Hail"));
    phenomena_metar_.push_back("BR");
    phenomena_label_.push_back(tr("Mist"));
    phenomena_metar_.push_back("FG");
    phenomena_label_.push_back(tr("Fog"));
    phenomena_metar_.push_back("FU");
    phenomena_label_.push_back(tr("Smoke"));
    phenomena_metar_.push_back("VA");
    phenomena_label_.push_back(tr("Volcanic Ash"));
    phenomena_metar_.push_back("DU");
    phenomena_label_.push_back(tr("Widespread Dust"));
    phenomena_metar_.push_back("SA");
    phenomena_label_.push_back(tr("Sand"));
    phenomena_metar_.push_back("HZ");
    phenomena_label_.push_back(tr("Haze"));
    phenomena_metar_.push_back("PO");
    phenomena_label_.push_back(tr("Sand Whirls"));
    phenomena_metar_.push_back("SQ");
    phenomena_label_.push_back(tr("Squall"));
    phenomena_metar_.push_back("FC");
    phenomena_label_.push_back(tr("Funnel Cloud"));
    phenomena_metar_.push_back("SS");
    phenomena_label_.push_back(tr("Sandstorm"));
    phenomena_metar_.push_back("DS");
    phenomena_label_.push_back(tr("Duststorm"));

    reset();
  }

  void meteo_data::set_metar(QString metar) {
    metar_=metar;
  }

  void meteo_data::set_relative_gmt(int value) {
    relative_gmt_=value;
  }

  void meteo_data::reset() {
    token_.clear();
    // station_= QString();
    day_=0;
    hour_=0;
    min_=0;
  
    wind_degree_=NOSIG;
    wind_speed_=NOSIG;
    wind_gusting_speed_=NOSIG;
    wind_degree_variations_min_=NOSIG;
    wind_degree_variations_max_=NOSIG;
  
    visibility_=NOSIG;
    temperature_=NOSIG;
    dew_point_=NOSIG;
    pressure_=NOSIG;

    sky_conditions_list_.clear();
    phenomena_list_.clear(); 
  }

  int meteo_data::search(QString str) const {
    int result=-1;
    for(int i=0; i<token_.size() && result==-1; ++i) {
      if(token_[i].contains(str)) {
	result=i;
      }
    }
    return result;  
  }

  int meteo_data::search(const QRegExp &rx) const {
    int result=-1;
    for(int i=0; i<token_.size() && result==-1; ++i) {
      if(token_[i].contains(rx)) {
	result=i;
      }
    }
    return result;  
  }


  void meteo_data::decode_phenomena() {
    for(int i=0; i<phenomena_metar_.size(); ++i) {
      int istr = search(phenomena_metar_[i]);
      if(istr>=0) {
	phenomena_list_.push_back(i);
	if(i==1 || i==2) {
	  icon_=QPixmap(ra_s_xpm);
	} else if(i==3 || i==4) {
	  icon_=QPixmap(sn_s_xpm);	
	} else if(i==7 || i==8) {
	  icon_=QPixmap(gr_s_xpm);	
	}		
      }
    }
  }

  int meteo_data::worst_sky_condition() {
    int result=MISSING;
    for(int i=0; i<int(sky_conditions_list_.size()); ++i) {
      if(sky_conditions_list_[i]>result) {
	result=sky_conditions_list_[i];
      }
    }
    return result;
  }

  void meteo_data::decode_sky_conditions() {
    int istr = search("CAVOK");
    if(istr>=0) {
      sky_conditions_list_.push_back(CLR);
      icon_=QPixmap(clr_s_xpm);
    }
    for(int i=0; i<sky_conditions_metar_.size(); ++i) {
      istr = search(sky_conditions_metar_[i]);
      if(istr>=0) {
	sky_conditions_list_.push_back(i);
	switch(i) {
        case 1: {
	  icon_=QPixmap(clr_s_xpm);	
	} break;
        case 2: {
	  icon_=QPixmap(few_s_xpm);	
	} break;
        case 3: {
	  icon_=QPixmap(sct_s_xpm);	
	} break;
        case 4: {
	  icon_=QPixmap(bkn_s_xpm);	
	} break;
        case 5: {
	  icon_=QPixmap(ovc_s_xpm);	
	} break;
	}
      }
    }
  }

  void meteo_data::decode() {
    reset();
    token_=metar_.split(" ");
    if(token_.size()<=2) return;
  
    int istr=search(QRegExp("^\\D\\D\\D\\D"));
    if(istr>=0) {
      station_=token_[istr];
    }

    istr=search(QRegExp("Z$"));
    if(istr>=0) {
      QString str=token_[istr];
      day_=str.mid(0,2).toInt();
      hour_=str.mid(2,2).toInt();
      min_=str.mid(4,2).toInt();
      timestamp_=day_*30*24*60+hour_*60+min_;
    }

    istr=search(QRegExp("KT$"));
    if(istr>=0) {
      QString str=token_[istr];
      if(str.contains("VRB")) {
	wind_degree_=VARIABLE;
      }
      else {
	wind_degree_=str.mid(0,3).toFloat();
      }
      int indexKT=str.indexOf("KT", 0); 
      if(str.contains("G")) {
	int indexG=str.indexOf("G", 0); 
	wind_speed_=str.mid(3,indexG-3).toFloat();
	wind_gusting_speed_=str.mid(indexG+1,indexKT-indexG-1).toFloat();
      } else {
	wind_speed_=str.mid(3,indexKT-3).toFloat();
      }
    }
    istr=search(QRegExp("\\d+V\\d+"));
    if(istr>=0) {
      QString str=token_[istr];
      wind_degree_variations_min_=str.mid(0,3).toFloat();
      wind_degree_variations_max_=str.mid(4,3).toFloat();
    }
    istr=search(QRegExp("^\\d+$"));
    if(istr>=0) {
      QString str=token_[istr];
      visibility_=str.toFloat();
    }
    istr=search("CAVOK");
    if(istr>=0) {
      visibility_=VISIBILITY_MAX;
    }

    decode_sky_conditions();
    decode_phenomena();

    istr=search(QRegExp("^M*\\d+/M*\\d+$"));
    if(istr>=0) {
      QString str=token_[istr];
      QRegExp rx = QRegExp("^M*(\\d+)/M*(\\d+)$");
      int pos = rx.indexIn(str);
      if(pos>=0) {
	temperature_=rx.cap(1).toFloat();
	if(str[0]=='M'){
	  temperature_=-temperature_;
	}
	dew_point_=rx.cap(2).toFloat();
	if(str[3]=='M' || str[4]=='M'){
	  dew_point_=-dew_point_;
	}
      }
    }

    istr=search(QRegExp("^Q"));
    if(istr>=0) {
      QString str=token_[istr];
      pressure_=str.mid(1,4).toFloat();
    }  
  }

  QString meteo_data::get_wind_name(float degree) const {
    QString result;
    if(degree!=NOSIG) {
      int index=int(degree/22.5f)%wind_name_size();
      result=wind_name(index*2+1);
    }
    return result;
  }
 
  QString meteo_data::get_wind_direction(float degree) const {
    QString result;
    if(degree!=NOSIG) {
      int index=int(degree/22.5f)%wind_name_size();
      result=wind_name(index*2);
    }
    return result;
  }

  QString meteo_data::get_wind_status(float speed) const {
    QString result;
    for(int i=0; i<wind_status_size(); i++) {
      float speed_min=wind_status(i*3+1).toFloat();
      float speed_max=wind_status(i*3+2).toFloat();
      if(speed>=speed_min && speed<=speed_max) {
	result=wind_status(i*3);
	break;
      }
    }
    return result;
  }

  int meteo_data::get_wind_beaufort(float speed) const {
    int result=-1;
    for(int i=0; i<wind_status_size(); i++) {
      float speed_min=wind_status(i*3+1).toFloat();
      float speed_max=wind_status(i*3+2).toFloat();
      if(speed>=speed_min && speed<=speed_max) {
	result=i;
	break;
      }
    }
    return result;
  }
    
  // Queries
  const QString& meteo_data::metar() const {
    return metar_;
  }

  int meteo_data::timestamp() const {
    return timestamp_;
  }

  const QString& meteo_data::station() const {
    return station_;
  }

  int meteo_data::day() const {
    return day_;
  }

  int meteo_data::hour() const {
    return hour_;
  }
  int meteo_data::min() const {
    return min_;
  }

  int meteo_data::relative_gmt() const {
    return relative_gmt_;
  }

  float meteo_data::wind_degree() const {
    return wind_degree_;
  }

  float meteo_data::wind_speed() const {
    return wind_speed_;
  }

  float meteo_data::wind_gusting_speed() const {
    return wind_gusting_speed_;
  }

  float meteo_data::wind_degree_variations_min() const {
    return wind_degree_variations_min_;
  }
  float meteo_data::wind_degree_variations_max() const {
    return wind_degree_variations_max_;
  }
  
  float meteo_data::visibility() const {
    return visibility_;
  }

  float meteo_data::temperature() const {
    return temperature_;
  }

  float meteo_data::dew_point() const {
    return dew_point_;
  }

  float meteo_data::perceived_temperature() const {
    float t = temperature();
    float wct_value = wct(t,wind_speed_kn_to_ms(wind_speed()));
    float humidex_value = humidex(t,dew_point());
    return (humidex_value+wct_value)/2.0f;
  }

  float meteo_data::r_humidity() const {
    float result = NOSIG;
    if( (dew_point() != NOSIG) && (temperature() != NOSIG) ) {
      result=vapor_pressure(dew_point_)/vapor_pressure(temperature_)*100.0f;
    }
    return result;
  }

  float meteo_data::pressure() const {
    return pressure_;
  }

  const std::vector<int>& meteo_data::sky_conditions_list() const {
    return sky_conditions_list_;
  }

  const std::vector<int>& meteo_data::phenomena_list() const {
    return phenomena_list_;
  }

  bool meteo_data::good_sky_conditions_index(int id) const {
    return id>=0 && id<sky_conditions_label_.size();
  }

  const QString& meteo_data::sky_conditions_label(int id) const {
    assert(good_sky_conditions_index(id));
    return sky_conditions_label_[id];
  }

  bool meteo_data::good_phenomena_index(int id) const {
    return id>=0 && id<phenomena_label_.size();
  }

  const QString& meteo_data::phenomena_label(int id) const {
    assert(good_phenomena_index(id));
    return phenomena_label_[id];
  }

  int  meteo_data::wind_status_size() const {
    return WIND_STATUS_SIZE;
  }  

  QString meteo_data::wind_status(int id) const {
    const char *wind_status[] = {
      QT_TRANSLATE_NOOP("ratman::meteo_data", "Calm"), "0", "1", 
      QT_TRANSLATE_NOOP("ratman::meteo_data","Light Air"), "2", "3",
      QT_TRANSLATE_NOOP("ratman::meteo_data","Light Breeze"), "4", "6",
      QT_TRANSLATE_NOOP("ratman::meteo_data","Gentle Breeze"), "7", "10",
      QT_TRANSLATE_NOOP("ratman::meteo_data","Moderate Breeze"), "11", "16",
      QT_TRANSLATE_NOOP("ratman::meteo_data","Fresh Breeze"), "17", "21",
      QT_TRANSLATE_NOOP("ratman::meteo_data","Strong Breeze"), "22", "27",
      QT_TRANSLATE_NOOP("ratman::meteo_data","Near Gale"), "28", "33",
      QT_TRANSLATE_NOOP("ratman::meteo_data","Gale"), "34", "40",
      QT_TRANSLATE_NOOP("ratman::meteo_data","Severe Gale"), "41", "47",
      QT_TRANSLATE_NOOP("ratman::meteo_data","Storm"), "48", "55",
      QT_TRANSLATE_NOOP("ratman::meteo_data","Violent Storm"), "56", "63",
      QT_TRANSLATE_NOOP("ratman::meteo_data","Hurricane"), "64", "1000",
    };
    QString result;
    if(id>=0 && id<wind_status_size()*3) {
      result = tr(wind_status[id]);
    }
    return result;
  }

  int  meteo_data::wind_name_size() const {
    return WIND_NAME_SIZE;
  }  

  QString meteo_data::wind_name(int id) const {
    const char *wind_name[] = {
      "N", "Tramontana",
      "NNE", "Greco Tramontana",
      "NE", "Grecale",
      "ENE", "Greco Levante",
      "E", "Levante",
      "ESE", "Levante Scirocco",
      "SE", "Scirocco",
      "SSE", "Scirocco Mezzogiorno",
      "S", "Mezzogiorno",
      "SSW", "Libeccio Mezzogiorno",
      "SW", "Libeccio",
      "WSW", "Libeccio Ponente",
      "W", "Ponente",
      "WNW", "Maestro Ponente",
      "NW", "Maestrale",
      "NNW", "Maestro Tramontana",
    };
    QString result;
    if(id>=0 && id<wind_name_size()*2) {
      result = wind_name[id];
    }
    return result;
  }

  QPixmap meteo_data::meteo_icon_pixmap() const {
    QPixmap result(bkg_xpm);
    if(temperature()!=NOSIG) {
      QFont sansFont("Helvetica", 8);
      QPainter painter(&result);
      painter.setFont(sansFont);
      painter.drawPixmap(0,19,icon_);
      QString temp;
      temp.sprintf("%2.0f%c",temperature(),Qt::Key_degree);
      QRect rectangle(37, 26, 63, 44);
      painter.drawText(rectangle,temp);
    } else {
      result=QPixmap(nodata_xpm);
    }
    return result;
  }

  QImage meteo_data::meteo_icon_image() const {
    return meteo_icon_pixmap().toImage();
  }

}
