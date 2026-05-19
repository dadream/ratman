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
#include "config.hpp"
#include <cstdlib>
#include <QLocale>
#include <QFile>
#include <QDomDocument>
#include <QSettings>
#include <iostream>
#include <vic/curlstream/curlstream.hpp>

namespace ratman {

  // Utility
  
  static bool file_ok(const QString& fname) {
    bool result=false;
    vic::icurlstream ifile;
    ifile.open(fname.toStdString().c_str());
    if (ifile.good()) {
      std::string line;
      std::getline(ifile,line);
      if (!ifile.fail()) {
	result=true;
      }      
      ifile.close();
    }
    return result;
  }

  static QString tagtext(const QDomElement& node,
			 const QString& tag,
			 bool* ok) {

    QString result="";
    *ok=true;
    QDomNodeList base_lst = node.elementsByTagName(tag);
    if (base_lst.size() < 1) {
      *ok=false;
    } else {
      result=base_lst.item(0).toElement().text();
    }
    return result;
  }

  config::config() {
    error_=false;
    error_msg_="";
    base_url_="";
    language_=DEFAULT_LANGUAGE;
    load();
  }

  QString config::home_dir() const {
    /**#ifdef _WIN32
    return "."; 
    #endif**/
    return QString(std::getenv("GEORATMAN_DIR"));
  }

  QString config::language() const {
    return language_;
  }

  QString config::language_ext() const {
    return language().toLower().left(2);
  }

  QString config::translations_dir() const {
    QString result = home_dir();
    if (!result.isEmpty()) {
      result = result + DIRECTORY_SEPARATOR + "translations";
    }
    return result;
  }

  QString config::translations_file() const {
    return "ratman_" + language();    
  }

  QString config::translations_qt_file() const {
    return "qt_" + language();    
  }

  QString config::translations_file_path(const QString& fname) const {
    QString result = translations_dir();
    if (!result.isEmpty()) {
      result = result + DIRECTORY_SEPARATOR + fname + ".qm";
    }
    return result;  
  }
  
  QString config::qt_plugins_dir() const {
    QString result = home_dir();
    if (!result.isEmpty()) {
      result = result + DIRECTORY_SEPARATOR + "plugins";
    }
    return result;  
  }

  QString config::config_fname() const {
    QString result = home_dir();
    if (!result.isEmpty()) {
      result = result + DIRECTORY_SEPARATOR + "config.xml";
    }
    return result;  
  }

  config::this_t* config::instance() {
    static this_t* result = 0;
    if (!result) {
      result = new this_t;
    }
    return result;
  }
  
  void config::load() {
    if (config_fname().isEmpty()) {
      error_=true;
      error_msg_=QObject::tr("Error: bad configuration. Please provide GEORATMAN_DIR");
      return;
    }
    QFile file(config_fname()); 
    if (!file.open(QIODevice::ReadOnly)) {
      error_=true;
      error_msg_=QObject::tr("Error: config.xml not exist");
      return;
    }
    QDomDocument doc("config");	
    int err_line;
    int err_col;
    QString err_msg;
    error_=!doc.setContent(&file,&err_msg,&err_line,&err_col);  
    if (error_) {
      error_msg_=QObject::tr("Parse error: config.xml at line %1, column %2\n"
			     "%3").arg(err_line).arg(err_col).arg(err_msg);
      file.close();
      return;
    }
    file.close();
    QDomElement root = doc.documentElement();

    bool ok=true;

    // version
    version_url_=tagtext(root, "version", &ok);
    if (!ok) {
      error_=true;
      error_msg_=QObject::tr("Error: missing 'version' in config.xml");
    } else {
      // base_url
      base_url_=tagtext(root, "home", &ok);
      if (!ok) {
	error_=true;
	error_msg_=QObject::tr("Error: missing 'home' in config.xml");
      }
    }

    // Set Language

    QString temp=QLocale::system().name();
    if (temp.toLower()!="c") language_=temp;

    // Check if language support is available
    if (language_ != DEFAULT_LANGUAGE) {
      if ( !file_ok(translations_file_path(translations_file())) || 
	   !file_ok(translations_file_path(translations_qt_file())) ||
	   !file_ok(index_file_url()) ) {
	language_=DEFAULT_LANGUAGE;
      }
    }    
  }

  const QString&config:: version_url() const {
    return version_url_;
  }

  const QString&config:: base_url() const {
    return base_url_;
  }

  QString config::index_dir_url() const {
    QString result = base_url();
    if (!result.isEmpty()) {
      result = result + DIRECTORY_SEPARATOR + language_ext();
    }
    return result;
    
  }

  QString config::index_file_url() const {
    QString result = index_dir_url();
    if (!result.isEmpty()) {
      
      result = result + DIRECTORY_SEPARATOR + "index.xml";
    }
    return result;
    
  }
  

  QString config::about_file_url() const {
    QString result = index_dir_url();
    if (!result.isEmpty()) {
      
      result = result + DIRECTORY_SEPARATOR + "about.html";
    }
    return result;
    
  }

   bool config::exists_property(const QString& name){
  	QSettings settings( "RAS", "Italia 3D");
		return settings.contains(name);
	} 

  bool config::get_property(const QString& name) {
	  QSettings settings( "RAS", "Italia 3D");  
    return settings.value(name, false).toBool();
  } 

  void config::set_property(const QString& name, bool value) {
    QSettings settings( "RAS", "Italia 3D");  
    settings.setValue(name, value);
  } 

} //ratman
