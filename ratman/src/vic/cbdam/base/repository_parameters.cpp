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
#include <vic/cbdam/base/repository_parameters.hpp>
#include <vic/curlstream/curlstream.hpp>
#include <sl/utility.hpp>
#include <fstream>

namespace cbdam {

  repository_parameters::repository_parameters() {
    patch_dim_ = 0;
    height_scale_factor_ = 1.0;
    last_operation_success_ = true;
    geo_xform_ = 0;
    srs_="EPSG:4326";
    about_="CBDAM Elevation Layer";
  }

  repository_parameters& repository_parameters::operator=(const repository_parameters& rhs) {
    patch_dim_ = rhs.patch_dim_;
    height_scale_factor_ = rhs.height_scale_factor_;
    last_operation_success_ = rhs.last_operation_success_;
    if (rhs.geo_xform_) {
      geo_xform_ = rhs.geo_xform_->clone();
    }

    return *this;
  }

  repository_parameters::repository_parameters(const repository_parameters& rhs) {
    *this = rhs;
  }

  repository_parameters::repository_parameters(uint32_t patch_dim, 
					       double height_scale_factor, 
					       const coordinate_transform* geo_xform,
					       const std::string& srs,
					       const std::string& about) :
    patch_dim_(patch_dim), height_scale_factor_(height_scale_factor), geo_xform_(0), srs_(srs), about_(about) {
    set_coordinate_transform(geo_xform);
      
    last_operation_success_ = true;
    
  }

  repository_parameters::~repository_parameters() {
    if (geo_xform_ != 0) {
      delete geo_xform_;
    }
  }


  void repository_parameters::set_coordinate_transform(const coordinate_transform* geo_xform) {
    if (geo_xform_) {
      delete geo_xform_;
    }
    geo_xform_ = geo_xform->clone();
  }

  void repository_parameters::read_fail(vic::xml::node_iterator ptr,std::string space) {
    std::cerr << space << "   " << ptr.error_msg() << std::endl;
    ptr.clear_error();
    last_operation_success_ = false;
  }
  
  void repository_parameters::traverse_cbdam_xml(vic::xml::node_iterator ptr,std::string space, bool print) {
    if (!ptr.is_null() && last_operation_success_) {
      if (ptr.is_element_node()) {
        if (print) std::cerr << space << "<" << ptr.tag() << ">" << std::endl;
      
        if(ptr.tag() == "info") {
          int pd;
	  read_int_in(pd, "patch_dim", ptr, space, print);
	  read_double_in(height_scale_factor_, "height_scale_factor", ptr, space, print);
	  read_string_in(srs_, "srs", ptr, space, print);
	  read_string_in(about_, "about", ptr, space, print);
	  patch_dim_ = pd;
	} else if (ptr.tag() == "coordinate_transform") {
	  read_coordinate_transform(ptr, space, print);
	}

	std::string child_space = "  " + space;	
	for(vic::xml::node_iterator child_it=ptr.down();
	    !child_it.is_null();
	    child_it = child_it.next()) {
	  traverse_cbdam_xml(child_it, child_space, print);
	}
	if (print) std::cerr << space << "</" << ptr.tag() << ">" << std::endl;
      }
    }
  }

  void repository_parameters::read_from_file(const char* file_name, bool print) {
#if 0
    SL_TRACE_OUT(-1) << file_name << "FIXME REPLACED CURLISTREAM WITH ISTREAM FOR PREPROCESSING " << std::endl;
    std::ifstream ics(file_name, std::ios::in | std::ios::binary);
#else
    vic::icurlstream ics(file_name, std::ios::in | std::ios::binary);
#endif
    last_operation_success_ = ics.rdbuf()->is_open();
    if (!last_operation_success_) {
      std::cerr << "unable to open " << file_name << " for reading parameters" << std::endl;
      return;
    }
    vic::xml::document doc;
    doc.parse(ics);
    
    if(!doc.error()) {
      vic::xml::node_iterator ptr=doc.root();
      if (doc.root().tag() != "cbdam") {
	std::cerr << "missing cbdam root:not a cbdam xml file" << std::endl;
	return;
      } else {
	traverse_cbdam_xml(doc.root(),"", print);
      }
    } else {
      std::cerr << doc.error_msg() << std::endl;
      return;
    }
    SL_TRACE_OUT(-1) << "done" << std::endl;
  }
  
  void repository_parameters::write_to_file(const char* file_name, bool print) const {
    std::ofstream ocs(file_name, std::ios::out | std::ios::binary);
    last_operation_success_ = ocs.rdbuf()->is_open();
    if (!last_operation_success_) {
      std::cerr << "unable to open " << file_name << " for writing parameters" << std::endl;
      return;
    }
    
    ocs << "<cbdam>" << std::endl;
    ocs << "   <info" << std::endl;
    ocs << "     patch_dim=\"" << patch_dim_ << "\"" << std::endl;
    ocs << "     height_scale_factor=\"" << height_scale_factor_ << "\"" << std::endl;
    ocs << "     srs=\"" << srs_ << "\"" << std::endl;
    ocs << "     about=\"" << about_ << "\"" << std::endl;
    ocs << "   />" << std::endl;
    ocs << "   <coordinate_transform" << std::endl;
    if (geo_xform_->is_planar()) {
      coordinate_transform::aabox_t box = geo_xform_->bounding_rectangle();
      ocs << "     type=\"planar\"" << std::endl;
      ocs << "     u0=\"" << box[0][0] << "\"" << std::endl;
      ocs << "     v0=\"" << box[0][1] << "\"" << std::endl;
      ocs << "     u1=\"" << box[1][0] << "\"" << std::endl;
      ocs << "     v1=\"" << box[1][1] << "\"" << std::endl;
    } else if (dynamic_cast<const cylindrical_coordinate_transform*>(geo_xform_) != 0) {
      double radius = dynamic_cast<const cylindrical_coordinate_transform*>(geo_xform_)->radius();    
      ocs << "     type=\"cylindrical\"" << std::endl;
      ocs << "     radius=\"" << radius << "\"" << std::endl;
    } else if (dynamic_cast<const spherical_coordinate_transform*>(geo_xform_) != 0) {
      double radius = dynamic_cast<const spherical_coordinate_transform*>(geo_xform_)->radius();    
      ocs << "     type=\"spherical\"" << std::endl;
      ocs << "     radius=\"" << radius << "\"" << std::endl;
    } else {
      ocs << "     type=\"unknown\"" << std::endl;
      std::cerr << "unknown coordinate transform type" << std::endl;
    }
    ocs << "   />" << std::endl;
    ocs << "</cbdam>" << std::endl;

    if (print) {
      print_parameters();
    }
  }
    
  void repository_parameters::print_parameters() const {
    std::cerr << "    patch dim           = " << patch_dim_ << std::endl;
    std::cerr << "    root count          = " << geo_xform_->root_count() << std::endl;
    std::cerr << "    height scale        = " << height_scale_factor_ << std::endl;
    std::cerr << "    srs                 = " << srs_ << std::endl;
    std::cerr << "    about               = " << about_ << std::endl;
    if (geo_xform_->is_planar()) {
      planar_coordinate_transform::aabox_t box = dynamic_cast<const planar_coordinate_transform*>(geo_xform_)->bounding_rectangle();
      std::cerr << "    projection type     = planar" << std::endl;
      std::cerr << "    u0                  = " << box[0][0] << std::endl;
      std::cerr << "    v0                  = " << box[0][1] << std::endl;
      std::cerr << "    u1                  = " << box[1][0] << std::endl;
      std::cerr << "    v1                  = " << box[1][1] << std::endl;
    } else {
      double radius = dynamic_cast<const spherical_coordinate_transform*>(geo_xform_)->radius();    
      std::cerr << "    projection type     = spherical" << std::endl;
      std::cerr << "    radius              = " << radius << std::endl;
    }
  } 

  void repository_parameters::read_coordinate_transform(vic::xml::node_iterator ptr,std::string space, bool print) {
    std::string xform_type;
    read_string_in(xform_type, "type", ptr, space, print);

    if (geo_xform_ != 0) delete geo_xform_;
    
    if (last_operation_success()) {
      if (xform_type == "planar") {
	double u0, v0, u1, v1;
	read_double_in(u0, "u0", ptr, space, print);
	read_double_in(v0, "v0", ptr, space, print);
	read_double_in(u1, "u1", ptr, space, print);
	read_double_in(v1, "v1", ptr, space, print);

	if (last_operation_success()) {
	  planar_coordinate_transform::aabox_t box(point2d_t(u0, v0),
						   point2d_t(u1, v1));
	  planar_coordinate_transform* xform = new planar_coordinate_transform(box);
	  geo_xform_ = xform;
	}
      } else if (xform_type == "cylindrical") {
	cylindrical_coordinate_transform* xform = new cylindrical_coordinate_transform;
	double radius;
	read_double_in(radius, "radius", ptr, space, print);
      
	if (last_operation_success()) {
	  xform->set_radius(radius);
	  geo_xform_ = xform;
	  //	std::cerr << "cylindrical transform with radius " << xform->radius() << std::endl;
	}
      } else if (xform_type == "spherical") {
	spherical_coordinate_transform* xform = new spherical_coordinate_transform;
	double radius;
	read_double_in(radius, "radius", ptr, space, print);
	
	if (last_operation_success()) {
	  xform->set_radius(radius);
	  geo_xform_ = xform;
	}
      } else {
	std::cerr << space << "   ERROR: unknown coordinate transform type " << xform_type << std::endl;
	last_operation_success_ = false;
      }
    } else {
      std::cerr << space << "   ERROR: unable to read bounding rectangle " << xform_type << std::endl;     
    }
  }

  void repository_parameters::read_double_in(double& x, 
					     const char* tag, 
					     vic::xml::node_iterator ptr,
					     std::string space, 
					     bool print) {
    if (ptr.has_attribute(tag)) {
      x = sl::from_string<double>(ptr.attribute(tag));
      if (!ptr.error()) {
	if (print) std::cerr << space << "   " << tag << " = " << x << std::endl;
      } else {
	read_fail(ptr, space);
      }
    } else {
      std::cerr << space << "   ERROR: missing attribute " << tag << std::endl;
      last_operation_success_ = false;
    }
  }

  void repository_parameters::read_int_in(int& x, 
					  const char* tag, 
					  vic::xml::node_iterator ptr,
					  std::string space, 
					  bool print) {
    if (ptr.has_attribute(tag)) {
      x = sl::from_string<int>(ptr.attribute(tag));
      if (!ptr.error()) {
	if (print) std::cerr << space << "   " << tag << " = " << x << std::endl;
      } else {
	read_fail(ptr, space);
      }
    } else {
      std::cerr << space << "   ERROR: missing attribute " << tag << std::endl;
      last_operation_success_ = false;
    }
  }

  void repository_parameters::read_string_in(std::string& x, 
					     const char* tag, 
					     vic::xml::node_iterator ptr,
					     std::string space, 
					     bool print) {
    if (ptr.has_attribute(tag)) {
      x = ptr.attribute(tag);
      if (!ptr.error()) {
	if (print) std::cerr << space << "   " << tag << " = " << x << std::endl;
      } else {
	read_fail(ptr, space);
      }
    } else {
      std::cerr << space << "   ERROR: missing attribute " << tag << std::endl;
      last_operation_success_ = false;
    }
  }
}
