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
#ifndef QUAD_PROCESSOR_HPP
#define QUAD_PROCESSOR_HPP

#include <vector>
#include <string>

class GDALDataset;

namespace vic {
  namespace geo {
    class quad_processor {
    public:

    protected:
      mutable bool last_operation_success_;
      mutable std::string last_error_message_;
      int min_data_value_;
      
    public:
      quad_processor();
      virtual ~quad_processor();
      
      inline bool last_operation_success() const {
	return last_operation_success_;
      }
      
      inline const std::string& last_error_message() const {
	return last_error_message_;
      }

      inline void reset_error() {
	last_operation_success_=true;
	last_error_message_=std::string("");
      }

      inline void set_min_data_value(int value) {
	min_data_value_=value;
      }
      
      inline int min_data_value() const {
	return min_data_value_;
      }

      GDALDataset *create_from_template(GDALDataset *sample);
      void coarsen(GDALDataset *out, const std::vector<GDALDataset *> &samples);
      void coarsen(GDALDataset *out, GDALDataset *sample00, GDALDataset *sample01, GDALDataset *sample10, GDALDataset *sample11);
      void combine(GDALDataset *out, const std::vector<GDALDataset *> &samples);
      void combine(GDALDataset *out, GDALDataset *sample0, GDALDataset *sample1);
      bool is_null(GDALDataset *sample) const;

      void compute_null_pixel_stats(GDALDataset *sample, 
				    int* null_pixel_count, 
				    int* non_null_pixel_count) const;

      void color_remap(GDALDataset *out, GDALDataset *in, 
		       int black_out, int black_in, 
		       int white_out, int white_in, 
		       int below_to_black = 0, 
		       int above_to_black = -1);
      void global_remap_nodata_to_black(GDALDataset *inout, 
					int black_out, int black_in, 
					int white_out, int white_in,
					int below_to_black, int above_to_black);

      void print_info(GDALDataset *sample) const;

    };

  } // namespace geo
} // namespace vic

#endif
