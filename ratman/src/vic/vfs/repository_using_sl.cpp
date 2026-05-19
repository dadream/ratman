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
#include <vic/vfs/repository.hpp>
#include <vic/vfs/sl_repository.hpp>
#include <cassert>


// ----------------------------------------------------------------------
// Class implementation
// ----------------------------------------------------------------------

namespace vic {

// ----------------------------------------------------------------------
// Details
// ----------------------------------------------------------------------
  namespace detail {
    class repository {
    protected:
      vfs::sl_repository repo_;
    public:
      repository()  {}
      ~repository() {}
      const vfs::sl_repository& repo() const {return repo_;}
      vfs::sl_repository& repo() {return repo_;}
    };
  }

  namespace vfs {

    repository::repository() {
      implementation_ = new detail::repository;
    }

    repository::~repository() {
      implementation_->repo().close();
      delete implementation_;
    }
      
    void repository::open_read(const std::string& filename) {
      implementation_->repo().open_read(filename);
    }

    void repository::open_write(const std::string& filename, 
				   std::size_t expected_average_data_size,
				   bool is_temporary) {
      implementation_->repo().open_write(filename, expected_average_data_size, is_temporary);
    }

    void repository::close() {
      implementation_->repo().close();
    }

    void repository::set_data(const key_t& key, const uint8_t* data_buffer, uint32_t size) {
      implementation_->repo().set_data(key, data_buffer, size);
    }

    const sl::uint8_t* repository::get_data(const key_t& key, uint32_t& size) const {
      return implementation_->repo().get_data(key, size);
    }

    sl::uint64_t repository::size() const {
      return implementation_->repo().size();
    }  

    bool repository::has_data(const key_t& key) const {
      return implementation_->repo().has_data(key);
    }

    bool repository::is_open() const {
     return implementation_->repo().is_open();
    }

    bool repository::write_mode() const {
     return implementation_->repo().write_mode();
    }

    const std::string& repository::file_name() const {
      return implementation_->repo().file_name();
    }

    void repository::minimize_footprint() const {

    }
  }
}
