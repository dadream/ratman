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
#ifndef RATMAN_CONFIG_HPP
#define RATMAN_CONFIG_HPP

#ifdef _WIN32
//#define DIRECTORY_SEPARATOR "\\"
#define DIRECTORY_SEPARATOR "/"
#else
#define DIRECTORY_SEPARATOR "/"
#endif

#define DEFAULT_LANGUAGE "en_EN"

#include <QString>

namespace ratman {

  /**
   * Program basic configuration: home_dir, translations, plugins information. 
   * Reads home_dir from the GEORATMAN_DIR environment variable. 
   * home_dir contains also config.xml, bookmarks.xml files.
   * config.xml contains the url of the configuration file which will 
   * be parsed by xml_config_parser.
   */
  class config {
  public: 
    typedef config this_t;
  protected:
    bool error_;
    QString error_msg_;
    QString version_url_;
    QString base_url_;
    QString language_;

  private: // disable copy
    config(const config&);
    config operator=(const config&);
  protected:
    config(); // protected
    void load();

    // Utility
    QString translations_file_path(const QString& fname) const; 

  public:
    static this_t* instance();

    inline bool error() const {
      return error_;
    }

    inline const QString& error_msg() const {
      return error_msg_;
    }
    
    QString home_dir() const;
    QString language() const;
    QString language_ext() const;

    const QString& version_url() const;

    const QString& base_url() const;
    QString index_dir_url() const;
    QString index_file_url() const;
    QString about_file_url() const;

    QString translations_dir() const;
    QString translations_file() const;
    QString translations_qt_file() const;

    QString qt_plugins_dir() const;
    QString config_fname() const;
    bool exists_property(const QString& name);
  	bool get_property(const QString& name);
    void set_property(const QString& name, bool value);

  };

} // ratman

#endif // RATMAN_CONFIG_HPP
