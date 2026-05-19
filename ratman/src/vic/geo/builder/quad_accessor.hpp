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
#ifndef QUAD_ACCESSOR_HPP
#define QUAD_ACCESSOR_HPP

#include <string>
#include <vector>

class GDALDriver;
class GDALDataset;

namespace vic {
  namespace geo {
    class quad_accessor {
    public:

    protected:
      mutable bool last_operation_success_;
      mutable std::string last_error_message_;
      GDALDriver *output_driver_;
      char **output_driver_opts_;
      std::string root_dir_;
      std::string output_format_;
      std::string file_extension_;
      std::size_t quadtree_root_count_[2];
      
    public:
      quad_accessor();
      virtual ~quad_accessor();
      
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

      inline const std::string& root_dir() const {
	return root_dir_;
      }

      inline void set_root_dir(const std::string& root_dir) {
	root_dir_=root_dir;
      }

      inline const std::string& output_format() const {
	return output_format_;
      }
 
      void set_output_format(const std::string& format, const std::vector<std::string> &opts = std::vector<std::string>());

      inline std::string file_extension() const {
	return file_extension_;
      }

      inline void set_file_extension(const std::string& file_extension) {
	file_extension_=file_extension;
      }

      virtual std::string quad_filename(int l, int x, int y) const;

      inline void set_quadtree_root_count(std::size_t nx, std::size_t ny) {
	quadtree_root_count_[0] = nx; 
	quadtree_root_count_[1] = ny; 
      }
      
      inline std::size_t quadtree_root_count(std::size_t i) const {
	return quadtree_root_count_[i];
      }

      inline std::size_t level_quad_count(std::size_t level, std::size_t i) const {
	return quadtree_root_count_[i] * (0x01<<level);
      }

      // Read/Write GDAL dataset

      virtual GDALDataset* read(int l, int x, int y);

      virtual GDALDataset* read_to_memory(int l, int x, int y);

      virtual void write(int l, int x, int y, GDALDataset *t, bool path_known_to_exist=false);

    protected:
      void clear_output_driver_opts();

    };

  } // namespace geo
} // namespace vic

#endif
