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
#include <vic/geo/builder/quad_accessor.hpp>
#include <vic/geo/builder/geo_utility.hpp>
#include <vic/geo/base/victms_conventions.hpp>
#include <iostream>

// GDAL include
#include <gdal_priv.h>
#include <cpl_string.h>

namespace vic {
  namespace geo {

    quad_accessor::quad_accessor() :
      last_operation_success_(true),
      output_driver_(0),
      output_driver_opts_(0) {
      quadtree_root_count_[0]=0;
      quadtree_root_count_[1]=0;
    }

    quad_accessor::~quad_accessor() {
      clear_output_driver_opts();
    }

    void quad_accessor::clear_output_driver_opts() {
      if(output_driver_opts_) CSLDestroy(output_driver_opts_);
       output_driver_opts_=0;
    }

    void quad_accessor::set_output_format(const std::string& format, const std::vector<std::string> &opts) {
      output_driver_=GetGDALDriverManager()->GetDriverByName(format.c_str());
      if(!output_driver_) {
	last_error_message_=std::string("Unable to set output format");
	last_operation_success_=false;
	return;
      }
      clear_output_driver_opts();
      for (std::vector<std::string>::const_iterator it = opts.begin(); it != opts.end(); ++it) {
	output_driver_opts_ = CSLSetNameValue(output_driver_opts_, 
					      it->substr(0, it->find('=')).c_str(), it->substr(it->find('=') + 1).c_str());
      }
    }

    std::string quad_accessor::quad_filename(int l, int x, int y) const {
      return geo::base::victms_conventions::quad_filename(root_dir(), l, x, y, file_extension());
    }

    void quad_accessor::write(int l, int x, int y, GDALDataset *tile, bool path_known_to_exist) {
      std::string loc = quad_filename(l, x, y);

      if (!path_known_to_exist) {
	// Directory might need creation
	int p = loc.rfind('/');      
	if (p > 0) {
	  // Has directory, try to create path to it
	  if(!geo_utility::mkpath(loc.substr(0, p))) {
	    last_operation_success_=false;
	    last_error_message_=std::string("Unable to create path: ") + loc;
	    return;
	  }
	}
      }

      GDALDataset *datacp = output_driver_->CreateCopy(loc.c_str(), tile, 0, output_driver_opts_, GDALDummyProgress, 0);
      if (datacp) {
	delete datacp;
	datacp=0;
      } else {
	char msg[512];
	snprintf(msg, sizeof(msg), "Unable to write quad %d %d %d", l, x, y);
	last_error_message_=std::string(msg);
	return;
	last_operation_success_=false;
      }
    }

    GDALDataset* quad_accessor::read(int l, int x, int y) {
      std::string loc = quad_filename(l, x, y);
      FILE* f = fopen(loc.c_str(), "r");
      if (!f) return 0; 
      fclose(f);

      GDALDataset *tile=reinterpret_cast<GDALDataset*>(GDALOpen(loc.c_str(), GA_ReadOnly));
      if (!tile) {
	char msg[512];
	snprintf(msg, sizeof(msg), "Unable to read quad %d %d %d", l, x, y);
	last_error_message_=std::string(msg);
	last_operation_success_=false;
      }
      return tile;      
    }

    GDALDataset* quad_accessor::read_to_memory(int l, int x, int y) {
      GDALDataset* result = read(l,x,y);
      if (result) {
	GDALDataset* result_copy = GetGDALDriverManager()->GetDriverByName("MEM")->CreateCopy("MEM",
											      result,
											      0,
											      0,
											      GDALDummyProgress,
											      0);
	delete result;
	result = result_copy;
      }
      return result;
    }
    
  } // namespace geo
} // namespace vic
