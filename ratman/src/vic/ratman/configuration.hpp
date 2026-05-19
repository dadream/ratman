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
#ifndef VIC_RATMAN_CONFIGURATION_HPP
#define VIC_RATMAN_CONFIGURATION_HPP

#include <iostream>
#include <string>
#include <map>
#include <QMutex>

namespace ratman {

  class configuration {
  public: 
    typedef configuration this_t;
  private: // disable copy
    configuration(const configuration&);
    configuration operator=(const configuration&);
    
  protected:
    mutable QMutex mutex_;
    std::map<std::string,std::string> map_;

    configuration(); // protected

  public:

    static configuration* instance();

  public:
    
    // Call it in main before launching threads accessing config
    void load();

    void save() const;

    std::string value(const std::string& key, const std::string& default_value ="") const;

    std::ostream& operator<<(std::ostream& os) const;
  };
}

#endif
