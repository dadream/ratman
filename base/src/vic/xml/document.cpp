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
#include "document.hpp"

#include "tinyxml.h"

using namespace my_tinyxml;

#include <sstream>

#include <cassert>
#include <cstdlib>
#include <cerrno>
#include <iostream>

namespace vic {
  namespace xml {



    ////////////////////////////////
    // xml_node_ptr

    node_iterator::node_iterator(void* ptr) {
      ptr_=ptr;
      error_=false;
      error_msg_="";
    }

    node_iterator::~node_iterator() {
    }

    // Access functions

    node_iterator node_iterator::next() {
      assert(!is_null());
      return this_t(((TiXmlNode *)ptr_)->NextSibling());
    }

    node_iterator node_iterator::down() {
      assert(!is_null());
      return this_t(((TiXmlNode *)ptr_)->FirstChild());
    }

    node_iterator node_iterator::up() {
      assert(!is_null());
      return this_t(((TiXmlNode *)ptr_)->Parent());
    }

    // Queries
    
    bool node_iterator::is_element_node() const {
      assert(!is_null());
      return ( ((TiXmlNode *)(ptr_))->ToElement() );
    }

    bool node_iterator::is_text_node() const {
      assert(!is_null());
      return ( ((TiXmlNode *)(ptr_))->ToText() );
    }

    bool node_iterator::has_attribute(const std::string& name) const {
      assert(!is_null());
       TiXmlElement* element =  ((TiXmlNode *)(ptr_))->ToElement();
       return ( element && element->Attribute(name.c_str()) != 0 );
    }

    std::string node_iterator::attribute(const std::string& name) const {
      assert(!is_null());
      std::string result;
      TiXmlNode* node = (TiXmlNode *)(ptr_);
      TiXmlElement* element =  node->ToElement();
      if ( element ) {
	const char* attr = element->Attribute(name.c_str());
	if (attr) {
	  result=attr;
	} else {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + " does not exist!";
	  error_=true;
 	}  
      } else {
	error_msg_ = "Error in node " + tag() + ": attribute element" + name + " does not exist!";
	error_=true;
      }
      return result;
    }

    long int node_iterator::attributei(const std::string& name) const {
      const std::string attribute_str = attribute(name);
      const char *input = attribute_str.c_str();
      long int result = 0;
      if (!error()) {
	char *endptr;
	errno = 0;
	result = strtol(input, &endptr, 10);
	if(errno !=0) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + " out of range!";
	  error_=true;
	} else if (endptr == input || *endptr != '\0') {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + " is not a long int!";
	  error_=true;
	}
      }
      return result; 
    }

    double node_iterator::attributed(const std::string& name) const {
      const std::string attribute_str = attribute(name);
      const char *input = attribute_str.c_str();
      double result = 0.0;
      if (!error()) {
	char *endptr;
	errno = 0;
	result = strtod(input, &endptr);
	if(errno !=0) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + " out of range!";
	  error_=true;
	} else if (endptr == input || *endptr != '\0') {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + " is not a double!";
	  error_=true;
	}
      }
      return result; 
    }

    float node_iterator::attributef(const std::string& name) const {
      const std::string attribute_str = attribute(name);
      const char *input = attribute_str.c_str();
      float result = 0.0;
      if (!error()) {
	char *endptr;
	errno = 0;
	result = strtod(input, &endptr);
	if(errno !=0) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + " out of range!";
	  error_=true;
	} else if (endptr == input || *endptr != '\0') {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + " is not a float!";
	  error_=true;
	}
      }
      return result; 
    }

    node_iterator::point3d_t node_iterator::attributep(const std::string& name) const {
      const std::string attribute_str = attribute(name);
      std::istringstream input(attribute_str);
      point3d_t result;
      result.c[0]=0.0;
      result.c[1]=0.0;
      result.c[2]=0.0;
      if (!error()) {
	if(!(input >> result.c[0])) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + ": conversion error\n";
	  error_=true;
	}
	if(!(input >> result.c[1])) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + ": conversion error\n";
	  error_=true;
	}
	if(!(input >> result.c[2])) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + ": conversion error\n";
	  error_=true;
	}
      }
      return result; 
    }

    void node_iterator::attributeiv(const std::string& name, int *v, std::size_t n) const {
      const std::string attribute_str = attribute(name);
      std::istringstream input(attribute_str);
      for (std::size_t i=0; i<n && !error(); ++i) {
	if(!(input >> v[i])) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + ": conversion error\n";
	  error_=true;
	}
      }
    }

    void node_iterator::attributefv(const std::string& name, float *v, std::size_t n) const {
      const std::string attribute_str = attribute(name);
      std::istringstream input(attribute_str);
      for (std::size_t i=0; i<n && !error(); ++i) {
	if(!(input >> v[i])) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + ": conversion error\n";
	  error_=true;
	}
      }
    }

    void node_iterator::attributedv(const std::string& name, double *v, std::size_t n) const {
      const std::string attribute_str = attribute(name);
      std::istringstream input(attribute_str);
      for (std::size_t i=0; i<n && !error(); ++i) {
	if(!(input >> v[i])) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + ": conversion error\n";
	  error_=true;
	}
      }
    }

    std::string node_iterator::attribute(const std::string& name, const std::string& default_string) const {
      assert(!is_null());
      std::string result = default_string;
      TiXmlNode* node = (TiXmlNode *)(ptr_);
      TiXmlElement* element =  node->ToElement();
      if (element) {
	const char* attr = element->Attribute(name.c_str());
	if (attr) {
	  result=attr;
	}
      }
      return result;
    }

    long int node_iterator::attributei(const std::string& name, const std::string& default_string) const {
      //      const char *input = attribute(name,default_string).c_str();
      const std::string attribute_str = attribute(name, default_string);
      const char *input = attribute_str.c_str();
      long int result = 0;
      if (!error()) {
	char *endptr;
	errno = 0;
	result = strtol(input, &endptr, 10);
	if(errno !=0) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + " out of range!";
	  error_=true;
	} else if (endptr == input || *endptr != '\0') {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + " is not a long int!";
	  error_=true;
	}
      }
      return result; 
    }

    double node_iterator::attributed(const std::string& name, const std::string& default_string) const {
      //      const char *input = attribute(name,default_string).c_str();
      const std::string attribute_str = attribute(name, default_string);
      const char *input = attribute_str.c_str();
      double result = 0.0;
      if (!error()) {
	char *endptr;
	errno = 0;
	result = strtod(input, &endptr);
	if(errno !=0) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + " out of range!";
	  error_=true;
	} else if (endptr == input || *endptr != '\0') {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + " is not a double!";
	  error_=true;
	}
      }
      return result; 
    }

    float node_iterator::attributef(const std::string& name, const std::string& default_string) const {
      //      const char *input = attribute(name,default_string).c_str();
      const std::string attribute_str = attribute(name, default_string);
      const char *input = attribute_str.c_str();
      float result = 0.0;
      if (!error()) {
	char *endptr;
	errno = 0;
	result = strtod(input, &endptr);
	if(errno !=0) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + " out of range!";
	  error_=true;
	} else if (endptr == input || *endptr != '\0') {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + " is not a float!";
	  error_=true;
	}
      }
      return result; 
    }

    node_iterator::point3d_t node_iterator::attributep(const std::string& name, const std::string& default_string) const {
      const std::string attribute_str = attribute(name, default_string);
      std::istringstream input(attribute_str);
      point3d_t result;
      result.c[0]=0.0;
      result.c[1]=0.0;
      result.c[2]=0.0;
      if (!error()) {
	if(!(input >> result.c[0])) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + ": conversion error\n";
	}
	if(!(input >> result.c[1])) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + ": conversion error\n";
	}
	if(!(input >> result.c[2])) {
	  error_msg_ = "Error in node " + tag() + ": attribute " + name + ": conversion error\n";
	}
      }
      return result; 
    }

    std::string node_iterator::tag() const {
      assert(!is_null());
      return std::string((const char *)(((TiXmlNode *)(ptr_))->Value()));
    }
 
    std::string node_iterator::text() const {
      assert(!is_null());
      std::string result;
      TiXmlNode* node = (TiXmlNode *)(ptr_);
      TiXmlText* text =  node->ToText();
      if (text) {
	result = text->Value();
      }
      return result;      
    }

    long int node_iterator::texti() const {
      //      const char *input = text().c_str();
      const std::string text_str = text();
      const char *input = text_str.c_str();
      long int result = 0;
      if (!error()) {
	char *endptr;
	errno = 0;
	result = strtol(input, &endptr, 10);
	if(errno !=0) {
	  error_msg_ = "Error in node " + tag() + ": out of range!";
	  error_=true;
	} else if (endptr == input || *endptr != '\0') {
	  error_msg_ = "Error in node " + tag() + ": is not a long int!";
	  error_=true;
	}
      }
      return result; 
    }

    double node_iterator::textd() const {
      //     const char *input = text().c_str();
      const std::string text_str = text();
      const char *input = text_str.c_str();
      double result = 0.0;
      if (!error()) {
	char *endptr;
	errno = 0;
	result = strtod(input, &endptr);
	if(errno !=0) {
	  error_msg_ = "Error in node " + tag() + ": out of range!";
	  error_=true;
	} else if (endptr == input || *endptr != '\0') {
	  error_msg_ = "Error in node " + tag() + ": is not a double!";
	  error_=true;
	}
      }
      return result; 
    }


    ////////////////////////////////
    // Class document
    

    document::document() {
      doc_=0;
      error_msg_="";
      error_=false;
    }

    document::~document() {
      clear();
    }

    void document::clear() {
      if(doc_) {
	TiXmlDocument *xml_doc = (TiXmlDocument *)(doc_);
	delete xml_doc;
      }
      doc_=0;
      error_msg_="";
      error_=false;
    }

    void document::parse(std::istream& reader) {
      clear();
      TiXmlDocument *xml_doc = new TiXmlDocument();
      doc_=xml_doc;
      reader >> *xml_doc;
      if ( xml_doc->Error() ) {
	error_=true;
	std::stringstream s; 
	s <<  "Error in XML file (Row " << xml_doc->ErrorRow() << " ,Col " << xml_doc->ErrorCol() << "): " << xml_doc->ErrorDesc() << std::endl;
	error_msg_ = s.str();
      }
    }
    
    /**
     * This method is used in classes who heritate from document. To mantain compatibility
     * the parse method is called from here. This implementation also calls read_node, that
     * may be implemented in the class who heritate from document.
     */
    void document::read_stream(std::istream& reader) {
      parse(reader);
      assert(!is_null());
      if ( !this->error() ) {
        read_node(this->root());
      } else {
	  std::cerr << "ERROR > document::read_stream :  " << error_msg_ << std::endl;
      }
    }

    void document::read_node(node_iterator /*node*/) {
      /* Nothing to do. Making this method no virtual = 0 allows to instanciate
         this class and this way, mantain backward compatibility. */
	std::cerr << "If you are calling document::read_node(node_iterator) remember to reimplement this method in your document subclass!" << std::endl;
    }

    node_iterator document::root() {
      assert(!is_null());
      return node_iterator( ((TiXmlDocument *)(doc_))->FirstChild() );
    }

    node_iterator document::first_root(const std::string& tag) {
      assert(!is_null());
      return node_iterator( ((TiXmlDocument *)(doc_))->FirstChildElement(tag.c_str()) );
    }

  }
}

