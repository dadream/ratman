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
#ifndef CBDAM_DIAMOND_REPOSITORY_STORAGE_HPP
#define CBDAM_DIAMOND_REPOSITORY_STORAGE_HPP


#include <cstring>

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_point.hpp>
#include <vic/cbdam/base/diamond_repository.hpp>
#include <vic/cbdam/base/byte_array_accessor.hpp>
#include <vic/vfs/repository.hpp>
#include <sl/os_file.hpp>
#include <sl/dense_array.hpp>
//#include <sl/hidwt_array_codec.hpp>
//#include <sl/micro_jpgls_array_codec.hpp>
#include <sl/quantized_array_codec.hpp>

// FIXME ####################
//#include <sl/quantized_array_codec_v2.hpp>

namespace cbdam {
  
  /**
   * FIXME: The current version of the repository has a "broken" replace
   * policy, as no free list is maintained.
   */
  template<class OPERATOR_T>
  class diamond_repository_storage : public diamond_repository<OPERATOR_T> {
  public:
    typedef grid_point_t                        diamond_id_t;
    typedef diamond_repository<OPERATOR_T>      super_t;
    typedef typename super_t::value_t           value_t;
    typedef typename super_t::operator_t        operator_t;
    typedef sl::dense_array<value_t,2,void>     array2_t;
    typedef std::vector<uint8_t>                data_buffer_t;
    typedef vic::vfs::repository		repository_t;

    
  protected:
    operator_t	     diamond_operator_;
    std::string      file_name_;
    repository_t     repo_data_;    // maps id to data coords in the array
    repository_t     repo_root_;   // maps id to root data coords in the array
    bool	     is_temporary_;

    // ############### FIXME
    mutable sl::quantized_array_codec    codec_[CBDAM_BUILD_MAX_THREAD_COUNT];
    
    sl::array_codec* current_codec(uint32_t thread_idx) const {
      assert(thread_idx<CBDAM_BUILD_MAX_THREAD_COUNT);

      return &(this->codec_[thread_idx]);
    }

  public:
    diamond_repository_storage();

    virtual ~diamond_repository_storage();

    virtual void get_data(const diamond_id_t& id, 
                          array2_t& offset, 
                          uint32_t thread_idx = 0,
                          float patch_edge_length = 1) const;
    
    void get_data_buffer(const diamond_id_t& id, 
			 data_buffer_t& x) const;

    virtual void get_root_data(const diamond_id_t& id,
                               array2_t& offset, 
                               uint32_t thread_idx = 0,
                               float patch_edge_length = 1) const;
    
    //    bool is_empty() const;


    float height_scale_factor() const;

    void set_height_scale_factor(float x);

    void set_srs(const std::string& x);

    void set_about(const std::string& x);

    void open_write(const char *file_name, uint32_t expected_average_data_size, bool is_temporary = false);

    void open_read(const char* file_name);

    void close();

    void delete_all_files();

    void minimize_footprint() const;

    bool has_root_data(const diamond_id_t& id) const;
    
    bool has_data(const diamond_id_t& id) const;

    void compress_to_target_error(const array2_t& delta, data_buffer_t& compressed_delta, float tolerance, bool use_amax_error, uint32_t thread_idx = 0) const;
    void compress_lossless(const array2_t& delta, data_buffer_t& compressed_delta, uint32_t thread_idx = 0) const;

    void decompress_to(array2_t& delta, const uint8_t* data_buf, uint32_t data_buf_size, uint32_t thread_idx = 0) const;

    void set_root_data(const diamond_id_t& id, const data_buffer_t& r);

    void set_data(const diamond_id_t& id, const data_buffer_t& p);
    
    bool is_open() const;

    uint64_t size() const;

    const repository_parameters& params() const;

    std::string file_name() const;
    
  protected:
    void set_data_to_repo(const diamond_id_t& id,
                          const data_buffer_t& delta,
                          repository_t& repo);

    void get_data_from_repo(const diamond_id_t& id, 
                            array2_t& delta,
                            uint32_t thread_idx,
                            const repository_t& repo) const;

    
  };


} // namespace cbdam 

#endif // CBDAM_DIAMOND_REPOSITORY_STORAGE_HPP

#ifndef CBDAM_DIAMOND_REPOSITORY_STORAGE_IPP
#define CBDAM_DIAMOND_REPOSITORY_STORAGE_IPP

namespace cbdam {

  template<class OPERATOR_T>
  inline diamond_repository_storage<OPERATOR_T>::diamond_repository_storage() {
    for (std::size_t i=0; i<CBDAM_BUILD_MAX_THREAD_COUNT; ++i) {
      codec_[i].set_is_compressing_header(true); // FIXME 
      codec_[i].set_is_delta_encoding(false);
    }
  }

  template<class OPERATOR_T>
  inline diamond_repository_storage<OPERATOR_T>::~diamond_repository_storage() {
    close();
  }

#if 0
  template<class OPERATOR_T>
  inline bool diamond_repository_storage<OPERATOR_T>::is_empty() const {
    return repo_data_.number_of_elements() == 0 && repo_root_.number_of_elements() == 0;
  }
#endif

  template<class OPERATOR_T>
  inline float diamond_repository_storage<OPERATOR_T>::height_scale_factor() const {
    return this->repo_params_.height_scale_factor();
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::set_height_scale_factor(float x) {
    this->repo_params_.height_scale_factor() = x;
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::set_srs(const std::string& x) {
    this->repo_params_.srs() = x;
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::set_about(const std::string& x) {
    this->repo_params_.about() = x;
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::open_write(const char* file_name, 
								 uint32_t expected_average_data_size,
                                                                 bool is_temporary) {
    is_temporary_ = is_temporary;
    file_name_ = std::string(file_name);
    std::string data_name = file_name_ + ".data";
    std::string root_name = file_name_ + ".root";
    repo_data_.open_write(data_name.c_str(), expected_average_data_size, is_temporary_);
    repo_root_.open_write(root_name.c_str(), expected_average_data_size, is_temporary_);
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::open_read(const char* file_name) {
    file_name_ = std::string(file_name);
    std::string data_name = file_name_ + ".data";
    std::string root_name = file_name_ + ".root";
    repo_data_.open_read(data_name.c_str());
    repo_root_.open_read(root_name.c_str());

    if (is_open()) {
      // read params
      std::string repo_name = file_name_ + ".xml";
      this->repo_params_.read_from_file(repo_name.c_str(), false); // bool = debugging!
      if (!this->repo_params_.last_operation_success()) {
        std::cerr << "unable to read parameters from " << repo_name << std::endl;
      }
    }
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::minimize_footprint() const {
    repo_data_.minimize_footprint();
    repo_root_.minimize_footprint();
  }
  
  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::delete_all_files() {
    if (is_open()) close();
    if (!file_name_.empty()) {
      std::string param_name = file_name_ + ".xml";
      sl::os_file::file_delete(param_name.c_str());
    }
    if (!repo_data_.file_name().empty()) sl::os_file::file_delete(repo_data_.file_name().c_str());
    if (!repo_root_.file_name().empty()) sl::os_file::file_delete(repo_root_.file_name().c_str());
  }
  
  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::close() {
    repo_data_.close();
    repo_root_.close();

    if (repo_data_.write_mode() && is_temporary_) {
      std::string param_name = file_name_ + ".xml";
      sl::os_file::file_delete(param_name.c_str());
      sl::os_file::file_delete(repo_data_.file_name().c_str());
      sl::os_file::file_delete(repo_root_.file_name().c_str());
    }

    if (repo_data_.write_mode() & !is_temporary_) {
      // set header parameters
      std::string param_name = file_name_ + ".xml";
      this->repo_params_.write_to_file(param_name.c_str(), false);
      if (!this->repo_params_.last_operation_success()) {
        std::cerr << "unable to open " << param_name << " for writing params" << std::endl;
        std::cerr << "these are the parames that shoud be saved:" << std::endl;
        this->repo_params_.print_parameters();
      }
    }
  }

  template<class OPERATOR_T>
  inline bool diamond_repository_storage<OPERATOR_T>::has_root_data(const diamond_id_t& id) const {
    return repo_root_.has_data(repository_t::key_t(id[0], id[1], id[2]));
  }
    
  template<class OPERATOR_T>
  inline bool diamond_repository_storage<OPERATOR_T>::has_data(const diamond_id_t& id) const {
    return repo_data_.has_data(repository_t::key_t(id[0], id[1], id[2]));
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::compress_to_target_error(const array2_t& delta, 
                                                                               data_buffer_t& compressed_delta, 
                                                                               float tolerance,
                                                                               bool use_amax_error, 
									       uint32_t thread_idx) const {
    diamond_operator_.compress_to_target_error(delta, compressed_delta, tolerance, current_codec(thread_idx), use_amax_error);
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::compress_lossless(const array2_t& delta, 
									data_buffer_t& compressed_delta, 
									uint32_t thread_idx) const {
    this->compress_to_target_error(delta, compressed_delta, 0.0f, true, thread_idx);
  }


  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::decompress_to(array2_t& delta, 
								    const uint8_t* data_buf, 
								    uint32_t data_buf_size, 
								    uint32_t thread_idx) const {
    diamond_operator_.decompress_to(delta, 
				    data_buf, 
				    data_buf_size,  
				    current_codec(thread_idx));
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::set_root_data(const diamond_id_t& id, const data_buffer_t& r) {
    assert(!has_root_data(id));
    data_buffer_t fake_data_buf;
    set_data_to_repo(id, r, repo_root_);
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::get_root_data(const diamond_id_t& id, 
                                                                    array2_t& offset, 
                                                                    uint32_t thread_idx,
                                                                    float /*patch_edge_length*/) const {
    get_data_from_repo(id, offset, thread_idx, repo_root_);
    //    assert(fake_offset.extent() == sl::index<2>(0,0));
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::set_data(const diamond_id_t& id,
                                                               const data_buffer_t& delta) {
    set_data_to_repo(id, delta, repo_data_);
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::get_data(const diamond_id_t& id, 
							       array2_t& delta, 
                                                               uint32_t thread_idx,
                                                               float /*patch_edge_length*/) const {
    get_data_from_repo(id, delta, thread_idx, repo_data_);
  }
  
  template<class OPERATOR_T>
  inline uint64_t diamond_repository_storage<OPERATOR_T>::size() const { 
    return repo_data_.size() + repo_root_.size();
  }

  template<class OPERATOR_T>
  inline std::string diamond_repository_storage<OPERATOR_T>::file_name() const { 
    return file_name_;
  }

  template<class OPERATOR_T>
  inline bool diamond_repository_storage<OPERATOR_T>::is_open() const {
    return repo_root_.is_open() && repo_data_.is_open();
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::set_data_to_repo(const diamond_id_t& id,
								       const data_buffer_t& delta,
                                                                       repository_t& repo) {
    assert(delta.size()!=0);
    uint32_t delta_size = delta.size();
    uint32_t size =  delta_size + sizeof(uint32_t);
    data_buffer_t data;
    data.resize(size);
    byte_array_accessor::set_first_patch_size(&(data[0]), delta_size);
    if (delta_size) { 
      memcpy(byte_array_accessor::first_patch_pointer(&(data[0])), &(delta[0]), delta_size);
    } else {
      SL_TRACE_OUT(-1) << "set data to " << id << " of size 0" << std::endl;
    }
    repo.set_data(repository_t::key_t(id[0], id[1], id[2]), &(data[0]), data.size());
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::get_data_from_repo(const diamond_id_t& id, 
									 array2_t& delta,
                                                                         uint32_t thread_idx,
                                                                         const repository_t& repo) const {
    uint32_t size;
    const uint8_t* data = repo.get_data(repository_t::key_t(id[0], id[1], id[2]), size);
    assert(data);

    uint32_t input_size = byte_array_accessor::first_patch_size(data);
    if (input_size > 0) {
      decompress_to(delta, byte_array_accessor::first_patch_pointer(data), input_size, thread_idx);
    } else {
      SL_TRACE_OUT(-1) << "extracting null data from " << id  << std::endl;
    }
  }

  template<class OPERATOR_T>
  inline const repository_parameters& diamond_repository_storage<OPERATOR_T>::params() const {
    return this->repo_params_;
  }

  template<class OPERATOR_T>
  inline void diamond_repository_storage<OPERATOR_T>::get_data_buffer(const diamond_id_t& id, 
								      data_buffer_t& x) const {
    uint32_t size;
    const uint8_t* data = repo_data_.get_data(repository_t::key_t(id[0], id[1], id[2]), size);
    assert(data);

    uint32_t data_size = byte_array_accessor::first_patch_size(data);
    if (data_size > 0) {
      x.resize(data_size);
      memcpy(&(x[0]), byte_array_accessor::first_patch_pointer(data), data_size);
    } else {
      SL_TRACE_OUT(-1) << "extracting null data from " << id  << std::endl;
    }

  }
} // namespace cbdam 

#endif // CBDAM_DIAMOND_REPOSITORY_STORAGE_IPP
