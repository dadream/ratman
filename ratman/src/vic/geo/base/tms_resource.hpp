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
#ifndef VIC_GEO_BASE_TMS_RESOURCE_HPP
#define VIC_GEO_BASE_TMS_RESOURCE_HPP

#include <vic/xml/document.hpp>
#include <string>

namespace vic {

  namespace geo {

    namespace base {

      class tms_resource {
      protected:
	std::string last_error_message_;
	bool        last_operation_success_;

	static std::string to_lower(const std::string& s);
	static bool lc_equal(const std::string& s1, const std::string& s2);

	inline virtual void fail(const std::string& msg) {
	  last_operation_success_=false;
	  last_error_message_=msg;
	}
	
	virtual void xml_parse_string(const std::string& xml_description, const std::string& root_tag);
	virtual void xml_parse(vic::xml::node_iterator ptr) = 0;

      public:
	tms_resource() {
	  reset_error();
	}

	virtual ~tms_resource() {
	}
	
	inline void reset_error() {
	  last_operation_success_=true;
	  last_error_message_=std::string("");
	}

	inline bool last_operation_success() const {
	  return last_operation_success_;
	}

	inline const std::string& last_error_message() const {
	  return last_error_message_;
	}

	virtual std::string description() const = 0;
 


      };

    }
  }
}

#endif
