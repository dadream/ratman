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
#ifndef METEO_DATA_HPP
#define METEO_DATA_HPP

#include <QObject>
#include <QString>
#include <QStringList>
#include <QPixmap>
#include <QImage>
#include <vector>

//DEFAULT VALUES

namespace ratman {
  static const float NOSIG=-9999;
  static const float VARIABLE=-7777;
  static const float VISIBILITY_MAX=9999;
  static const int MISSING=0;
  static const int CLR=1;
  
  /**
   * Decode metar data, provided by terrain_tile_meteo
   */
  class meteo_data : public QObject{

    Q_OBJECT

  protected:
    QString metar_;
    QStringList token_;
    int timestamp_;
    QString station_name_;  
    QString station_;  
    int day_;
    int hour_;
    int min_;
    int relative_gmt_;
    
    float wind_degree_;
    float wind_speed_;
    float wind_gusting_speed_;
    float wind_degree_variations_min_;
    float wind_degree_variations_max_;
    
    float visibility_;
    float temperature_;
    float dew_point_;
    float r_humidity_;
    float pressure_;
    
    QStringList sky_conditions_label_;
    QStringList sky_conditions_metar_;
    QStringList phenomena_label_;
    QStringList phenomena_metar_;
    
    std::vector<int> sky_conditions_list_;
    std::vector<int> phenomena_list_;
    
    QPixmap icon_;
    
  public:
    meteo_data(const QString& station_name, QObject *parent = 0);

    const QString& station_name() const {
      return station_name_;
    }

    const QString& metar() const;
    void set_metar(QString metar);
    
    int timestamp() const;
    const QString& station() const;
    int day() const;
    int hour() const;
    int min() const;
    int relative_gmt() const;
    void set_relative_gmt(int value);
    
    float wind_degree() const;
    float wind_speed() const;
    float wind_gusting_speed() const;
    float wind_degree_variations_min() const;
    float wind_degree_variations_max() const;
    
    float visibility() const;
    float temperature() const;
    float dew_point() const;
    float perceived_temperature() const;
    float r_humidity() const;
    float pressure() const;
    
    const std::vector<int>& sky_conditions_list() const;
    int worst_sky_condition();
    
    const std::vector<int>& phenomena_list() const;
    
    void reset();
    void decode();
    
    int day_local() const {
      if(hour_+relative_gmt_>=24) {
	return day_+1;
      } else {
	return day_;
      }
    }
    
    int hour_local() const {
      if(hour_+relative_gmt_>=24) {
	return (hour_+relative_gmt_)%24;
      } else {
	return (hour_+relative_gmt_);
      }
    }
    
    QString get_wind_name(float degree) const;
    QString get_wind_direction(float degree) const;
    QString get_wind_status(float speed) const;
    int get_wind_beaufort(float speed) const;
    
    bool good_sky_conditions_index(int id) const;
    const QString& sky_conditions_label(int id) const;
    
    bool good_phenomena_index(int id) const;
    const QString& phenomena_label(int id) const;
    
    int wind_status_size() const;
    QString wind_status(int id) const;
    
    int  wind_name_size() const;
    QString wind_name(int id) const;
    
    QPixmap meteo_icon_pixmap() const;
    QImage meteo_icon_image() const;
   
  protected:
    void decode_phenomena();
    void decode_sky_conditions();
    int search(const QRegExp &rx) const;
    int search(QString str) const;
  };
}
#endif //METEO_DATA_HPP
