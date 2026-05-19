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
#include <vic/vfs/db_repository.hpp>
#include <sl/bitops.hpp>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h> 
#include <limits.h>
#include <db.h>

#include <cassert>

// ----------------------------------------------------------------------
// Helpers (os)
// ----------------------------------------------------------------------

static off_t file_size(const char*  file_name) { 
  struct stat file_stat; 
  int err = stat(file_name, &file_stat); 
  if (0 != err) return 0; 
  return file_stat.st_size; 
}

// ----------------------------------------------------------------------
// Helpers (db4+conversions)
// ----------------------------------------------------------------------

static const std::size_t Default_cache_size = 16*1024*1024;

static inline vic::vfs::db_repository::key_t as_key(const DBT *db_key) {
  vic::vfs::db_repository::key_t key;
  memcpy(&(key[0]), db_key->data, db_key->size); // copy, don't cast, for alignment purposes
  return key;
}

static inline DBT dbt_make() {
  DBT result;
  memset(&result, 0, sizeof(DBT));
  return result;
}

static inline DBT dbt_make(void* ptr, std::size_t sz) {
  DBT result;
  memset(&result, 0, sizeof(DBT));
  result.data = ptr;
  result.size = sz;
  return result;
}

static inline DBT dbt_make(const void* ptr, std::size_t sz) {
  DBT result;
  memset(&result, 0, sizeof(DBT));
  result.data = const_cast<void*>(ptr);
  result.size = sz;
  return result;
}

// ----------------------------------------------------------------------
// Helpers (sorting)
// ----------------------------------------------------------------------

static const sl::uint32_t grid_sub_max   = 28;
static const sl::int32_t  grid_coord_max = sl::int32_t(sl::int32_t(1) << grid_sub_max);
static const sl::int32_t  grid_coord_min = -grid_coord_max;

static inline sl::uint64_t hi_morton(const vic::vfs::db_repository::key_t& gp) {
  sl::uint32_t x = (-grid_coord_min + gp[0]); // Positive
  sl::uint32_t y = (-grid_coord_min + gp[1]); // Positive
  sl::uint32_t z = (-grid_coord_min + gp[2]); // Positive
  return
    sl::morton_bitops<sl::uint64_t,3>::encoded(x>>16, 0) |
    sl::morton_bitops<sl::uint64_t,3>::encoded(y>>16, 1) |
    sl::morton_bitops<sl::uint64_t,3>::encoded(z>>16, 2);
}

static inline sl::uint64_t lo_morton(const vic::vfs::db_repository::key_t& gp) {
  sl::uint32_t x = (-grid_coord_min + gp[0]); // Positive
  sl::uint32_t y = (-grid_coord_min + gp[1]); // Positive
  sl::uint32_t z = (-grid_coord_min + gp[2]); // Positive
  return
    sl::morton_bitops<sl::uint64_t,3>::encoded(x&((1<<16)-1), 0) |
    sl::morton_bitops<sl::uint64_t,3>::encoded(y&((1<<16)-1), 1) |
    sl::morton_bitops<sl::uint64_t,3>::encoded(z&((1<<16)-1), 2);
}

static inline int morton_cmp(const vic::vfs::db_repository::key_t& x, 
			     const vic::vfs::db_repository::key_t& y) {
  const sl::uint64_t hi_x = hi_morton(x);
  const sl::uint64_t hi_y = hi_morton(y);
  if (hi_x == hi_y) {
    const sl::uint64_t lo_x = lo_morton(x);
    const sl::uint64_t lo_y = lo_morton(y);
    if (lo_x == lo_y) {
      return  0;
    } else if (lo_x < lo_y) {
      return -1;
    } else {
      return  1;
    }
  } else if (hi_x<hi_y) {
    return -1;
  } else { 
    return 1;
  }
}

static inline int key_compare(DB*, const DBT *key1, const DBT *key2) {
  const vic::vfs::db_repository::key_t k1 = as_key(key1);
  const vic::vfs::db_repository::key_t k2 = as_key(key2);

#if 0
  if (k1 < k2) {
    return -1;
  } else if (k1 == k2) {
    return 0;
  } else {
    return 1;
  }
#else
  return morton_cmp(k1, k2);
#endif
}

// ----------------------------------------------------------------------
// Class implementation
// ----------------------------------------------------------------------

namespace vic {
  namespace vfs {

    db_repository::db_repository() {
      write_mode_ = false;
      is_temporary_ = false;
      db_ = NULL;
    }

    db_repository::~db_repository() {
      close();
    }
      
    void db_repository::open_read(const std::string& filename) {
      close();
      file_name_ = filename;
      write_mode_ = false;
      is_temporary_ = false;

      int ret = db_create(&db_, NULL, 0);
      if (ret != 0) {
	std::cerr << "unable to create db " << 	db_strerror(ret) << std::endl;
	return;
      }
      db_->set_bt_compare(db_, key_compare);
      db_->set_cachesize(db_, 0, Default_cache_size, 0);
      ret = db_->open(db_,
		      NULL,
		      filename.c_str(), 
		      NULL,
		      DB_BTREE, 
		      DB_RDONLY,
		      0);
      if (ret) {
	std::cerr << "unable to open db " << filename << " for reading " << db_strerror(ret) << std::endl;
	db_ = NULL;
      }
    }

    void db_repository::open_write(const std::string& filename, 
				   std::size_t expected_average_data_size,
				   bool is_temporary) {
      close();
      file_name_ = filename;
      write_mode_ = true;
      is_temporary_ = is_temporary;

      int ret = db_create(&db_, NULL, 0);
      if (ret != 0) {
	std::cerr << "unable to create db " << 	db_strerror(ret) << std::endl;
	return;
      }
      const std::size_t min_db_page_size =  4096;
      const std::size_t max_db_page_size = 65536;

      std::size_t db_page_size = min_db_page_size;
      std::size_t chunk_size = (4+2) * expected_average_data_size;
      while (db_page_size < chunk_size && db_page_size < max_db_page_size) db_page_size *= 2;
      if (db_page_size > max_db_page_size) db_page_size = max_db_page_size;

      //      std::cerr << "Open write db '" << filename << "' with page size = " << db_page_size << std::endl;

      db_->set_pagesize(db_, db_page_size);
      db_->set_bt_compare(db_, key_compare);
      ret = db_->open(db_,
		      NULL,
		      filename.c_str(), 
		      NULL,
		      DB_BTREE, 
		      DB_CREATE | DB_TRUNCATE,
		      0);
      if (ret) {
	std::cerr << "unable to open db " << filename << " for writing " << db_strerror(ret) << std::endl;
	db_ = NULL;
      }
    }

    void db_repository::close() {
      if (db_ != NULL) {
	db_->close(db_,0);
#if 0
	// FIXME: remove does not work
	if (is_temporary_) {
	  int ret = db_->remove(db_,
				file_name_.c_str(),
				NULL,
				0);
	  if (ret) {
	    std::cerr << "error removing db " <<  db_strerror(ret) << " " << file_name_ << std::endl;
	  }
	}
#endif
	db_ = NULL;
      } 
    }

    void db_repository::set_data(const key_t& key, const uint8_t* data_buffer, uint32_t size) {
      assert(write_mode());
      assert(is_open());

      DBT db_key  = dbt_make(&(key[0]), sizeof(key_t));
      DBT db_data = dbt_make(data_buffer, size);
      db_->put(db_, NULL, &db_key, &db_data, 0);
    }

    const sl::uint8_t* db_repository::get_data(const key_t& key, uint32_t& size) const {
      assert(is_open());
      //   int (*get)(const DB *db, DBT *key, DBT *data, u_int flags);
      DBT db_key  = dbt_make(&(key[0]), sizeof(key_t));
      DBT db_data = dbt_make();
      
      int res = db_->get(db_, NULL, &db_key, &db_data, 0);
      if (res == 0) {
	size = db_data.size;
	return (sl::uint8_t*)(db_data.data);
      } else if (res == -1) {
	std::cerr << "db_repository::error unable to read data at " << key << std::endl;
	return 0;
      } else {
	// Data is missing
	//	std::cerr << "db_repository::missing data at " << key << std::endl;
	return 0;
      }
    }

    sl::uint64_t db_repository::size() const {
      if (!file_name_.empty()) {
	return file_size(file_name_.c_str());
      } else {
	return 0;
      }
    }  

    bool db_repository::has_data(const key_t& key) const {
      assert(is_open());
      //   int (*get)(const DB *db, DBT *key, DBT *data, u_int flags);
      DBT db_key = dbt_make(&(key[0]), sizeof(key_t));
      DBT db_data = dbt_make();
      int res = db_->get(db_, NULL, &db_key, &db_data, 0);
      return (res == 0); // true if found
    }
  }
}
