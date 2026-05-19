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
#ifndef CBDAM_DIAMOND_REPOSITORY_STORAGE_OLD_HPP
#define CBDAM_DIAMOND_REPOSITORY_STORAGE_OLD_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_point.hpp>
#include "diamond_repository_old.hpp"
#include <sl/external_array.hpp>
#include <sl/dense_array.hpp>
#include <sl/hidwt_array_codec.hpp>
#include <sl/micro_jpgls_array_codec.hpp>
#include <sl/quantized_array_codec.hpp>
#include <map>
#include <set>
#include <zlib.h>

namespace cbdam {
  
  /// 3 int components: data offset, control point size, new point size
  class compressed_data_handle_old {
  protected:
    uint64_t offset_;
    uint32_t control_points_size_;
    uint32_t new_points_size_;
  public:

    inline compressed_data_handle_old(uint64_t o = 0, uint32_t sz1 = 0, uint32_t sz2 = 0) :
          offset_(o), control_points_size_(sz1), new_points_size_(sz2) {
    }
    
    inline uint64_t first_index() const {
      return offset_;
    }
    inline uint32_t size() const {
      return new_points_size_ + control_points_size_;
    }
    inline uint64_t last_index() const {
      return first_index() + uint64_t(size());
    }
    inline bool empty() const {
      return size() == 0;
    }
    
    inline uint64_t control_points_first_index() const {
      return first_index();
    }
    inline uint32_t control_points_size() const {
      return control_points_size_;
    }
    inline uint64_t control_points_last_index() const {
      return control_points_first_index()+uint64_t(control_points_size());
    }
    inline bool control_points_empty() const {
      return control_points_size() == 0;
    }
    
    inline uint64_t new_points_first_index() const {
      return first_index()+control_points_size();
    }
    inline uint32_t new_points_size() const {
      return new_points_size_;
    }
    inline uint64_t new_points_last_index() const {
      return new_points_first_index()+uint64_t(new_points_size());
    }
    inline bool new_points_empty() const {
      return new_points_size() == 0;
    }
  };

  typedef std::map<grid_point_t, compressed_data_handle_old> data_handle_map_t;

  /**
   * FIXME: The current version of the repository has a "broken" replace
   * policy, as no free list is maintained.
   */
  template<class OPERATOR_T>
  class diamond_repository_storage_old : public diamond_repository_old<OPERATOR_T> {
  public:
    typedef grid_point_t                        diamond_id_t;
    typedef diamond_repository<OPERATOR_T>      super_t;
    typedef typename super_t::value_t           value_t;
    typedef typename super_t::delta_t           delta_t;
    typedef typename super_t::diamond_data_t    diamond_data_t;
    typedef sl::dense_array<value_t,2,void>     array2_t;
    typedef sl::dense_array<delta_t,2,void>     array2_delta_t;
    
    typedef compressed_data_handle_old                      buffer_coords_t;
    typedef sl::external_array1<uint8_t>                data_xarray_t;
    typedef std::vector<uint8_t>                        data_buffer_t;
    typedef data_handle_map_t                           map_coords_t;
    
  protected:
    map_coords_t     map_id_data_;    // maps id to data coords in the array
    map_coords_t     map_id_roots_;   // maps id to root data coords in the array
    data_xarray_t*   data_repo_;
    std::string      file_name_;
    bool             write_mode_;
    bool             last_operation_success_;

    mutable sl::micro_jpgls_array_codec  multi_scale_codec_[CBDAM_BUILD_MAX_THREAD_COUNT];   
    mutable sl::quantized_array_codec    mono_scale_codec_[CBDAM_BUILD_MAX_THREAD_COUNT];

    sl::array_codec* current_codec(uint32_t thread_idx) const {
      assert(thread_idx<CBDAM_BUILD_MAX_THREAD_COUNT);
      if (is_mono_scale()) {
	return &(this->mono_scale_codec_[thread_idx]);
      } else {
	return &(this->multi_scale_codec_[thread_idx]);
      }
    }

    bool             is_mono_scale_;

  public:
    diamond_repository_storage_old();

    virtual ~diamond_repository_storage_old();

    virtual uint32_t compressed_diamond_size(const diamond_id_t& id) const;

    virtual void get_offsets(const diamond_id_t& id, 
			     float patch_edge_length, 
                             array2_delta_t& offset_p,
                             array2_delta_t& offset_q, 
			     uint32_t thread_idx = 0) const;
    
    virtual void get_root_offsets(const diamond_id_t& id,
                                  float patch_edge_length,
                                  array2_delta_t& offset, 
				  uint32_t thread_idx = 0) const;
    
    void get_root_buffers(typename map_coords_t::const_iterator it,
			  data_buffer_t& data_buf) const;
    
    void get_buffers(const diamond_id_t& id,
		     data_buffer_t& offset_p,
		     data_buffer_t& offset_q) const;
    
    bool is_empty() const;

    bool is_mono_scale() const;

    void set_mono_scale(bool x);

    void open_write(const char *file_name, bool is_temporary = false);

    void open_read(const char* file_name);

    void close();

    void minimize_footprint() const;

    bool has_root_patches(const diamond_id_t& id) const;
    
    bool has(const diamond_id_t& id) const;

    void compress_to_target_error(const array2_delta_t& delta, data_buffer_t& compressed_delta_p, float tolerance, bool use_amax_error, uint32_t thread_idx = 0) const;
    void compress_to_target_rms_error(const array2_delta_t& delta, data_buffer_t& compressed_delta_p, float tolerance, uint32_t thread_idx = 0) const;
    void compress_to_target_amax_error(const array2_delta_t& delta, data_buffer_t& compressed_delta_p, float tolerance, uint32_t thread_idx = 0) const;
    void decompress_to(array2_delta_t& delta, const uint8_t* data_buf, uint32_t data_buf_size, uint32_t thread_idx = 0) const;

    void set_root_patches(const diamond_id_t& id,
                          const data_buffer_t& data_buf);

    void set_deltas(const diamond_id_t& id,
                    const data_buffer_t& offset_p,
                    const data_buffer_t& offset_q);
    
    bool last_operation_success() const;

    std::vector<grid_point_t> roots() const;

    uint64_t size() const;

    const map_coords_t& map_id_data() const {
      return map_id_data_;
    }

    const map_coords_t& map_id_roots() const {
      return map_id_roots_;
    }
  };


} // namespace cbdam 

#endif // CBDAM_DIAMOND_REPOSITORY_STORAGE_OLD_HPP

#ifndef CBDAM_DIAMOND_REPOSITORY_STORAGE_OLD_IPP
#define CBDAM_DIAMOND_REPOSITORY_STORAGE_OLD_IPP

namespace cbdam {
  
  static inline std::size_t compact_fwrite(gzFile fp,  const data_handle_map_t& x) {
    // FIXME - check for errors with ferror, feof
    std::size_t bytes_written = 0;
    std::size_t key_bytes_written = 0;
    std::size_t data_bytes_written = 0;

    uint32_t x_size = x.size();
    bytes_written+= gzwrite(fp, &x_size, sizeof(uint32_t));
    for(data_handle_map_t::const_iterator it = x.begin(); 
	it != x.end(); 
	++it) {
      // Delta encode id
      const grid_point_t& x_id = it->first;
      grid_value_t x_0 = x_id[0];
      grid_value_t x_1 = x_id[1];
      grid_value_t x_2 = x_id[2];
      key_bytes_written+= gzwrite(fp, &x_0, sizeof(grid_value_t));
      key_bytes_written+= gzwrite(fp, &x_1, sizeof(grid_value_t));
      key_bytes_written+= gzwrite(fp, &x_2, sizeof(grid_value_t));
      
      // Store data
      const compressed_data_handle_old& x_data = it->second;
      data_bytes_written+= gzwrite(fp, &x_data, sizeof(compressed_data_handle_old));
    }
    bytes_written+= key_bytes_written + data_bytes_written;
    return bytes_written;
  }
  
  static inline std::size_t compact_fread(gzFile fp, data_handle_map_t& x) {
    // FIXME - check for errors with ferror, feof
    std::size_t bytes_read = 0;
    std::size_t key_bytes_read = 0;
    std::size_t data_bytes_read = 0;
    uint32_t x_size;
    bytes_read+= gzread(fp, &x_size, sizeof(uint32_t));
    SL_TRACE_OUT(1) << "fread: MAP SIZE = " << x_size << std::endl;
    
    for (std::size_t i=0; i<x_size; ++i) {
      grid_value_t x_0, x_1, x_2;
      key_bytes_read+= gzread(fp, &x_0, sizeof(grid_value_t));
      key_bytes_read+= gzread(fp, &x_1, sizeof(grid_value_t));
      key_bytes_read+= gzread(fp, &x_2, sizeof(grid_value_t));
      grid_point_t x_id(x_0, x_1, x_2);

      compressed_data_handle_old x_data;
      data_bytes_read+= gzread(fp, &x_data, sizeof(compressed_data_handle_old));
      if (i<10) SL_TRACE_OUT(1) << "x_id = " << x_id[0] << " " << x_id[1] << " " << x_id[2] << std::endl;
      x[x_id] = x_data;
    }

    bytes_read += key_bytes_read + data_bytes_read;
    return bytes_read;
  }

  template<class OPERATOR_T>
  inline diamond_repository_storage_old<OPERATOR_T>::diamond_repository_storage_old() {
    last_operation_success_ = true;
    write_mode_ = false;
    is_mono_scale_ = false;

    // pointers
    data_repo_  = 0;
  }

  template<class OPERATOR_T>
  inline diamond_repository_storage_old<OPERATOR_T>::~diamond_repository_storage_old() {
    close();
  }

  template<class OPERATOR_T>
  inline bool diamond_repository_storage_old<OPERATOR_T>::is_empty() const {
    return map_id_data_.empty() && map_id_roots_.empty();
  }

  template<class OPERATOR_T>
  inline bool diamond_repository_storage_old<OPERATOR_T>::is_mono_scale() const {
    return is_mono_scale_;
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::set_mono_scale(bool x) {
    assert(is_empty());
    is_mono_scale_ = x;
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::open_write(const char* file_name,
                                                                 bool is_temporary) {
    file_name_ = std::string(file_name);
    write_mode_ = !is_temporary;
    if (write_mode_) {
      data_repo_ = new data_xarray_t(file_name_ + ".repo", "w", 16*1024*1024);
    } else {
      data_repo_ = new data_xarray_t(file_name_ + ".repo", "t", 16*1024*1024);
    }
    last_operation_success_ = data_repo_->is_open();
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::open_read(const char* file_name) {
    file_name_ = std::string(file_name);

    // check if file exist
    FILE* fp = fopen((file_name_+".param").c_str(), "rb");
    if (fp == 0) {
      last_operation_success_ = false;
      return;
    } else {
      fclose(fp);
    }

    write_mode_ = false;
    data_repo_ = new data_xarray_t(file_name_ + ".repo", "r", 16*1024*1024);
    last_operation_success_ = data_repo_->is_open();

    // FIXME
    if (false) {
      uint64_t prefetch_size = std::min(uint64_t(512*1024*1024),
                                        uint64_t(data_repo_->size()));
      std::cerr << "Prefetch first " << prefetch_size/1024/1024 << "MB..." << std::endl;
      uint8_t* data_ptr = data_repo_->range_page_in(0, prefetch_size);
      uint8_t tmp =0;
      for (uint64_t i=0; i<prefetch_size; ++i) {
        tmp += data_ptr[i];
      }
      if (tmp) std::cerr << "... prefetching done." << std::endl;
    } else {
      std::cerr << "no prefetch\n";
    }
    
    // read param file
    std::string name = file_name_ + ".param";
    fp = fopen(name.c_str(), "rb");
    if (fp == 0) {
      last_operation_success_ = false;
      std::cerr << "ERROR: failed to open param file " << name << " for reading\n";
      return;
    }
    int count =
      fread(&this->patch_dim_, sizeof(this->patch_dim_), 1, fp) +
      fread(&this->is_planar_, sizeof(this->is_planar_), 1, fp) +
      fread(&this->is_mono_scale_, sizeof(this->is_mono_scale_), 1, fp) +
      fread(&this->wavelet_alpha_, sizeof(this->wavelet_alpha_), 1, fp);

    if (count != 4) {
      last_operation_success_ = false;
      std::cerr << "ERROR: failed to read param file " << name << std::endl;
      fclose(fp);
      return;
    }
    if (this->is_planar_) {
      count = fread(&this->planar_terrain_root_side_length_, sizeof(this->planar_terrain_root_side_length_), 1, fp);
    } else {
      count = fread(&this->spherical_terrain_radius_, sizeof(this->spherical_terrain_radius_), 1, fp);
    }
    if (count != 1) {
      std::cerr << "ERROR: failed to read param file " << name << std::endl;
      fclose(fp);
      return;
    }
    fclose(fp);

    if (this->is_planar()) {
      set_patch_dim(this->patch_dim_);
      set_planar_with_side_length(this->planar_terrain_root_side_length_);
      std::cerr << "Planar terrain with side = " << this->planar_terrain_root_side_length() << std::endl;
    } else {
      set_patch_dim(this->patch_dim_);
      set_spherical_with_radius(this->spherical_terrain_radius_);
      std::cerr << "Spherical terrain with radius = " << this->spherical_terrain_radius() << std::endl;
    }

    std::cerr << "wavelet alpha = " << this->wavelet_alpha_ << std::endl;
    std::cerr << "patch dim     = " << this->patch_dim() << std::endl;    

    // read root file
    name = file_name_ + ".root";
    gzFile zfp = gzopen(name.c_str(), "rb");
    if (zfp == 0) {
      last_operation_success_ = false;
      std::cerr << "ERROR: failed to open root file " << name << " for reading\n";
      return;
    }
    map_id_roots_.clear();
    compact_fread(zfp, map_id_roots_);
    gzclose(zfp);
    SL_TRACE_OUT(1) << "Inserted " << map_id_roots_.size() << " root diamonds\n";
    
    // read data file
    name = file_name_ + ".data";
    zfp = gzopen(name.c_str(), "rb");
    if (zfp == 0) {
      last_operation_success_ = false;
      std::cerr << "ERROR: failed to open data file " << name << " for reading\n";
      return;
    }
    map_id_data_.clear();
    compact_fread(zfp, map_id_data_);
    gzclose(zfp);
    SL_TRACE_OUT(1) << "Inserted " << map_id_data_.size() << " data diamonds\n";
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::minimize_footprint() const {
    if (data_repo_) data_repo_->minimize_footprint();
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::close() {
    if (data_repo_) {
      if (write_mode_) {
        // write param file
        std::string name = file_name_ + ".param";
        FILE* fp = fopen(name.c_str(), "wb");
        if (fp == 0) {
          last_operation_success_ = false;
          std::cerr << "ERROR: failed to open param file " << name << " for writing\n";
          return;
        }

        int count = 
          fwrite(&this->patch_dim_, sizeof(this->patch_dim_), 1, fp) +
          fwrite(&this->is_planar_, sizeof(this->is_planar_), 1, fp) +
	  fwrite(&this->is_mono_scale_, sizeof(this->is_mono_scale_), 1, fp) +
	  fwrite(&this->wavelet_alpha_, sizeof(this->wavelet_alpha_), 1, fp);
	
        if (count != 4) {
          last_operation_success_ = false;
          std::cerr << "ERROR: failed to write param file " << name << std::endl;
          fclose(fp);
          return;
        }
        if (this->is_planar_) {
          count = fwrite(&this->planar_terrain_root_side_length_, sizeof(this->planar_terrain_root_side_length_), 1, fp);
        } else {
          count = fwrite(&this->spherical_terrain_radius_, sizeof(this->spherical_terrain_radius_), 1, fp);
        }
        if (count != 1) {
          last_operation_success_ = false;
          std::cerr << "ERROR: failed to write param file " << name << " for reading\n";
          fclose(fp);
          return;
        }
        fclose(fp);
        
	std::cerr << "written alpha " << this->wavelet_alpha_ << std::endl;
        // save maps
        name = file_name_ + ".root";
        gzFile zfp = gzopen(name.c_str(), "wb");
        if (zfp == 0) {
          last_operation_success_ = false;
          std::cerr << "ERROR: failed to save root file " << name << std::endl;
          return;
        }
	compact_fwrite(zfp, map_id_roots_);
        gzclose(zfp);

        name = file_name_ + ".data";
        zfp = gzopen(name.c_str(), "wb");
        if (zfp == 0) {
          last_operation_success_ = false;
          std::cerr << "ERROR: failed to save data file " << name << std::endl;
          return;
        }
	compact_fwrite(zfp, map_id_data_);
        gzclose(zfp);
      }
    
      data_repo_->close();
      delete data_repo_;
      data_repo_ = 0;

      // clear maps
      map_id_data_.clear();
      map_id_roots_.clear();
    }
  }

  template<class OPERATOR_T>
  inline bool diamond_repository_storage_old<OPERATOR_T>::has_root_patches(const diamond_id_t& id) const {
    return map_id_roots_.find(id) != map_id_roots_.end();
  }
    
  template<class OPERATOR_T>
  inline bool diamond_repository_storage_old<OPERATOR_T>::has(const diamond_id_t& id) const {
    return map_id_data_.find(id) != map_id_data_.end();
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::compress_to_target_rms_error(const array2_delta_t& delta,
                                                                                   data_buffer_t& compressed_delta,
                                                                                   float tolerance, 
										   uint32_t thread_idx) const {
    this->value_operator_.compress_to_target_rms_error(delta, 
						       compressed_delta, 
						       tolerance,
						       current_codec(thread_idx));
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::compress_to_target_amax_error(const array2_delta_t& delta, 
                                                                                    data_buffer_t& compressed_delta, 
                                                                                    float tolerance, 
										    uint32_t thread_idx) const {
   this->value_operator_.compress_to_target_amax_error(delta, 
							compressed_delta, 
							tolerance,
							current_codec(thread_idx));
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::compress_to_target_error(const array2_delta_t& delta, 
                                                                               data_buffer_t& compressed_delta, 
                                                                               float tolerance,
                                                                               bool use_amax_error, 
									       uint32_t thread_idx) const {
    if (use_amax_error) {
      compress_to_target_amax_error(delta, compressed_delta, tolerance, thread_idx);
    } else {
      compress_to_target_rms_error(delta, compressed_delta, tolerance, thread_idx);
    }
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::decompress_to(array2_delta_t& delta, const uint8_t* data_buf, uint32_t data_buf_size, uint32_t thread_idx) const {
    this->value_operator_.decompress_to(delta, 
					data_buf, 
					data_buf_size,  
					current_codec(thread_idx));
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::set_root_patches(const diamond_id_t& id, const data_buffer_t& data_buf) {
    assert(!has_root_patches(id));
    // FIXME FIXME FIXME
    // FIXME BROKEN REPLACE POLICY
    // If data is already there, we create a memory leak!

    // set map entry
    uint64_t current_size = data_repo_->size();
    uint32_t data_size =  data_buf.size();
    map_id_roots_[id] = buffer_coords_t(current_size, data_size, 0);

    // copy to output
    uint64_t updated_size = current_size + data_size;
    data_repo_->resize(updated_size);
    uint8_t* data = data_repo_->range_page_in(current_size, updated_size);
    memcpy(data, &(data_buf[0]), data_size);
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::set_deltas(const diamond_id_t& id,
                                                                 const data_buffer_t& delta_p,
                                                                 const data_buffer_t& delta_q) {
    // set map entry
    uint32_t p_size =  delta_p.size();
    uint32_t q_size =  delta_q.size();
    uint32_t pq_size = p_size+q_size;

    // FIXME FIXME FIXME
    // FIXME BROKEN REPLACE POLICY
    // If data is already there, use the slot if new data fits in.
    // Otherwise, append at end and create a memory leak!
    typename map_coords_t::iterator old_it = map_id_data_.find(id);
    if (old_it != map_id_data_.end() && old_it->second.size() >= pq_size) {
      // REPLACE
      SL_TRACE_OUT(1) << "Replacing " << id << std::endl;

      buffer_coords_t old_coords = old_it->second;
      uint8_t* data = data_repo_->range_page_in(old_coords.first_index(),
                                                old_coords.last_index());
      if (p_size) { memcpy(data, &(delta_p[0]), p_size);}
      if (q_size) { memcpy(&(data[p_size]), &(delta_q[0]), q_size);}

      old_it->second = buffer_coords_t(old_coords.first_index(), p_size, q_size);
    } else {
      // APPEND
      uint64_t repo_size = data_repo_->size();
      map_id_data_[id] = buffer_coords_t(repo_size, p_size, q_size);

      // copy to output
      uint64_t updated_size = repo_size + pq_size;
      data_repo_->resize(updated_size);
      uint8_t* data = data_repo_->range_page_in(repo_size, updated_size);

      if (p_size) { memcpy(data, &(delta_p[0]), p_size);}
      if (q_size) { memcpy(&(data[p_size]), &(delta_q[0]), q_size);}
    }
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::get_offsets(const diamond_id_t& id, float /*patch_edge_length*/, 
                                                                  array2_delta_t& offset_p,
                                                                  array2_delta_t& offset_q, 
								  uint32_t thread_idx) const {
    typename map_coords_t::const_iterator it = map_id_data_.find(id);
    if (it != map_id_data_.end()) {
      const buffer_coords_t& coords = it->second;
      assert(coords.last_index() <= data_repo_->size());
      const uint8_t* data = data_repo_->range_page_in(coords.first_index(), coords.last_index());
      if (coords.control_points_size()) {
        decompress_to(offset_p, data, coords.control_points_size(), thread_idx);
      }
      if (coords.new_points_size()) {
        decompress_to(offset_q, &(data[coords.control_points_size()]), coords.new_points_size(), thread_idx);
      }
    } else {
      SL_TRACE_OUT(-1) << "missing data " << id << std::endl;
    }
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::get_root_offsets(const diamond_id_t& id, float /*patch_edge_length*/,
                                                                       array2_delta_t& offset, 
								       uint32_t thread_idx) const {
    typename map_coords_t::const_iterator it = map_id_roots_.find(id);
    assert(it != map_id_roots_.end());
    const buffer_coords_t& coords = it->second;
    // coords0 = data_offset, coords1 control_data_size, coords2 new_data_size

    // decompress data
    assert(coords.last_index() <= data_repo_->size());
    const uint8_t* data = data_repo_->range_page_in(coords.first_index(), coords.last_index());
    decompress_to(offset, data, coords.control_points_size(), thread_idx);
  }
  
  template<class OPERATOR_T>
  inline bool diamond_repository_storage_old<OPERATOR_T>::last_operation_success() const {
    return last_operation_success_;
  }

  template<class OPERATOR_T>
  inline std::vector<grid_point_t> diamond_repository_storage_old<OPERATOR_T>::roots() const {
    std::vector<diamond_id_t> r;
    for(map_coords_t::const_iterator it = map_id_roots_.begin();
	it != map_id_roots_.end();
	++it) {
      r.push_back(it->first);
    }
    return r;
  }

  template<class OPERATOR_T>
  inline uint64_t diamond_repository_storage_old<OPERATOR_T>::size() const { 
    const float gzcompress_factor = 0.30f; // FIXME Approx
    return uint64_t(gzcompress_factor * (map_id_roots_.size() + map_id_data_.size()) * sizeof(buffer_coords_t)) + data_repo_->size();
  }

  template<class OPERATOR_T>
  inline uint32_t diamond_repository_storage_old<OPERATOR_T>::compressed_diamond_size(const diamond_id_t& id) const {
    typename map_coords_t::const_iterator it = map_id_data_.find(id);
    if (it != map_id_data_.end()) {
      return it->second.size();
    } else {
      return 0;
    }
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::get_root_buffers(typename map_coords_t::const_iterator it,
									   data_buffer_t& data_buf) const {
    if (it != map_id_roots_.end()) {
      const buffer_coords_t& coords = it->second;
      const uint8_t* data = data_repo_->range_page_in(coords.first_index(), coords.last_index());
      data_buf.resize(coords.size());
      memcpy(&(data_buf[0]), data, coords.size());
    } else {
      SL_TRACE_OUT(-1) << "missing data " << it->first << std::endl;
    }
  }
    
  template<class OPERATOR_T>
  inline void diamond_repository_storage_old<OPERATOR_T>::get_buffers(const diamond_id_t& id,
								      data_buffer_t& offset_p,
								      data_buffer_t& offset_q) const {
    typename map_coords_t::const_iterator it = map_id_data_.find(id);
    if (it != map_id_data_.end()) {
      const buffer_coords_t& coords = it->second;
      assert(coords.last_index() <= data_repo_->size());
      const uint8_t* data = data_repo_->range_page_in(coords.first_index(), coords.last_index());
      offset_p.resize(coords.control_points_size());
      offset_q.resize(coords.new_points_size());
      if (offset_p.size()) { memcpy(&(offset_p[0]), data, offset_p.size());}
      if (offset_q.size()) { memcpy(&(offset_q[0]), &(data[offset_p.size()]), offset_q.size());}
    } else {
      SL_TRACE_OUT(-1) << "missing data " << it->first << std::endl;
    }
 
  }
    
  
} // namespace cbdam 

#endif // CBDAM_DIAMOND_REPOSITORY_STORAGE_OLD_IPP
