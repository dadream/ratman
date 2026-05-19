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
#ifndef MPI_QUAD_BUILDER_HPP
#define MPI_QUAD_BUILDER_HPP

#include <vic/geo/builder/quad_builder.hpp>
#include <vic/geo/builder/geo_transform.hpp>
#include <sl/axis_aligned_box.hpp>
#include <vic/mpi/mpi.hpp>
#include <sl/clock.hpp>

namespace vic {

  namespace geo {

    class mpi_tile_chunk {
    protected:
      typedef sl::axis_aligned_box<2,int> aabox2i_t;
      typedef sl::fixed_size_point<2,int> point2i_t;
      typedef sl::fixed_size_point<3,int> point3i_t;

    protected:
      std::string   tile_fname_;
      std::size_t   tile_level_;
      geo_transform tile_warp_xform_;
      aabox2i_t     quad_box_xy_;
      bool          is_split_;

    public:

      inline mpi_tile_chunk(const std::string&   tile_fname = "",
			    const std::size_t&   tile_level = 0,
			    const geo_transform& tile_warp_xform = geo_transform(),
			    const aabox2i_t&     quad_box_xy = aabox2i_t(),
			    const bool           is_split = false) :
	tile_fname_(tile_fname),
	tile_level_(tile_level),
	tile_warp_xform_(tile_warp_xform),
	quad_box_xy_(quad_box_xy),
	is_split_(is_split) {
      }

      inline ~mpi_tile_chunk() {}

      inline const std::string&   tile_fname() const { return tile_fname_; }
      inline std::size_t          tile_level() const { return tile_level_; }
      inline const geo_transform& tile_warp_xform() const { return tile_warp_xform_; }
      inline const aabox2i_t&     quad_box_xy() const { return quad_box_xy_; }
      inline bool                 is_split() const  { return is_split_; }

      void store_to(sl::output_serializer& s) const;
      void retrieve_from(sl::input_serializer& s);
    }; 

    class mpi_quad_builder: public quad_builder {
    public:
      typedef quad_builder super_t;
      typedef mpi_quad_builder this_t;
      
    protected:
      typedef sl::axis_aligned_box<2,int> aabox2i_t;
      typedef sl::fixed_size_point<2,int> point2i_t;
      typedef sl::fixed_size_point<3,int> point3i_t;

    protected:

      sl::real_time_clock build_clock_;

    protected: // MPI communication protocol

      enum MPI_TAG {
	MPI_TAG_DIE                         = 10,
	MPI_TAG_ERROR                       = 20,
	MPI_TAG_PROCNAME                    = 30,
	MPI_TAG_GENERATE_QUADS_FROM_TILE    = 60,
	MPI_TAG_GENERATE_QUADS_FROM_CHILDREN= 70,
	MPI_TAG_SIGNAL_QUAD_DAMAGED         = 80,
	MPI_TAG_SIGNAL_WORK_DONE            = 90,
      };

      std::vector<mpi_tile_chunk> mpi_tile_chunks_;
      std::vector<aabox2i_t> mpi_main_worker_box_;
      
      void mpi_main_insert_tile_chunks(const mpi_tile_chunk& ck);

      void mpi_main_create_tile_chunks();

      void mpi_main_init_worker_boxes();

      std::size_t mpi_main_busy_worker_count() const;

      double mpi_main_worker_load() const;

      int  mpi_main_worker_for(const aabox2i_t& b) const;

      void mpi_main_assign_box(std::size_t i, const aabox2i_t& b);

      void mpi_main_mark_idle(std::size_t i);

    public:
      mpi_quad_builder();

      virtual ~mpi_quad_builder();

      virtual std::string out_quad_index_fname() const;

     public: 

      virtual void process();

      virtual void repair();

      virtual void repair_level(int l);
 
      virtual void damage(int l, int x, int y);
 
      virtual void begin_processing();

      virtual void process_tile(const std::string& tile_fname, int level= -1);
      virtual void process_directory(const std::string& dirname,
				     const std::string& pattern = std::string("*.tif"), int level= -1);

      virtual void end_processing();

     protected: 

      void mpi_main_process();
      void mpi_main_repair_level(int l);

      void mpi_worker_process_requests();

    };
  }

}

#endif
