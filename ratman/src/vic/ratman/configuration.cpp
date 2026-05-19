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
#include <vic/ratman/configuration.hpp>
#include <cstdlib> 

namespace ratman {

  configuration::configuration() {
  }

  configuration* configuration::instance() {
    // Locking???
    static configuration* result = 0;
    if (!result) {
      result = new configuration;
    }
    return result;
  }

  void configuration::load() {
    mutex_.lock();
    std::cerr << "Configuration load not implemented." << std::endl;
    mutex_.unlock();
  }

  void configuration::save() const {
    mutex_.lock();
    std::cerr << "Configuration save not implemented." << std::endl;
    mutex_.unlock();
  }

  std::string configuration::value(const std::string& key, 
				   const std::string& default_value) const {
    std::string result = default_value;
    mutex_.lock();
    std::map<std::string,std::string>::const_iterator it = map_.find(key);
    if (it != map_.end()) {
      result = it->second;
    }
    mutex_.unlock();
    return result;
  }

  std::ostream& configuration::operator<<(std::ostream& os) const {
    mutex_.lock();
    for (std::map<std::string,std::string>::const_iterator it = map_.begin();
	 it != map_.end();
	 ++it) {
      os << it->first << " = " << '"' << it->second << '"' << std::endl;
    }
    mutex_.unlock();
    return os;
  }

}
