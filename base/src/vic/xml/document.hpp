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
#ifndef VIC_XML_DOCUMENT_HPP
#define VIC_XML_DOCUMENT_HPP

#include <string>
#include <istream>

namespace vic {
  namespace xml {

    class node_iterator {

    public:
      typedef node_iterator this_t;
      typedef struct {
	double c[3];
      } point3d_t;

    protected:
      mutable std::string error_msg_;
      mutable bool error_;
      void* ptr_;
      
    public:

      node_iterator(void* ptr=0);
      ~node_iterator();

      inline void clear_error() {
	error_=false;
	error_msg_="";
      }

      inline bool error() const {
	return error_;
      }
      
      inline const std::string& error_msg() const {
	return error_msg_;
      }
      
      // Access functions
      this_t  next();
      this_t  prev();
      this_t  up();
      this_t  down();
      
      inline bool is_null() const { return ptr_ == 0; }

    public:
      
      bool is_element_node() const;
      bool is_text_node() const;
      
      std::string tag() const;

      std::string text() const;
      long int texti() const;
      double textd() const;

      bool has_attribute(const std::string& name) const;

      std::string attribute(const std::string& name) const;
      long int attributei(const std::string& name) const;
      double attributed(const std::string& name) const;
      float attributef(const std::string& name) const;
      point3d_t attributep(const std::string& name) const;

      void attributeiv(const std::string& name, int *v, std::size_t n) const;
      void attributefv(const std::string& name, float *v, std::size_t n) const;
      void attributedv(const std::string& name, double *v, std::size_t n) const;


      std::string attribute(const std::string& name, const std::string& default_string) const;
      long int attributei(const std::string& name, const std::string& default_string) const;
      double attributed(const std::string& name, const std::string& default_string) const;
      float attributef(const std::string& name, const std::string& default_string) const;
      point3d_t attributep(const std::string& name, const std::string& default_string) const;

    };


    class document {
    protected:
      mutable std::string error_msg_;
      mutable bool error_;
      void *doc_;

    public:

      document();
      virtual ~document();

      void clear();
      
      void parse(std::istream& reader);

      void read_stream(std::istream& reader);

      virtual void read_node(node_iterator node);

      inline void clear_error() {
	error_=false;
	error_msg_="";
      }

      inline bool error() const {
	return error_;
      }
      
      inline void set_error(bool b) {
	error_=b;
      }
      
      inline const std::string& error_msg() const {
	return error_msg_;
      }
      
      inline void set_error_msg(const std::string& msg) {
	error_msg_=msg;
      }
      
      
      // Access functions

      inline bool is_null() const { return doc_ == 0; }

      node_iterator root();

      node_iterator first_root(const std::string& tag);
    };

  }
}

#endif // VIC_XML_DOCUMENT_HPP
