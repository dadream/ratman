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
#ifndef CBDAM_MPI_BUILDER_HPP
#define CBDAM_MPI_BUILDER_HPP

/////////////////////// FIXME
//#undef NDEBUG
/////////////////////// FIXME


#include <vic/mpi/mpi.hpp>
#include <vic/cbdam/base/builder.hpp>
#include <sl/os_file.hpp>

#include <unistd.h>

namespace cbdam {

  /**
   *  Parallel builder of cbdam structures based on the
   *  Message Passing Interface (MPI) standard. 
   */
  template<class OPERATOR_T>
  class mpi_builder : public builder<OPERATOR_T> {
  public:
    typedef builder<OPERATOR_T>				super_t;
    typedef typename super_t::diamond_graph_builder_t   diamond_graph_builder_t;
    typedef typename super_t::diamond_graph_t	        diamond_graph_t;
    typedef typename super_t::diamond_t                 diamond_t;
    typedef typename super_t::diamond_state_t           diamond_state_t;
    typedef typename super_t::diamond_id_t              diamond_id_t;
    
    typedef OPERATOR_T                                  operator_t;
    typedef typename operator_t::value_t                value_t;
    typedef typename sl::dense_array<value_t,2,void>    array2_t;

    typedef typename super_t::diamond_repository_t      diamond_repository_t;
    typedef typename super_t::data_buffer_t             data_buffer_t;

    typedef typename super_t::geo_map_sampler_t         geo_map_sampler_t; 
    typedef typename super_t::geo_xform_t               geo_xform_t;
   
  public:

    mpi_builder();

    virtual ~mpi_builder();


  public: // Construction

    virtual void build(const std::string& output_fname,
                       geo_map_sampler_t* input_sampler,
		       geo_xform_t*       geo_xform);

  protected: // MPI communication protocol

    enum MPI_TAG {
      MPI_TAG_DIE         = 1,
      MPI_TAG_ERROR       = 2,
      MPI_TAG_PROCNAME    = 3,
      MPI_TAG_LEVEL_BEGIN = 4,
      MPI_TAG_LEVEL_END   = 5,
      MPI_TAG_PROCESS_DIAMOND = 6,
      MPI_TAG_PROCESS_DIAMOND_RESULT = 8,
    };
    
  protected: // Main coordinator process
    
    virtual void main_build_end();

    virtual void main_build_level_begin(std::size_t level);

    virtual void main_build_level_end();

    virtual void main_build_level();

  protected:

    virtual void main_progress_report_begin();

  protected: // Worker: sample and compress

    virtual void worker_build_process_requests();
    
  }; // class mpi_builder
  
} // namespace cbdam


namespace cbdam {

  // =========================== GENERAL
  
  template <class OPERATOR_T>
  mpi_builder<OPERATOR_T>::mpi_builder() :
    super_t() {

  }

  template <class OPERATOR_T>
  mpi_builder<OPERATOR_T>::~mpi_builder() {
    // FIXME
  }

  template <class OPERATOR_T>
  void mpi_builder<OPERATOR_T>::build(const std::string& output_fname,
                                      geo_map_sampler_t* input_sampler,
				      geo_xform_t*       geo_xform) {
    this->arg_output_fname_ = output_fname;
    this->in_data_sampler_ = input_sampler;
    this->in_geo_xform_ = geo_xform;
    this->in_data_scale_factor_ = this->arg_data_scale_factor_ / double(this->in_data_sampler_->unit_scale());

    if (vic::mpi::process_rank() == 0) {
      this->main_build();
    } else {
      input_sampler->set_verbose(false);
      this->worker_build();
    }
  }
  
  template <class OPERATOR_T>
  void mpi_builder<OPERATOR_T>::main_build_end() {
    super_t::main_build_end();

    // Kill workers
    std::cerr << "Killing workers..." << std::endl;
    for (int worker=1; worker<vic::mpi::process_count(); ++worker) {
      vic::mpi::send(worker, MPI_TAG_DIE, true);
    }
  }

  template <class OPERATOR_T>
  void mpi_builder<OPERATOR_T>::main_build_level_begin(std::size_t level) {
    super_t::main_build_level_begin(level);

    if (this->build_aborted_) return;
    
    for (int worker=1; worker<vic::mpi::process_count(); ++worker) {
      vic::mpi::send(worker, MPI_TAG_LEVEL_BEGIN, this->build_current_level_);
    }
  }
  
  template <class OPERATOR_T>
  void mpi_builder<OPERATOR_T>::main_build_level_end() {
    super_t::main_build_level_end();

    // Signal workers to stop with this level (this will release repository access)
    for (int worker=1; worker<vic::mpi::process_count(); ++worker) {
      vic::mpi::send(worker, MPI_TAG_LEVEL_END, this->build_current_level_);
    }

    // Resync, just to make sure all workers did close the repositories...
    for (int worker=1; worker<vic::mpi::process_count(); ++worker) {
      std::string worker_processor_name;
      vic::mpi::send(worker, MPI_TAG_PROCNAME, vic::mpi::processor_name());
      vic::mpi::receive(worker, MPI_TAG_PROCNAME, worker_processor_name);
      SL_TRACE_OUT(1) << "Sync received from worker#" << worker << " running on " << worker_processor_name << std::endl;
    }
  }
  
  template <class OPERATOR_T>
  void mpi_builder<OPERATOR_T>::main_build_level() {
    if (this->build_aborted_) return;

    const std::size_t process_count = vic::mpi::process_count();
    if (process_count < 2) {
      this->main_build_level_sequential();
      return;
    } 

    std::vector< std::list< std::pair<diamond_t, diamond_state_t> > > worker_diamonds(process_count);
    
    const std::size_t N= this->arg_patch_dim_;

    const typename diamond_graph_t::grid_diamond_map_const_iterator_t cdiamond_end = this->diamond_graph_->level_end(this->build_current_level_);
    const std::size_t cdiamond_count = this->diamond_graph_->level_diamond_count(this->build_current_level_);
      
    typename diamond_graph_t::grid_diamond_map_const_iterator_t cdiamond_it = this->diamond_graph_->level_begin(this->build_current_level_);
    std::size_t cdiamond_send_count = 0;
    std::size_t cdiamond_receive_count = 0;

    int next_unseeded_worker = 1;
    /*
     * Process all diamonds in this level
     */
    while ((!this->build_aborted_) &&
           (cdiamond_receive_count < cdiamond_count)) {
      /*
       * Process oustanding work requests and determine
       * which worker will be used next and how many 
       * requests will be sent to it
       */
      int unemployed_worker = 0;
      std::size_t unemployed_worker_batch_count = 1;
      if (next_unseeded_worker<int(process_count)) {
        // A worker is still unemployed, use it and send him
	// a number of requests to init pipeline
        unemployed_worker_batch_count = 8; // FIXME DOES NOT WORK IF > 1 !!!!!
	unemployed_worker = next_unseeded_worker;
        ++next_unseeded_worker;
      } else {
        // All workers are employed, receive work result from one of them,
        // and mark it unemployed.
        int worker;
        int tag;
	SL_TRACE_OUT(1) << "Waiting for msg" << std::endl;
        sl::tie(worker,tag) = vic::mpi::probe(); // FIXME - handle timeouts?
	SL_TRACE_OUT(1) << "Got worker=" << worker << " tag=" << tag << std::endl;
        switch (tag) {
        case MPI_TAG_ERROR: {
          std::string msg;
          vic::mpi::receive(worker, tag, msg);
          // FIXME: Signal error
          std::cerr << "Worker#" << worker << " signals an error, construction process will abort: " << msg << std::endl;
          this->build_aborted_ = true;
        } break;
        case MPI_TAG_PROCESS_DIAMOND_RESULT: {
          diamond_id_t  x_id;
          data_buffer_t x_compressed_l;
          data_buffer_t x_compressed_h;
          vic::mpi::receive(worker, tag, x_id, x_compressed_l, x_compressed_h);
          ++cdiamond_receive_count;
	
	  SL_TRACE_OUT(1) << "Diamond received." << std::endl;

          // Update stats and report progress

          this->main_progress_report(x_compressed_l, x_compressed_h);
          
          // Store result and mark worker as unemployed
          this->out_compressed_repo_.set_data(x_id, x_compressed_h);
          if (this->build_current_level_ == 0) {
            data_buffer_t x_compressed_l_root;
            array2_t x_l_root(N+1,N+1);
            this->tmp_filtered_repo_[(this->build_current_level_+1)%2].decompress_to(x_l_root, &(x_compressed_l[0]), x_compressed_l.size());
            this->out_compressed_repo_.compress_to_target_error(x_l_root,
								x_compressed_l_root,
								0.0,
								true);
            this->out_compressed_repo_.set_root_data(x_id, x_compressed_l_root);
          } else {
            // L goes into current writable tmp (odd or even based on level)
            this->tmp_filtered_repo_[this->build_current_level_%2].set_data(x_id, x_compressed_l);

          }
          unemployed_worker = worker;
        } break;
        default: {
          // FIXME: Signal error
          this->build_aborted_ = true;
          std::cerr << "Worker#" << worker << "sent unknown message, construction process will abort: msg tag=" << tag << std::endl;
        } break;          
        }
      }

      assert(this->build_aborted_ ||
             ((unemployed_worker>0) && (unemployed_worker<int(process_count))));

      SL_TRACE_OUT(1) << "Send batch." << std::endl;
      
      /*
       * If work requests have not been exhausted, dispatch next
       * unemployed_worker_batch_count ones to the unemployed worker.
       */
      std::size_t batch_send_count = 0;
      while (batch_send_count < unemployed_worker_batch_count &&
	     (!this->build_aborted_) && 
	     (cdiamond_send_count < cdiamond_count)) {
	
	// Fill unemployed worker's list of diamonds to send
	// (group diamonds in batches to improve locality of
	// access
	if (worker_diamonds[unemployed_worker].empty()) {
	  const std::size_t PREFETCH_COUNT = 64; // FIXME
	  while ((worker_diamonds[unemployed_worker].size() < PREFETCH_COUNT) &&
		 (cdiamond_it != cdiamond_end)) {
	    const diamond_t         x            = cdiamond_it->first;
	    const diamond_state_t   x_state      = cdiamond_it->second;
	    
	    worker_diamonds[unemployed_worker].push_back(std::make_pair(x,x_state));
	    ++cdiamond_it;
	  }
	}

	// Detect active queue - By default worker's one
	std::size_t active_queue_index = unemployed_worker;
	if (worker_diamonds[active_queue_index].empty()) {
	  // Worker has finished, pop from another worker's queue!
	  ++active_queue_index;
	  if (active_queue_index >= process_count) active_queue_index = 1;
	  while (worker_diamonds[active_queue_index].empty() &&
		 active_queue_index != std::size_t(unemployed_worker)) {
	    ++active_queue_index;
	    if (active_queue_index >= process_count) active_queue_index = 1;
	  }
	}
	
	// Send out a diamond from active queue
	if (worker_diamonds[active_queue_index].empty()) {
	  SL_TRACE_OUT(-1) << "Attempting to pop from empty queues???" << std::endl;
	  this->build_aborted_ = true;
	} else {
	  const diamond_t         x            = worker_diamonds[active_queue_index].front().first;
	  const diamond_state_t   x_state      = worker_diamonds[active_queue_index].front().second;
	  const diamond_id_t      x_id         = x.id();
	  const bool              x_is_leaf    = x_state.is_leaf();
	  const bool              x_has_fragment0 = x_state.has_fragment(0);
	  const bool              x_has_fragment1 = x_state.has_fragment(1);
	  worker_diamonds[active_queue_index].pop_front();
	  
	  SL_TRACE_OUT(1) << "Sending x " << x_id << " to worker " << unemployed_worker << " is leaf " << x_is_leaf 
			  << " has 0 " << x_has_fragment0 << " has 1 " << x_has_fragment1 << std::endl;
	  vic::mpi::send(unemployed_worker,
			 MPI_TAG_PROCESS_DIAMOND,
			 x,
			 x_is_leaf,
			 x_has_fragment0,
			 x_has_fragment1);
	  SL_TRACE_OUT(1) << "Sent x." << std::endl;
	  ++cdiamond_send_count;
	  ++batch_send_count;
	}
      } // while batch_count
    } // while !done
  }

  // =========================== PROGRESS REPORTING

  template<class OPERATOR_T>
  inline void mpi_builder<OPERATOR_T>::main_progress_report_begin() {
    super_t::main_progress_report_begin();

    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    std::cerr << "MPI Configuration: " << vic::mpi::process_count() << " processes" << std::endl;
    std::cerr << "---------------------------------------------------------------------------" << std::endl;
    std::cerr << "  Coordinator: " << vic::mpi::processor_name() << std::endl;
    
    for (int worker=1; worker<vic::mpi::process_count(); ++worker) {
      std::string worker_processor_name;
      vic::mpi::send(worker, MPI_TAG_PROCNAME, vic::mpi::processor_name());
      vic::mpi::receive(worker, MPI_TAG_PROCNAME, worker_processor_name);
      std::cerr << "  Worker#" << worker << ": " << worker_processor_name << std::endl;
    }
  }

  // =========================== WORKER PROCESS

  template <class OPERATOR_T>
  void mpi_builder<OPERATOR_T>::worker_build_process_requests() {
    bool running = true;
    while (running && !this->build_aborted_) {
      SL_TRACE_OUT(1) << "WORKER # " << vic::mpi::process_rank() << " waiting for command..." << std::endl;
      int src;
      int tag;
      sl::tie(src,tag) = vic::mpi::probe(MPI_ANY_SOURCE); // FIXME - handle timeouts?
      switch (tag) {

      case MPI_TAG_DIE: {
        SL_TRACE_OUT(1) << "WORKER # " << vic::mpi::process_rank() << " processing DIE command..." << std::endl;
        bool x;
        vic::mpi::receive(src, tag, x);
        running = false;
      } break;

      case MPI_TAG_PROCNAME: {
        SL_TRACE_OUT(1) << "WORKER # " << vic::mpi::process_rank() << " processing PROCNAME command..." << std::endl;
        std::string master_procname;
        vic::mpi::receive(src, tag, master_procname);
        vic::mpi::send(src, tag, vic::mpi::processor_name());
      } break;

      case MPI_TAG_LEVEL_BEGIN: {
        SL_TRACE_OUT(1) << "WORKER # " << vic::mpi::process_rank() << " processing BEGIN LEVEL command..." << std::endl;
        std::size_t level;
        vic::mpi::receive(src, tag, level);
        this->worker_build_level_begin(level);
      } break;

      case MPI_TAG_LEVEL_END: {
        SL_TRACE_OUT(1) << "WORKER # " << vic::mpi::process_rank() << " processing END LEVEL command..." << std::endl;
        std::size_t level;
        vic::mpi::receive(src, tag, level);
        this->worker_build_level_end();
      } break;

      case MPI_TAG_PROCESS_DIAMOND: {
        SL_TRACE_OUT(1) << "WORKER # " << vic::mpi::process_rank() << " processing PROCESS_DIAMOND command..." << std::endl;
        diamond_t x;
        bool x_is_leaf;
        bool x_has_fragment0;
        bool x_has_fragment1;
        vic::mpi::receive(src,tag,x,x_is_leaf,x_has_fragment0,x_has_fragment1);

        data_buffer_t compressed_l;
        data_buffer_t compressed_h;
        if (x_is_leaf) {
          SL_TRACE_OUT(1) << "WORKER # " << vic::mpi::process_rank() << " building diamond " << x.id() << " from leafs..." << std::endl;
          this->worker_build_diamond_from_input_in(compressed_l, compressed_h, x);
	  if (this->build_aborted_) vic::mpi::send(0,MPI_TAG_ERROR, std::string("Unable to access input data"));
        } else {
          SL_TRACE_OUT(1) << "WORKER # " << vic::mpi::process_rank() << " building diamond " << x.id() << " from previous level..." << std::endl;
          this->worker_build_diamond_from_children_in(compressed_l, compressed_h, x, x_has_fragment0, x_has_fragment1);
	  if (this->build_aborted_) vic::mpi::send(0,MPI_TAG_ERROR, std::string("Unable to access tmp data"));
	}
        if (!this->build_aborted_) {
          SL_TRACE_OUT(1) << "WORKER # " << vic::mpi::process_rank() << " returning l=" << compressed_l.size() << "B, h= " << compressed_h.size() << "B to src " << src << ".." << std::endl;
          vic::mpi::send(src,MPI_TAG_PROCESS_DIAMOND_RESULT, x.id(), compressed_l, compressed_h);
          SL_TRACE_OUT(1) << "WORKER # " << vic::mpi::process_rank() << " returning done" << std::endl;
        } 
      } break;
        
      default: {
        SL_TRACE_OUT(-1) << "WORKER # " << vic::mpi::process_rank() << " processing UNKNOWN command. -- aborting" << std::endl;
        vic::mpi::send(src,MPI_TAG_ERROR, std::string("Received unknown command"));
        this->build_aborted_ = true;
        running = false;
      } break;

      } // switch
    } // while running && !aborted
  }
  
}
#endif
