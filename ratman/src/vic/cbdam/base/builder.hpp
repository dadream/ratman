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
#ifndef CBDAM_BUILDER_HPP
#define CBDAM_BUILDER_HPP

/////////////////////// FIXME
//#undef NDEBUG
/////////////////////// FIXME

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_point.hpp>
#include <vic/cbdam/base/color_rgb.hpp>
#include <vic/cbdam/base/grid_diamond.hpp>
#include <vic/cbdam/base/diamond_operator.hpp>
#include <vic/cbdam/base/diamond_repository_storage.hpp>
#include <vic/cbdam/base/diamond_graph_builder.hpp>
#include <sl/clock.hpp>
#include <sl/utility.hpp>
#include <sl/os_file.hpp>
#include <cstring>

#ifndef _WIN32
  #include <unistd.h>
#endif

namespace cbdam {

  // !!!! FIXME !!!!
  
  template <class T> struct nearly_lossless_tolerance_traits { };
  template <> struct nearly_lossless_tolerance_traits<int32_t> { static inline double value(double tol) { return 0.01*tol; } };
  template <> struct nearly_lossless_tolerance_traits<delta_color3_t> { static inline double value(double tol) { return std::min(tol, 0.87); } };
  
  /**
   *  Parallel builder of cbdam structures
   */
  template<class OPERATOR_T>
  class builder {
  public:
    typedef diamond_graph_builder                       diamond_graph_builder_t;
    typedef diamond_graph_builder::diamond_graph_t      diamond_graph_t;
    typedef diamond_graph_builder::diamond_t            diamond_t;
    typedef diamond_graph_builder::diamond_state_t      diamond_state_t;
    typedef grid_point_t                                diamond_id_t;
    
    typedef OPERATOR_T                                  operator_t;
    typedef typename operator_t::value_t                value_t;
    typedef typename sl::dense_array<value_t,2,void>    array2_t;

    typedef diamond_repository_storage<operator_t>      diamond_repository_t;
    typedef std::vector<uint8_t>                        data_buffer_t;

    typedef vic::geo::generic_map_sampler<value_t>      geo_map_sampler_t; 
    typedef coordinate_transform                        geo_xform_t;

  protected:
    operator_t                          diamond_operator_;

    // Input
    const geo_map_sampler_t*            in_data_sampler_;
    const geo_xform_t*                  in_geo_xform_;

    diamond_graph_t*                    diamond_graph_;
    
    // Output
    diamond_repository_t                out_compressed_repo_;

    // Temporary
    diamond_repository_t                tmp_filtered_repo_[2]; // 0->level=even; 1->level=odd

    // Parameters
    uint32_t                            arg_patch_dim_;
    double                              arg_tolerance_;
    bool                                arg_use_amax_error_;
    double                              arg_min_sample_spacing_;
    double                              arg_data_scale_factor_;
    std::string                         arg_tmp_dir_;
    std::string                         arg_srs_;
    std::string                         arg_about_;
    bool                                arg_reuse_graph_;
    bool                                arg_keep_graph_;

    std::string                         arg_output_fname_;
    
    double                              in_data_scale_factor_; // ==  arg_data_scale_factor_ / in_data_sampler->unit_scale() 

    // Running values
    bool                                build_aborted_;
    std::size_t                         build_current_level_;
    std::size_t                         build_tmp_expected_average_record_size_;
    std::size_t                         build_out_expected_average_record_size_;
    
    // Statistics
    sl::real_time_clock                 stats_clock_;
    std::size_t                         stats_processed_diamonds_;
    sl::uint64_t                        stats_stored_data_size_;
    sl::uint64_t                        stats_max_tmp_size_;
    
  public:

    builder();

    virtual ~builder();

  public: // Parameters
    
    CBDAM_RW_ACCESSOR(uint32_t, arg_patch_dim);
    CBDAM_RW_ACCESSOR(double, arg_tolerance);
    CBDAM_RW_ACCESSOR(bool, arg_use_amax_error);
    CBDAM_RW_ACCESSOR(double, arg_min_sample_spacing);
    CBDAM_RW_ACCESSOR(double, arg_data_scale_factor);
    CBDAM_RW_ACCESSOR(std::string, arg_tmp_dir);
    CBDAM_RW_ACCESSOR(std::string, arg_srs);
    CBDAM_RW_ACCESSOR(std::string, arg_about);
    CBDAM_RW_ACCESSOR(bool, arg_reuse_graph);
    CBDAM_RW_ACCESSOR(bool, arg_keep_graph);

  public: // Construction

    virtual void build(const std::string& output_fname,
                       geo_map_sampler_t* input_sampler,
		       geo_xform_t*       geo_xform);

    bool last_build_successful() const;
    
  protected: // Access to tmprepositories
    
    std::size_t tmp_expected_average_record_size() const;
    std::size_t out_expected_average_record_size() const;
    
    std::string tmp_file_name(std::size_t level) const;
    void        tmp_open_read(std::size_t level);
    void        tmp_open_write(std::size_t level);
    void        tmp_close(std::size_t level);

  protected: // Off-core graph
    
    std::string tmp_graph_name() const;

  protected: // Main coordinator process
    
    virtual void main_build();

    virtual void main_build_begin();
    
    virtual void main_build_end();

    virtual void main_build_graph();

    virtual void main_build_estimate_bitrates_and_open_repositories();
    
    virtual void main_build_levels_fine_to_coarse();

    virtual void main_build_level_begin(std::size_t level);

    virtual void main_build_level_end();

    virtual void main_build_level();

    virtual void main_build_level_sequential();

  protected:

    virtual void main_progress_report_begin();

    virtual void main_progress_report(const data_buffer_t& x_compressed_l,
                                      const data_buffer_t& x_compressed_h);

    virtual void main_progress_report_end();
    
  protected: // Worker: sample and compress

    virtual void worker_build();

    virtual void worker_build_begin();

    virtual void worker_build_process_requests();
    
    virtual void worker_build_end();

    virtual void worker_build_level_begin(std::size_t level);

    virtual void worker_build_level_end();

    virtual void worker_sample_children_in(array2_t& p,
                                           array2_t& q,
                                           const diamond_t& d,
                                           bool has_fragment_0,
                                           bool has_fragment_1);

    virtual void worker_sample_input_in(array2_t& p,
                                        array2_t& q,
                                        const diamond_t& d);
        
    virtual void worker_build_diamond_from_children_in(data_buffer_t& compressed_l,
                                                       data_buffer_t& compressed_h,
                                                       const diamond_t& d,
                                                       bool has_fragment_0,
                                                       bool has_fragment_1);
                                                       
    virtual void worker_build_diamond_from_input_in(data_buffer_t& compressed_l,
                                                    data_buffer_t& compressed_h,
                                                    const diamond_t& d);

  protected: // Diamond operations - move somewhere else

    value_t get_value_from_child(int dx, int dy, 
                                 const array2_t child_l[4]) const;

    void child_coords_from_parent_coords(int x, int y, uint32_t child_id,
                                         int& c_x, int& c_y) const;
    
    void parent_coords_from_child_coords(int x, int y, uint32_t child_id,
                                         int& p_x, int& p_y) const;

  }; // class builder
  
} // namespace cbdam


namespace cbdam {

  // =========================== GENERAL
  
  template <class OPERATOR_T>
  builder<OPERATOR_T>::builder() {
    in_data_sampler_ = 0;
    in_geo_xform_ = 0;

    diamond_graph_ = 0;
    
    arg_patch_dim_=32;
    arg_tolerance_=0.0;
    arg_use_amax_error_=false;
    arg_min_sample_spacing_=0.0;
    arg_data_scale_factor_=1.0;
    arg_tmp_dir_="";
    arg_reuse_graph_ = false;
    arg_keep_graph_ = false;

    in_data_scale_factor_ = 1.0;

    arg_output_fname_="";

    build_aborted_ = false;
    build_current_level_=0;
    build_tmp_expected_average_record_size_ = 0;
    build_out_expected_average_record_size_ = 0;
  }
  

  template <class OPERATOR_T>
  builder<OPERATOR_T>::~builder() {
    // FIXME
  }

  template <class OPERATOR_T>
  void builder<OPERATOR_T>::build(const std::string& output_fname,
                                      geo_map_sampler_t* input_sampler,
				      geo_xform_t*       geo_xform) {
    arg_output_fname_ = output_fname;
    in_data_sampler_ = input_sampler;
    in_geo_xform_ = geo_xform;
    
    in_data_scale_factor_ = arg_data_scale_factor_ / double(in_data_sampler_->unit_scale());
    main_build();
  }
  
  template <class OPERATOR_T>
  bool builder<OPERATOR_T>::last_build_successful() const {
    return !build_aborted_;
  }
  
  template <class OPERATOR_T>
  std::string builder<OPERATOR_T>::tmp_file_name(std::size_t level) const {
    std::string result = arg_output_fname_ + "_tmp_";
    if (!arg_tmp_dir_.empty()) {
      result = arg_tmp_dir_ + "/" + sl::pathname_base(result);
    }
    result = result + ((level%2)?"odd":"even");
    return result;
  }

  template <class OPERATOR_T>
  std::string builder<OPERATOR_T>::tmp_graph_name() const {
    std::string result = arg_output_fname_ + "_tmpgraph_";
    if (!arg_tmp_dir_.empty()) {
      result = arg_tmp_dir_ + "/" + sl::pathname_base(result);
    }
    return result;
  }
  
  template <class OPERATOR_T>
  std::size_t builder<OPERATOR_T>::tmp_expected_average_record_size() const {
    return
      (build_tmp_expected_average_record_size_) ?
      (build_tmp_expected_average_record_size_) :
      (((arg_patch_dim_+1)*(arg_patch_dim_+1))*sizeof(value_t)/2);
  }
  
  template <class OPERATOR_T>
  std::size_t builder<OPERATOR_T>::out_expected_average_record_size() const {
    return
      (build_out_expected_average_record_size_) ?
      (build_out_expected_average_record_size_) :
      (((arg_patch_dim_)*(arg_patch_dim_))*sizeof(value_t)/2);
  }
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::tmp_open_read(std::size_t level) {
    diamond_repository_t& repo = tmp_filtered_repo_[level%2];
    if (repo.is_open()) repo.close();
    repo.open_read(tmp_file_name(level).c_str());
  }

  template <class OPERATOR_T>
  void builder<OPERATOR_T>::tmp_open_write(std::size_t level) {
    diamond_repository_t& repo = tmp_filtered_repo_[level%2];

    if (repo.is_open()) repo.close();

    repo.set_coordinate_transform(in_geo_xform_); // FIXME update repo!!

    repo.set_patch_dim(arg_patch_dim_+1); // Filtered repo stores (N+1 x N+1) values
    //    repo.set_mono_scale(false);
    repo.set_height_scale_factor(in_data_scale_factor_); // FIXME?
    repo.set_srs(arg_srs_);
    repo.set_about(arg_about_);
    repo.open_write(tmp_file_name(level).c_str(), tmp_expected_average_record_size());
  }
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::tmp_close(std::size_t level) {
    diamond_repository_t& repo = tmp_filtered_repo_[level%2];

    if (repo.is_open()) repo.close();
  }

  // =========================== MAIN COORDINATOR PROCESS

  template <class OPERATOR_T>
  void builder<OPERATOR_T>::main_build() {
    main_build_begin();
    main_build_graph();
    main_build_estimate_bitrates_and_open_repositories();
    main_build_levels_fine_to_coarse();
    main_build_end();
  }
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::main_build_begin() {
    build_aborted_ = false;

    build_tmp_expected_average_record_size_ = 0;
    build_out_expected_average_record_size_ = 0;

    main_progress_report_begin();
    
    std::cerr << "Opening output and tmp repositories..." << std::endl;

    /*
     * Initialize diamond graph
     */
    if (diamond_graph_) {
      delete diamond_graph_;
    }
    diamond_graph_ = 0;

    /*
     * Parameterize and open repos
     */
    out_compressed_repo_.set_coordinate_transform(in_geo_xform_);
    out_compressed_repo_.set_patch_dim(arg_patch_dim_);
    out_compressed_repo_.set_srs(arg_srs_);
    out_compressed_repo_.set_about(arg_about_);
    out_compressed_repo_.set_height_scale_factor(in_data_scale_factor_);
    std::cerr << "SET HEIGHT SCALE FACTOR " << in_data_scale_factor_ << std::endl;
    out_compressed_repo_.open_write(arg_output_fname_.c_str(),
                                    out_expected_average_record_size());
    if (!out_compressed_repo_.is_open()) {
      std::cerr << "Unable to open output repository: " << arg_output_fname_ << ". Aborting..." << std::endl;
      build_aborted_ = true;
    }
        
    for (std::size_t i=0; (!build_aborted_)&&(i<2); ++i) {
      tmp_open_write(i);
      if (!tmp_filtered_repo_[i].is_open()) {
        std::cerr << "Unable to open tmp repository: " << tmp_filtered_repo_[i].file_name() << ". Aborting..." << std::endl;
        build_aborted_ = true;
      } else {
        tmp_close(i);
      }
    }      
  }
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::main_build_end() {
    main_progress_report_end();

    std::cerr << "Deleting diamond graph structure..." << std::endl;
    if (diamond_graph_) {
      delete diamond_graph_;
    }
    diamond_graph_ = 0;

    // Close all repos
    std::cerr << "Closing all open repositories..." << std::endl;
    if (out_compressed_repo_.is_open()) out_compressed_repo_.close();
    if (tmp_filtered_repo_[0].is_open()) tmp_filtered_repo_[0].close();
    if (tmp_filtered_repo_[1].is_open()) tmp_filtered_repo_[1].close();

    // Delete tmp repos
    std::cerr << "Deleting temporary repos..." << std::endl;
    tmp_filtered_repo_[0].delete_all_files();
    tmp_filtered_repo_[1].delete_all_files();
    
    // Close output repo, delete whatever tmp data remains
    if (build_aborted_) {
      std::cerr << "ABORTING: Deleting output repo...." << std::endl;
      out_compressed_repo_.delete_all_files();
    }

    std::cerr << "======== FINISHED." << std::endl;
  }
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::main_build_graph() {
    if (build_aborted_) return;
    
    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    std::cerr << "Building diamond graph" << std::endl;
    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    
    if (diamond_graph_) {
      delete diamond_graph_;
    }
    diamond_graph_ = 0;

    diamond_graph_builder_t gb;
    diamond_graph_ = gb.new_diamond_graph(tmp_graph_name(),
                                          in_data_sampler_,
					  in_geo_xform_,
					  arg_patch_dim_, 
					  arg_min_sample_spacing_,
					  arg_reuse_graph_,
					  arg_keep_graph_);
    if (arg_keep_graph_) {
      std::cerr << "     save diamond graph" << std::endl;     
      //      diamond_graph_->save();
      delete diamond_graph_;
      diamond_graph_ = gb.new_diamond_graph(tmp_graph_name(),
					    in_data_sampler_,
					    in_geo_xform_,
					    arg_patch_dim_, 
					    arg_min_sample_spacing_,
					    true,
					    true);

    }

    const std::size_t N = arg_patch_dim_;

    std::cerr << "Graph construction finished" << std::endl;
    std::cerr << "Graph contains " << sl::human_readable_quantity(diamond_graph_->diamond_count()) << " diamonds (" << sl::human_readable_quantity(diamond_graph_->diamond_count()*N*N) << " samples)" << std::endl;
    for (std::size_t level=0; level<diamond_graph_->level_count(); ++level) {
      std::cerr <<
        "Level " << level << "/" << (diamond_graph_->level_count()) << ": " << sl::human_readable_quantity(diamond_graph_->level_diamond_count(level)) << " diamonds" << std::endl;
    }
    std::cerr << std::endl;

    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    std::cerr << "TIME: " << sl::human_readable_duration(stats_clock_.elapsed()) << std::endl;
    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    stats_clock_.restart();
  }

    
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::main_build_estimate_bitrates_and_open_repositories() {
    if (build_aborted_) return;

    assert(diamond_graph_);
    
    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    std::cerr << "Estimating bitrates" << std::endl;
    std::cerr << "---------------------------------------------------------------------------" << std::endl;

    // Last level, has only leafs
    const std::size_t level =  diamond_graph_->level_count()-1;
    const std::size_t level_diamond_count = diamond_graph_->level_diamond_count(level);
    const std::size_t approx_desired_sampled_leaf_count = std::min(std::size_t(50), level_diamond_count);
    const std::size_t sampled_leaf_interval = level_diamond_count/approx_desired_sampled_leaf_count;
    
    const std::size_t N = arg_patch_dim_;
    //const std::size_t N_times_2 = 2*N;

    std::size_t sampled_leafs_count = 0;
    std::size_t sampled_inners_count = 0;
    std::size_t sampled_leafs_l_size = 0;
    std::size_t sampled_leafs_h_size = 0;
    std::size_t sampled_inners_l_size = 0;
    std::size_t sampled_inners_h_size = 0;
    std::size_t cdiamond_count = 0;
    std::cerr << std::endl << "Sampling last level leafs..." << std::endl;
    for (diamond_graph_t::grid_diamond_map_const_iterator_t cdiamond_it = diamond_graph_->level_begin(level);
         (!build_aborted_) && (cdiamond_it != diamond_graph_->level_end(level));
         ++cdiamond_it) {
      if ((cdiamond_count % sampled_leaf_interval) == 0) {
        const diamond_t         x    = cdiamond_it->first;

        array2_t p(N+1, N+1);
        array2_t q(N, N);
        worker_sample_input_in(p, q, x);
        
        array2_t l(N+1, N+1);
        array2_t h(N, N);
        this->diamond_operator_.analysis_in(l, h, p, q);
        
        data_buffer_t compressed_l_as_leaf;
        data_buffer_t compressed_h_as_leaf;
        tmp_filtered_repo_[0].compress_to_target_error(l,
                                                       compressed_l_as_leaf,
                                                       nearly_lossless_tolerance_traits<value_t>::value(arg_tolerance_)/in_data_scale_factor_,
                                                       true);
        out_compressed_repo_.compress_to_target_error(h,
                                                      compressed_h_as_leaf,
                                                      arg_tolerance_/in_data_scale_factor_,
                                                      true);
        ++sampled_leafs_count;
        sampled_leafs_l_size += compressed_l_as_leaf.size();
        sampled_leafs_h_size += compressed_h_as_leaf.size();
        
        data_buffer_t compressed_l_as_inner;
        data_buffer_t compressed_h_as_inner;
        tmp_filtered_repo_[0].compress_to_target_error(l,
                                                       compressed_l_as_inner,
                                                       0.0,
                                                       true);
        out_compressed_repo_.compress_to_target_error(h,
                                                      compressed_h_as_inner,
                                                      arg_use_amax_error_ ? 0.0 : arg_tolerance_/in_data_scale_factor_,
                                                      true);
        ++sampled_inners_count;
        sampled_inners_l_size += compressed_l_as_inner.size();
        sampled_inners_h_size += compressed_h_as_inner.size();

	std::cerr << " [" << sampled_inners_count << "] last level diamonds sampled\r";
      }
      ++cdiamond_count;
    }
    std::cerr << std::endl << "Done - Estimating sizes." << std::endl;
    
    // Empty caches... 
    in_data_sampler_->minimize_footprint();
 
    // Estimate average leaf/inner node count

    const double average_l_leaf_size = double(sampled_leafs_l_size)/double(sampled_leafs_count);
    const double average_h_leaf_size = double(sampled_leafs_h_size)/double(sampled_leafs_count);
    const double average_l_inner_size = double(sampled_inners_l_size)/double(sampled_inners_count);
    const double average_h_inner_size = double(sampled_inners_h_size)/double(sampled_inners_count);

    const std::size_t diamond_count = diamond_graph_->diamond_count();
    const std::size_t diamond_leaf_count = diamond_count/2;
    const std::size_t diamond_inner_count = diamond_count - diamond_leaf_count;

    const sl::uint64_t estimated_sample_count = N*N*uint64_t(diamond_count);
    const sl::uint64_t estimated_output_size = sl::uint64_t(double(diamond_leaf_count)*average_h_leaf_size +
                                                            double(diamond_inner_count)*average_h_inner_size);
    const sl::uint64_t estimated_maxtmp_size = sl::uint64_t(double(diamond_leaf_count)*average_l_leaf_size +
                                                            double(0.5*diamond_inner_count)*average_l_inner_size);

    const float db_overhead_factor = 1.5f;
    const float estimated_output_bps = 8.0f*float(estimated_output_size)/float(estimated_sample_count);
    const float estimated_output_db_bps = db_overhead_factor * (8.0f*sizeof(diamond_id_t)/float(N*N) + estimated_output_bps);

    const float estimated_maxtmp_bps = 8.0f*float(estimated_maxtmp_size)/float(estimated_sample_count);
    const float estimated_maxtmp_db_bps = db_overhead_factor * (8.0f*sizeof(diamond_id_t)/float(N*N) + estimated_maxtmp_bps);
    
    char bit_rate_msg[64];
    sprintf(bit_rate_msg, "%.2fbps (data)   %.2fbps (dbrepo)", estimated_output_bps, estimated_output_db_bps);
    std::cerr << std::endl;
    std::cerr << "  Estimated output bit rates : " << bit_rate_msg << std::endl;
    std::cerr << "  Estimated output sizes     : " <<
      sl::human_readable_size(std::size_t(estimated_output_bps/8.0*estimated_sample_count)) << " (data)  " <<
      sl::human_readable_size(std::size_t(estimated_output_db_bps/8.0*estimated_sample_count)) << " (dbrepo)  " <<
      std::endl;
    
    sprintf(bit_rate_msg, "%.2fbps (data)   %.2fbps (dbrepo)", estimated_maxtmp_bps, estimated_maxtmp_db_bps);
    std::cerr << "  Estimated max tmp bit rates: " << bit_rate_msg << std::endl;
    std::cerr << "  Estimated max tmp sizes    : " <<
      sl::human_readable_size(std::size_t(estimated_maxtmp_bps/8.0*estimated_sample_count)) << " (data)  " <<
      sl::human_readable_size(std::size_t(estimated_maxtmp_db_bps/8.0*estimated_sample_count)) << " (dbrepo)  " <<
      std::endl;

    // Update record size estimation from inner nodes (which are larget than leafs)
    build_out_expected_average_record_size_ = std::size_t(average_h_inner_size+0.5);
    build_tmp_expected_average_record_size_ = std::size_t(average_l_inner_size+0.5);

    std::cerr << std::endl;
    std::cerr << "  Estimated db record sizes  : " <<
      sl::human_readable_size(build_out_expected_average_record_size_) << " (out)  " <<
      sl::human_readable_size(build_tmp_expected_average_record_size_) << " (tmp)  " <<
      std::endl;
    
    // Reopen output repositories with newly estimated record sizes
    if (out_compressed_repo_.is_open()) out_compressed_repo_.close();
    out_compressed_repo_.open_write(arg_output_fname_.c_str(),
                                    out_expected_average_record_size());

    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    std::cerr << "TIME: " << sl::human_readable_duration(stats_clock_.elapsed()) << std::endl;
    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    stats_clock_.restart();
  }
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::main_build_levels_fine_to_coarse() {
    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    std::cerr << "Building levels fine to coarse" << std::endl;
    std::cerr << "---------------------------------------------------------------------------" << std::endl;

    for (std::size_t x_level=0; x_level<diamond_graph_->level_count(); ++x_level) {
      if (build_aborted_) return;
      
      std::size_t level = diamond_graph_->level_count()-1-x_level;

      std::cerr <<
        "-------- level = " << level << "/" << (diamond_graph_->level_count()-1) << " (" << sl::human_readable_quantity(diamond_graph_->level_diamond_count(level)) << ")" << std::endl;

      main_build_level_begin(level);
      main_build_level();
      main_build_level_end();

      std::cerr << std::endl;
    }
  }
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::main_build_level_begin(std::size_t level) {
    if (build_aborted_) return;
    
    build_current_level_ = level;

    // Open output repository of this level for writing
    tmp_open_write(build_current_level_);
    tmp_open_read(build_current_level_+1); // Just for stats or sequential build
  }
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::main_build_level_end() {
    if (build_aborted_) return;

    // Close output repository for this level
    tmp_close(build_current_level_);
    tmp_close(build_current_level_+1);

    // Reset caches, next level will start another scan
    in_data_sampler_->minimize_footprint();
  }
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::main_build_level_sequential() {
    const diamond_graph_t::grid_diamond_map_const_iterator_t cdiamond_end = this->diamond_graph_->level_end(this->build_current_level_);
    const std::size_t N= arg_patch_dim_;
    for (diamond_graph_t::grid_diamond_map_const_iterator_t cdiamond_it = diamond_graph_->level_begin(build_current_level_);
         (!build_aborted_) && (cdiamond_it != cdiamond_end);
         ++cdiamond_it) {
      const diamond_t         x            = cdiamond_it->first;
      const diamond_state_t   x_state      = cdiamond_it->second;
      const diamond_id_t      x_id         = x.id();
      const bool              x_is_leaf    = x_state.is_leaf();
      const bool              x_has_fragment0 = x_state.has_fragment(0);
      const bool              x_has_fragment1 = x_state.has_fragment(1);

      data_buffer_t x_compressed_l;
      data_buffer_t x_compressed_h;
      if (x_is_leaf) {
        worker_build_diamond_from_input_in(x_compressed_l, x_compressed_h, x);
      } else {
        worker_build_diamond_from_children_in(x_compressed_l, x_compressed_h, x, x_has_fragment0, x_has_fragment1);
      }
      
      // Update stats and report progress
      main_progress_report(x_compressed_l, x_compressed_h);
      
      // Store result and mark worker as unemployed
      out_compressed_repo_.set_data(x_id, x_compressed_h);
      if (build_current_level_ == 0) {
        // L goes into roots, we need to recompress it with output compressor
        data_buffer_t x_compressed_l_root;
        array2_t x_l_root(N+1,N+1);
        tmp_filtered_repo_[(build_current_level_+1)%2].decompress_to(x_l_root, &(x_compressed_l[0]), x_compressed_l.size());
        out_compressed_repo_.compress_to_target_error(x_l_root,
                                                      x_compressed_l_root,
                                                      0.0,
                                                      true);
        out_compressed_repo_.set_root_data(x_id, x_compressed_l_root);
      } else {
        // L goes into current writable tmp (odd or even based on level)
        tmp_filtered_repo_[build_current_level_%2].set_data(x_id, x_compressed_l);
        //std::cerr << "STORE L[" << build_current_level_ << "] <-" << x_id[0] << " " << x_id[1] << " " << x_id[2] << std::endl;
      }
    }
  }
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::main_build_level() {
    if (build_aborted_) return;

    main_build_level_sequential();
  }

  // =========================== PROGRESS REPORTING

  template<class OPERATOR_T>
  inline void builder<OPERATOR_T>::main_progress_report_begin() {
    std::cerr << std::endl;
    std::cerr << "===========================================================================" << std::endl;
    std::cerr << "Starting construction" << std::endl;
    std::cerr << "===========================================================================" << std::endl;
    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    std::cerr << "Construction parameters" << std::endl;
    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    std::cerr << "  Output file name   : " << arg_output_fname_ << std::endl;
    std::cerr << "  TMPDIR             : " << (arg_tmp_dir_.empty() ? sl::pathname_directory(arg_output_fname_) : arg_tmp_dir_) << std::endl;
    std::cerr << "  Patch dimension    : " << arg_patch_dim_ << "x" << arg_patch_dim_ << std::endl;
    std::cerr << "  Tolerance          : " << arg_tolerance_ << std::endl;
    std::cerr << "  Error control      : " << (arg_use_amax_error_ ? "AMAX" : "RMS") << std::endl;
    std::cerr << "  Min sample spacing : " << arg_min_sample_spacing_ << std::endl;
    std::cerr << "  Data scaling factor: " << arg_data_scale_factor_ << std::endl;
    std::cerr << "  Reuse graph        : " << arg_reuse_graph_ << std::endl;
    std::cerr << "  Keep graph         : " << arg_keep_graph_ << std::endl;
    
    stats_clock_.restart();
    stats_processed_diamonds_ = 0;
    stats_stored_data_size_ = 0;
    stats_max_tmp_size_ = 0;
  }

  template<class OPERATOR_T>
  inline void builder<OPERATOR_T>::main_progress_report(const data_buffer_t& x_compressed_l,
                                                            const data_buffer_t& x_compressed_h) {
    ++stats_processed_diamonds_;
    stats_stored_data_size_ += x_compressed_h.size();
    if (build_current_level_ == 0) stats_stored_data_size_ += x_compressed_l.size();
    
    const std::size_t completed_diamonds = stats_processed_diamonds_;
    const std::size_t total_diamonds     = diamond_graph_->diamond_count();
    const std::size_t report_diamonds    = std::max(total_diamonds / std::size_t(5000), std::size_t(10));
        
    if ((completed_diamonds == total_diamonds) ||
        (completed_diamonds < 100) ||
        ((completed_diamonds % report_diamonds) == 0)) {
      sl::time_duration t_current_rt   = stats_clock_.elapsed();
    
      // Update "heavy" stats and report progress
      const uint64_t    repo_size = out_compressed_repo_.size();
      const uint64_t    tmp_size  = tmp_filtered_repo_[0].size()+tmp_filtered_repo_[1].size();
      stats_max_tmp_size_ = std::max(stats_max_tmp_size_, tmp_size);
      
      const float    sample_count      = stats_processed_diamonds_ * float(arg_patch_dim_ * arg_patch_dim_); 
      const float    real_bit_rate     = stats_stored_data_size_ * 8.0f / sample_count;
      const float    bit_rate          = repo_size * 8.0f / sample_count;

      std::stringstream progress_report_strstr;
      
      progress_report_strstr << "[" << sl::human_readable_percent(100.0f*completed_diamonds/float(total_diamonds)) << "]";
      progress_report_strstr << " - tmp:" << sl::human_readable_size(tmp_size) << " - out:" << sl::human_readable_size(repo_size);
      
      char bit_rate_msg[64]; sprintf(bit_rate_msg, "%.2fbps (data) %.2fbps (dbrepo)", real_bit_rate, bit_rate);
      progress_report_strstr << " (" << bit_rate_msg << ")";
        
      sl::time_duration t_eta_rt = sl::time_duration(sl::int64_t(double(total_diamonds)*t_current_rt.as_microseconds()/double(completed_diamonds)));
      progress_report_strstr << " - T: " << sl::human_readable_duration(t_current_rt) << " ETA: " << sl::human_readable_duration(t_eta_rt);

      progress_report_strstr <<	" - Speed: " << sl::human_readable_quantity(sl::uint64_t(double(sample_count)/(0.001*double(t_current_rt.as_milliseconds())))) << " out samples/s";
      std::cerr << progress_report_strstr.str() << "      \r";
    }
  }

  template<class OPERATOR_T>
  inline void builder<OPERATOR_T>::main_progress_report_end() {
    const sl::time_duration t_current_rt   = stats_clock_.elapsed();
      
    const uint64_t    repo_size = out_compressed_repo_.size();

    const float    sample_count      = stats_processed_diamonds_ * float(arg_patch_dim_ * arg_patch_dim_); 
    const float    real_bit_rate     = stats_stored_data_size_ * 8.0f / sample_count;
    const float    bit_rate          = repo_size * 8.0f / sample_count;
    char real_bit_rate_msg[64]; sprintf(real_bit_rate_msg, "%.2fbps", real_bit_rate);
    char bit_rate_msg[64]; sprintf(bit_rate_msg, "%.2fbps", bit_rate);
    
    std::cerr << std::endl;
    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    std::cerr << "[CONSTRUCTION FINISHED]: time        = " << sl::human_readable_duration(t_current_rt) << std::endl;
    std::cerr << "                         data size   = " << sl::human_readable_size(stats_stored_data_size_) << std::endl;
    std::cerr << "                         data bps    ~ " << real_bit_rate_msg << " bits/output leaf sample" << std::endl;
    std::cerr << "                         dbrepo size = " << sl::human_readable_size(repo_size) << std::endl;
    std::cerr << "                         dbrepo bps  ~ " << bit_rate_msg << " bits/output leaf sample" << std::endl;
    std::cerr << std::endl;
    std::cerr << "                         max tmp repo size = " << sl::human_readable_size(stats_max_tmp_size_) << std::endl;
    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    std::cerr << std::endl;
  }

  // =========================== WORKER PROCESS
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::worker_build() {
    worker_build_begin();
    worker_build_process_requests();
    worker_build_end();
  }
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::worker_build_begin() {
    build_aborted_ = false;

    // Configure repo compressors
    out_compressed_repo_.set_patch_dim(arg_patch_dim_);
    //    out_compressed_repo_.set_mono_scale(true);
    tmp_filtered_repo_[0].set_patch_dim(arg_patch_dim_+1); // Filtered repo stores (N+1 x N+1) values
    //    tmp_filtered_repo_[0].set_mono_scale(false);
    tmp_filtered_repo_[1].set_patch_dim(arg_patch_dim_+1); // Filtered repo stores (N+1 x N+1) values
    //    tmp_filtered_repo_[1].set_mono_scale(false);
  }
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::worker_build_end() {
    tmp_close(0); tmp_close(1);
  }
    
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::worker_build_level_begin(std::size_t level) {
    build_current_level_ = level;
    tmp_open_read(build_current_level_+1);
  }

  template <class OPERATOR_T>
  void builder<OPERATOR_T>::worker_build_level_end() {
    tmp_close(build_current_level_+1);
  }

  template <class OPERATOR_T>
  void builder<OPERATOR_T>::worker_sample_children_in(array2_t& p,
                                                          array2_t& q,
                                                          const diamond_t& d,
                                                          bool has_fragment_0,
                                                          bool has_fragment_1) {
    const std::size_t N = arg_patch_dim_;
    const std::size_t N_times_2 = 2*N;

    assert(p.extent()[0]==N+1);
    assert(p.extent()[1]==N+1);
    assert(q.extent()[0]==N  );
    assert(q.extent()[1]==N  );
    
    diamond_repository_t& child_repo = tmp_filtered_repo_[(build_current_level_+1)%2];
    if (!child_repo.is_open()) {
      build_aborted_ = true;      
    } else {
      // Get data from children
      array2_t child_l[4];
      if (has_fragment_0) {
        assert(child_repo.has_data(d.child_id(0,0)));
        child_repo.get_data(d.child_id(0,0), child_l[0]);
        
        assert(child_repo.has_data(d.child_id(0,1)));
        child_repo.get_data(d.child_id(0,1), child_l[1]);
      }
      if (has_fragment_1) {
        assert(child_repo.has_data(d.child_id(1,0)));
        child_repo.get_data(d.child_id(1,0), child_l[2]);

        assert(child_repo.has_data(d.child_id(1,1)));
        child_repo.get_data(d.child_id(1,1), child_l[3]);
      }

      // get p, q from children
      for(int y = 1; y < int(N_times_2); y += 2) {
        for(int x = 1; x < int(N_times_2); x += 2) {
          q(y/2, x/2) = get_value_from_child(x, y, child_l);
        }
      }
      
      for(int y = 0; y <= int(N_times_2); y += 2) {
        for(int x = 0; x <= int(N_times_2); x += 2) {
          p(y/2, x/2) = get_value_from_child(x, y, child_l);
        }
      }
    }
  }

  template <class OPERATOR_T>
  void builder<OPERATOR_T>::worker_sample_input_in(array2_t& p,
                                                   array2_t& q,
                                                   const diamond_t& d) {
    const std::size_t N = arg_patch_dim_;
    //    const std::size_t N_times_2 = 2*N;

    assert(p.extent()[0]==N+1);
    assert(p.extent()[1]==N+1);
    assert(q.extent()[0]==N  );
    assert(q.extent()[1]==N  );

    const bool is_valid_fragment[2] = {
      d.is_valid_fragment(0),
      d.is_valid_fragment(1)
    };
    
    // FIXME check sampler consistency?
    assert(is_valid_fragment[0] || is_valid_fragment[1]);
    
    // sample input into input_p, input_q
    array2_t input_p(N+1,N+1);
    array2_t input_q(N,N);
    for (int patch_id=0; patch_id<2; ++patch_id) {
      // get patch data if valid
      if (is_valid_fragment[patch_id]) {
        // planar dataset: interpolate over corners
	const grid_point_t gp0 = d.corner((1+2*patch_id)%4);
	const grid_point_t gp1 = d.corner((2+2*patch_id)%4);
	const grid_point_t gp2 = d.corner((0+2*patch_id)%4);
	//	std::cerr << "sampling d " << d.id() << ", " << patch_id << ": " << gp0 << " : " << gp1 << " : " << gp2 << std::endl;
	
        const point3d_t  dgp0     = point3d_t(gp0[0], gp0[1], gp0[2]);
        const vector3d_t dgp0dgp1 = point3d_t(gp1[0], gp1[1], gp1[2]) - dgp0;
        const vector3d_t dgp0dgp2 = point3d_t(gp2[0], gp2[1], gp2[2]) - dgp0;
        
        const double inv_patch_dim = 1.0/float(N);
        
        // get control points
        for(int y = 0; y <= int(N); ++y) {
          int yy_p = patch_id ? int(N)-y : y;
          int yy_q = patch_id ? int(N-1)-y : y;
          for(int x = 0; x <= int(N) - y; ++x) {
            int xx_p = patch_id ? int(N)-x : x;
            int xx_q = patch_id ? int(N-1)-x : x;
            
	    // P
	    point3d_t dgp_p = (dgp0 + dgp0dgp1 * inv_patch_dim * (x     ) + dgp0dgp2 * inv_patch_dim * (y     ));
	    grid_point_t gp_p = grid_point_t(int32_t(dgp_p[0]), int32_t(dgp_p[1]), int32_t(dgp_p[2]));
	    point2d_t uv = in_geo_xform_->uv_from_grid(gp_p);
	    input_p(yy_p,xx_p) = in_data_sampler_->value_at(uv[0], uv[1]);

	    // Q
	    if ((y<int(N)) && (x<int(N)-y)) {
	      point3d_t dgp_q = (dgp0 + dgp0dgp1 * inv_patch_dim * (x+0.5f) + dgp0dgp2 * inv_patch_dim * (y+0.5f));
	      grid_point_t gp_q = grid_point_t(int32_t(dgp_q[0]), int32_t(dgp_q[1]), int32_t(dgp_q[2]));
	      point2d_t uv = in_geo_xform_->uv_from_grid(gp_q);
	      input_q(yy_q,xx_q) = in_data_sampler_->value_at(uv[0], uv[1]);
	    }

	    // Mirroring?
            if ((x != y) && !is_valid_fragment[1-patch_id]) {
              // The other patch is invalid, create reasonable values by
              // mirroring along  diagonal
              input_p(N-xx_p,N-yy_p) = input_p(yy_p,xx_p);
              if ((y<int(N))&&(x<int(N)-y)) input_q(N-1-xx_q,N-1-yy_q)=input_q(yy_q,xx_q);
            }
          }
        } 
      }
    }
    
    // Decorrelate data and return it
    diamond_operator_.decorrelate_channels(p, input_p);
    diamond_operator_.decorrelate_channels(q, input_q);
  }

  template <class OPERATOR_T>
  void builder<OPERATOR_T>::worker_build_diamond_from_children_in(data_buffer_t& compressed_l,
                                                                      data_buffer_t& compressed_h,
                                                                      const diamond_t& d,
                                                                      bool has_fragment_0,
                                                                      bool has_fragment_1) {

    const std::size_t N = arg_patch_dim_;
    //const std::size_t N_times_2 = 2*N;

    array2_t p(N+1, N+1);
    array2_t q(N, N);
    worker_sample_children_in(p, q, d, has_fragment_0, has_fragment_1);

    // Wavelet analysis
    array2_t l(N+1, N+1);
    array2_t h(N, N);
    this->diamond_operator_.analysis_in(l, h, p, q);

    // L: Amax compression to 0 to avoid error propagation
    tmp_filtered_repo_[build_current_level_%2].compress_to_target_error(l,
                                                                        compressed_l,
                                                                        0.0, 
                                                                        true);

    // H: Amax compression to 0 for amax, to tol for rms
    out_compressed_repo_.compress_to_target_error(h,
                                                  compressed_h,
                                                  arg_use_amax_error_ ? 0.0 : arg_tolerance_/in_data_scale_factor_,
                                                  true);
  }
  
  template <class OPERATOR_T>
  void builder<OPERATOR_T>::worker_build_diamond_from_input_in(data_buffer_t& compressed_l,
							       data_buffer_t& compressed_h,
							       const diamond_t& d) {


    const std::size_t N = arg_patch_dim_;
    //const std::size_t N_times_2 = 2*N;
    
    array2_t p(N+1, N+1);
    array2_t q(N, N);
    this->worker_sample_input_in(p, q, d);
      
    // Wavelet analysis
    array2_t l(N+1, N+1);
    array2_t h(N, N);
    this->diamond_operator_.analysis_in(l, h, p, q);

    // L: Amax compression to nearly lossless tolerance...
    tmp_filtered_repo_[build_current_level_%2].compress_to_target_error(l,
                                                                        compressed_l,
                                                                        nearly_lossless_tolerance_traits<value_t>::value(arg_tolerance_)/in_data_scale_factor_,
                                                                        true);

    // H: Amax compression to tolerance for both amax and rms
    out_compressed_repo_.compress_to_target_error(h,
                                                  compressed_h,
                                                  arg_tolerance_/in_data_scale_factor_,
                                                  true);
  }
    
  template<class OPERATOR_T>
  inline void builder<OPERATOR_T>::worker_build_process_requests() {

  }

  // =========================== Diamond accessors

  template<class OPERATOR_T>
  inline typename builder<OPERATOR_T>::value_t builder<OPERATOR_T>::get_value_from_child(int dx, int dy, 
                                                                                                 const array2_t child_l[4]) const {
    const std::size_t N = arg_patch_dim_;
    const std::size_t N_times_2 = 2*N;

    // identify in which child x,y falls
    uint32_t child_id = 0;
    bool read_from_null_child = false;

    if (dx <= int(N_times_2) - dy) {
      if (dx < dy) {
        child_id = 0;
      } else {
        child_id = 1;
      }
      // diamond does not have patches 0,1:
      read_from_null_child = child_l[0].count() == 0;
    } else {
      if (dx > dy) {
        child_id = 2;
      } else {
        child_id = 3;
      }
      // diamond does not have patches 2,3
      read_from_null_child = child_l[2].count() == 0;
    }
    if (read_from_null_child) {
      // diamond does not have requested patches : mirror along 
      // diagonal to get values from other 2 patches
      int t = dx;
      dx = N_times_2 - dy;
      dy = N_times_2 - t;
      child_id = 3 - child_id;
    }

    // transform parent coords in child coords
    int c_x=0, c_y=0;
    child_coords_from_parent_coords(dx, dy, child_id, c_x, c_y);
    c_y /= 2; c_x /= 2;

#ifndef NDEBUG
    const sl::index<2>& extent = child_l[child_id].extent();
    int h = (int)extent[0];
    int w = (int)extent[1];

    if (c_y < 0 || c_y > h-1 ||
	c_x < 0 || c_x > w-1) {
      SL_TRACE_OUT(-1) << " accessing data outside boundary: " << c_x << ", " << c_y << std::endl;
    }
    if (child_id>3) {
      SL_TRACE_OUT(-1) << " wrong child id: " << child_id << std::endl;
    }      
#endif
    return child_l[child_id](c_y, c_x);
  }

  template<class OPERATOR_T>
  inline void builder<OPERATOR_T>::child_coords_from_parent_coords(int x, int y, uint32_t child_id,
                                                                       int& c_x, int& c_y) const {
    assert(child_id<4);

    const std::size_t N = arg_patch_dim_;
    const std::size_t N_times_2 = 2*N;

    // comment corners are parent corners.
    switch(child_id) {
    case 0:     // triangle W (u goes from id to c0)
      c_x = -x + y;
      c_y = -x - y + N_times_2;
      break;
    case 1 :    // triangle N (u goes from other_corner to c2)
      c_x =  x + y; 
      c_y = -x + y + N_times_2; 
      break;
    case 2:     // triangle E (u goes from id to c2)
      c_x =  x - y;
      c_y =  x + y - N_times_2;
      break;
    case 3 :    // triangle S (u goes from other_corner to c0)
      c_x = -x - y + 2 * N_times_2;    
      c_y =  x - y + N_times_2;   
      break;
    default:
      c_x = 0;
      c_y = 0;
      SL_FAIL("Wrong child id");
      break;
    }
  }

  template<class OPERATOR_T>
  inline void builder<OPERATOR_T>::parent_coords_from_child_coords(int x, int y, uint32_t child_id,
                                                                       int& p_x, int& p_y) const {
    assert(child_id<4);
    
    const std::size_t N = arg_patch_dim_;
    const std::size_t N_times_2 = 2*N;

    switch(child_id) {
    case 0:     // T_W  
      p_x = (-x - y + N_times_2) / 2;
      p_y = ( x - y + N_times_2) / 2;
      break;
    case 1 :    // T_N  
      p_x = ( x - y + N_times_2 ) / 2;
      p_y = ( x + y - N_times_2 ) / 2;
      break;
    case 2 :    // T_E 
      p_x = ( x + y + N_times_2 ) / 2;
      p_y = (-x + y + N_times_2 ) / 2;
      break;
    case 3 :    // T_S 
      p_x = (-x + y + N_times_2 ) / 2;
      p_y = (-x - y + 3*N_times_2 ) / 2;
      break;
    default:
      p_x = 0;
      p_y = 0;
      SL_FAIL("Wrong child id");
      break;
    }
    assert(0 <= p_x && p_x <= N_times_2);
    assert(0 <= p_y && p_y <= N_times_2);
  }

}
#endif
