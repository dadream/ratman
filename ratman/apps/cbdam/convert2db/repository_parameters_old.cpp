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
#include "repository_parameters_old.hpp"

namespace cbdam {

  repository_parameters_old::repository_parameters_old() {
    patch_dim_ = 0;
    is_planar_ = 0;
    is_mono_scale_ = false;
    wavelet_alpha_ = 1.0f;
    length_ = 1.0f;
    root_count_  = 0;
  }

  repository_parameters_old::repository_parameters_old(uint32_t patch_dim, bool is_planar, bool is_mono_scale,
                                               float wavelet_alpha, double length, uint32_t root_count,
                                               const std::vector<diamond_id_t>& roots) :
      patch_dim_(patch_dim), is_planar_(is_planar), is_mono_scale_(is_mono_scale),
      wavelet_alpha_(wavelet_alpha), length_(length), root_count_(root_count),
      roots_(roots) {

  }

  repository_parameters_old::~repository_parameters_old() {

  }

  void repository_parameters_old::read_from_file(FILE* fp, bool print) {
    if (fread(&patch_dim_, sizeof(uint32_t), 1, fp) != 1) {
      std::cerr << "unable to read patch_dim from param file" << std::endl;
      return;
    }

    if (fread(&is_planar_, sizeof(bool), 1, fp) != 1) {
      std::cerr << "unable to read is_planar from param file" << std::endl;
      return;
    }

    if (fread(&is_mono_scale_, sizeof(bool), 1, fp) != 1) {
      std::cerr << "unable to read is_mono_scale from param file" << std::endl;
      return;
    }

    if (fread(&wavelet_alpha_, sizeof(float), 1, fp) != 1) {
      std::cerr << "unable to read wavelet_alpha from param file" << std::endl;
      return;
    }

    if (fread(&length_, sizeof(double), 1, fp) != 1) {
      std::cerr << "unable to read length_scale from param file" << std::endl;
      return;
    }

    if (fread(&root_count_, sizeof(uint32_t), 1, fp) != 1) {
      std::cerr << "unable to read root_count from param file" << std::endl;
      return;
    } else if (root_count_ > 6) {
      std::cerr << "warning: reading " << root_count_ << " roots from param file" << std::endl;
    }

    for(uint32_t i = 0; i < root_count_; ++i) {
      diamond_id_t r;
      if (fread(&r, sizeof(diamond_id_t), 1, fp) != 1) {
        std::cerr << "unable to read root[" << i << "] from param file" << std::endl;
        return;
      }
      roots_.push_back(r);
    }

    if (print) {
      print_parameters();
    }
  }
    
  void repository_parameters_old::write_to_file(FILE* fp, bool print) const {
    if (fwrite(&patch_dim_, sizeof(uint32_t), 1, fp) != 1) {
      std::cerr << "unable to write patch_dim to param file" << std::endl;
      return;
    }

    if (fwrite(&is_planar_, sizeof(bool), 1, fp) != 1) {
      std::cerr << "unable to write is_planar to param file" << std::endl;
      return;
    }

    if (fwrite(&is_mono_scale_, sizeof(bool), 1, fp) != 1) {
      std::cerr << "unable to write is_mono_scale to param file" << std::endl;
      return;
    }

    if (fwrite(&wavelet_alpha_, sizeof(float), 1, fp) != 1) {
      std::cerr << "unable to write wavelet_alpha to param file" << std::endl;
      return;
    }

    if (fwrite(&length_, sizeof(double), 1, fp) != 1) {
      std::cerr << "unable to write length_scale to param file" << std::endl;
      return;
    }

    if (fwrite(&root_count_, sizeof(uint32_t), 1, fp) != 1) {
      std::cerr << "unable to write root_count to param file" << std::endl;
      return;
    } 

    for(uint32_t i = 0; i < root_count_; ++i) {
      const diamond_id_t& r = roots_[i];
      if (fwrite(&r, sizeof(diamond_id_t), 1, fp) != 1) {
        std::cerr << "unable to write  root[" << i << "] to param file" << std::endl;
        return;
      }
    }

    if (print) {
      print_parameters();
    }
  }
    
  void repository_parameters_old::set_roots(const std::vector<diamond_id_t>& roots) {
    roots_ = roots;
    root_count_ = roots_.size();
  }

  void repository_parameters_old::add_root(const diamond_id_t& r) {
    roots_.push_back(r);
    root_count_ = roots_.size();
  }
  
  const std::vector<repository_parameters_old::diamond_id_t>& repository_parameters_old::roots() const {
    return roots_;
  }

  void repository_parameters_old::print_parameters() const {
    std::cerr << "    patch dim     = " << patch_dim_ << std::endl;
    std::cerr << "    is planar     = " << (int)is_planar_ << std::endl;
    std::cerr << "    is mono scale = " << (int)is_mono_scale_ << std::endl;
    std::cerr << "    wavelet alpha = " << wavelet_alpha_ << std::endl;
    std::cerr << "    length        = " << length_ << std::endl;
    std::cerr << "    root count    = " << root_count_ << std::endl;
  } 
}
