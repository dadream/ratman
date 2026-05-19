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
#ifndef CBDAM_REFERENCE_COUNTED_CACHE_HPP
#define CBDAM_REFERENCE_COUNTED_CACHE_HPP

//#undef NDEBUG
// FIXME!!!!!

#include <map>
#include <set>
#include <list>
#include <cassert>
#include <functional>

// FIXME REMOVE
#include <iostream>
#include <vic/cbdam/base/grid_point.hpp>

namespace cbdam {

  typedef std::pair<grid_point_t, int> diamond_patch_id_t;
  inline std::ostream&  operator<<(std::ostream& os, const diamond_patch_id_t& dp) {
    os << dp.first << " p_id " << dp.second << std::endl;
    return os;
  }

  class reference_counted;
  
  class reference_counted_owner {
  protected:
    mutable std::size_t release_time_stamp_;
    mutable std::size_t referenced_count_;
    mutable std::size_t not_referenced_count_;

  public:
    inline reference_counted_owner():
        release_time_stamp_(0), referenced_count_(0), not_referenced_count_(0) {
    }
    
    inline virtual ~reference_counted_owner() {}
    
    inline std::size_t release_time_stamp() const {
      return release_time_stamp_;
    }

  public:

    virtual void handle_create(const reference_counted* o);

    virtual void handle_destroy(const reference_counted* o);

    virtual void handle_last_deref(const reference_counted* o);

    virtual void handle_first_ref(const reference_counted* o);
    
  };
  
  /**
   *
   */
  class reference_counted {
  protected:
    mutable int reference_count_;
    mutable std::size_t release_time_stamp_;
    mutable reference_counted_owner* owner_;
    
  public:
    inline reference_counted(reference_counted_owner* owner) :
        reference_count_(0), release_time_stamp_(std::size_t(-1)), owner_(owner)  {
      assert(owner);
      owner_->handle_create(this);
    }
    
    inline virtual ~reference_counted() {
      assert(reference_count_ <= 0);
      owner_->handle_destroy(this);
    }

    inline void ref() const {
      ++reference_count_;
      if (reference_count_==1) {
	owner_->handle_first_ref(this);
      }
    }
    
    inline void deref() const {
      assert(reference_count_>0);
      --reference_count_;
      if (reference_count_ <= 0) {
        owner_->handle_last_deref(this);
      }
    }

    inline int use_count() const {
      return reference_count_;
    }

    inline std::size_t release_time_stamp() const {
      return release_time_stamp_;
    }
    
    inline void set_release_time_stamp(std::size_t x) const {
      release_time_stamp_ = x;
    }

  };


  template <class T> 
  class reference_counted_pointer {
  protected:
    T* pointer_;
  public:
      
    explicit inline reference_counted_pointer(T* p): 
      pointer_(p) {
      if (pointer_) pointer_->ref();
    }

    inline ~reference_counted_pointer() {
      if (pointer_) pointer_->deref();      
    }

    inline reference_counted_pointer(const reference_counted_pointer& other) : pointer_(other.raw_pointer()) {
      if (pointer_) pointer_->ref();
    }  // never throws
  
    inline reference_counted_pointer& operator=(const reference_counted_pointer& other) {
      if (pointer_ != other.raw_pointer()) {
	if (pointer_) pointer_->deref();
	pointer_ = other.raw_pointer();
	if (pointer_) pointer_->ref();
      }
      return *this;
    }
    
    inline T& operator*() const  { return *(this->pointer_); }  // never throws
    inline T* operator->() const { return (this->pointer_); }  // never throws

    /// The embedded raw pointer
    inline T* raw_pointer() const { return pointer_; }

    /// Is the pointer non void?
    inline operator bool() const { return pointer_; }

    /// The number of references to the pointed object
    inline int use_count() const { 
      return pointer_ ? pointer_->use_count() : 0;
    }

    /// Is the pointed object referenced only by one pointer()?
    inline bool is_unique() const { return use_count() == 1; }

    inline void swap(reference_counted_pointer& other) {  // never throws
      std::swap(pointer_,other.pointer_); 
    } 
  };

  template<class T>
  class reference_counted_object : public reference_counted {
  public:
    typedef T                                   object_t;

  protected:
    T* object_;
    sl::uint64_t global_time_stamp_;
    
  public:
    reference_counted_object(reference_counted_owner* owner = 0, object_t* obj = 0, sl::uint64_t global_time_stamp = 0) :
        reference_counted(owner), object_(obj), global_time_stamp_(global_time_stamp) {

    }

    virtual ~reference_counted_object() {
      if (object_) {
	delete object_;
	object_ = 0;
      }
    }
    
    inline void set_object(object_t* x) {
      object_ = x;
    }

    inline const object_t* object() const {
      return object_;
    }

    inline object_t* object() {
      return object_;
    }

    inline sl::uint64_t& global_time_stamp() {
      return global_time_stamp_;
    }
    
    inline sl::uint64_t global_time_stamp() const {
      return global_time_stamp_;
    }
  };
  
  /**
   * key_t : any key with operator<
   * data_t: derived from reference_counted
   */
  template<class KEY_T, class REFERENCE_COUNTED_DATA_T>
  class reference_counted_cache_base : public reference_counted_owner {
  public:
    typedef KEY_T                     key_t;
    typedef REFERENCE_COUNTED_DATA_T  data_t;
    typedef std::map<key_t, data_t*>  map_t;
    typedef typename map_t::iterator           map_iterator_t;
    typedef typename map_t::const_iterator     map_const_iterator_t;
      
  protected:
    map_t               cache_;
    // FIXME use pair instead of pure data* only for debug purposes: RESTORE TO data*
    std::list<data_t*>  previous_version_data_; // data updated when its use_count > 0, so the previous version has been moved here to be deleted when possible
    std::size_t         capacity_;

  public:
    inline reference_counted_cache_base() {
      capacity_ = 0;
     }

    inline virtual ~reference_counted_cache_base() {
      minimize_footprint();

      assert(cache_.size() == 0);
      assert(previous_version_data_.size() == 0);
    }

    inline std::size_t capacity() const {
      return capacity_;
    }

    inline void set_capacity(std::size_t x) {
      capacity_ = x;
    }

    inline void insert(const key_t& k, data_t* data) {
      //      assert(cache_.find(k) == cache_.end());

      map_iterator_t it = cache_.find(k);
      if (it != cache_.end()) {
	// data already in cache, delete previous one if no referenced otherwise move to previous_version_data_
	if (it->second->use_count() <= 0) {
	  SL_TRACE_OUT(0) << "data already in cache: delete" << std::endl;
	  delete it->second;
	} else {
	  SL_TRACE_OUT(0) << "data already in cache: delayed delete" << std::endl;
	  previous_version_data_.push_back(it->second);
	}

	// set new data in cache
	it->second = data;
      } else {
	std::pair<map_iterator_t, bool> pair_it = cache_.insert(std::pair<key_t,data_t*>(k, data));
	assert(pair_it.second);
      }
    }

    inline std::size_t size() const {
      return cache_.size();
    }

    // used in grid_diamond_graph_incore::refine(
    inline data_t* operator[](const key_t& k) {
      data_t* result = 0;
      map_iterator_t ci = cache_.find(k);
      if (ci != cache_.end()) {
        result = ci->second;
      }
      return result;
    }
    
    inline const data_t* operator[](const key_t& k) const {
      const data_t* result = 0;
      map_const_iterator_t ci = cache_.find(k);
      if (ci != cache_.end()) {
        result = ci->second;
      }
      return result;
    }
    

    typedef typename std::pair<int, map_iterator_t> time_iterator_pair_t;

    struct time_iterator_pair_less 
    {
      bool operator()(const time_iterator_pair_t& x, const time_iterator_pair_t& y) const {
        return x.first <y.first;
      }
    };
 
    inline void minimize_footprint() {
      //  clear cache
      SL_TRACE_OUT(1) << "clear cache" << std::endl;
      for(map_iterator_t it = cache_.begin();
          it != cache_.end();
          /**/) {
	assert(it->second);
	if (it->second->use_count() <= 0) {
	  // delete data
	  map_iterator_t del_it = it;
	  ++it;
	  delete (del_it->second);
	  del_it->second = 0;
	  cache_.erase(del_it);
	} else {
	  ++it;
	}
      }

      SL_TRACE_OUT(1) << "clear previous_version_data_" << std::endl;
      for(typename std::list<data_t*>::iterator v_it = previous_version_data_.begin();
	  v_it != previous_version_data_.end();
	  ) {
	if ((*v_it)->use_count() <= 0) {
	  // erase entry from previous_version_data_
	  typename std::list<data_t*>::iterator del_v_it = v_it;
	  ++v_it;

	  // delete data
	  delete (*del_v_it);
	  previous_version_data_.erase(del_v_it);
	} else {
	  ++v_it;
	}
      }
      SL_TRACE_OUT(1) << "After minimize footprint" << std::endl;
      //      print_stats();
    }

    inline void collect_garbage() {
      //      std::cerr << "collect_garbage, cache sz " << cache_.size() << " release_time_stamp_ " << this->release_time_stamp_ << std::endl;
      if (cache_.size() > capacity_ && 
	  (not_referenced_count_> 0.2*capacity_)) {

	// delete all data in previous_version_data_ with use_count = 0
	//	if (previous_version_data_.size() > 0) {std::cerr << "pending erasing sz " << previous_version_data_.size() << std::endl;}
	for(typename std::list<data_t*>::iterator v_it = previous_version_data_.begin();
	    v_it != previous_version_data_.end();
	    ) {
	  assert((*v_it) != 0);
	  if ((*v_it)->use_count() <= 0) {
	    // delete data
	    delete (*v_it);

	    // erase entry from previous_version_data_
	    typename std::list<data_t*>::iterator del_v_it = v_it;
	    ++v_it;
	    previous_version_data_.erase(del_v_it);
	  } else {
	    ++v_it;
	  }
	}

        std::set<time_iterator_pair_t, time_iterator_pair_less > dereferenceds;
        for(map_iterator_t it = cache_.begin();
            it != cache_.end();
            ++it) {
          if (it->second->use_count() <= 0) {
            dereferenceds.insert(std::make_pair(it->second->release_time_stamp(), it));
          }
        }
	//	int dereferenced_sz =  dereferenceds.size();
	int count = 0;        
        typename std::set<time_iterator_pair_t, time_iterator_pair_less >::iterator si = dereferenceds.begin();
        while((cache_.size() > 0.7*capacity_) && !dereferenceds.empty()) {
          typename std::set<time_iterator_pair_t, time_iterator_pair_less >::iterator del_si = si;
          ++si;
	  //	  std::cerr << "erasing " << del_si->second->first << std::endl;
          // delete data from the map
	  assert(del_si->second->second);
          delete del_si->second->second;
	  del_si->second->second = 0;

          // remove entry from the map
          cache_.erase(del_si->second);
          // delete from dereferenceds set
          dereferenceds.erase(del_si);
	  ++count;
        }
	//	std::cerr << "dereferenced objects " << dereferenced_sz << ", deleted objects " << count << ", cache size after sweep " << cache_.size() << " cache capacity " << capacity_ << std::endl;
      }
    }

    virtual void handle_last_deref(const reference_counted* o) {
      reference_counted_owner::handle_last_deref(o);
      collect_garbage();
    }

    map_const_iterator_t map_begin() const {
      return cache_.begin();
    }

    map_const_iterator_t map_end() const {
      return cache_.end();
    }

    void print_stats() const {
      int prev_count = 0;
      for(typename std::list<data_t*>::const_iterator v_it = previous_version_data_.begin();
	  v_it != previous_version_data_.end();
	  ++v_it) {
	assert((*v_it) != 0);
	if ((*v_it)->use_count() <= 0) {
	  ++prev_count;
	}
      }

      std::cerr << "CACHE STATS: " << 
	" size " << cache_.size() << 
	" capacity " << capacity_ << 
	" refcounted=" << referenced_count_ << 
	" notrefcounted=" << not_referenced_count_ << 
	" prev version=" << previous_version_data_.size() << 
	" not refcounted previous ver " << prev_count <<
	" release time stamp " << this->release_time_stamp_ << std::endl;
    }

    protected:
  
    };

} // namespace cbdam 

#endif // CBDAM_REFERENCE_COUNTED_CACHE_HPP

#ifndef CBDAM_REFERENCE_COUNTED_CACHE_IPP
#define CBDAM_REFERENCE_COUNTED_CACHE_IPP

namespace cbdam {

  inline void reference_counted_owner::handle_create(const reference_counted* o) {
    if (o) {};
    assert(o);
    assert(o->use_count()==0);
    ++not_referenced_count_;
  }

  inline void reference_counted_owner::handle_destroy(const reference_counted* o) {
    if (o) {};
    assert(o);
    assert(o->use_count()==0);
    assert(not_referenced_count_>0);
    --not_referenced_count_;
  }

  inline void reference_counted_owner::handle_last_deref(const reference_counted* o) {
    assert(o);
    assert(o->use_count()==0);
    assert(referenced_count_>0);
    
    ++not_referenced_count_;
    --referenced_count_;
    
    ++release_time_stamp_;
    
    o->set_release_time_stamp(release_time_stamp_);
  }

  inline void reference_counted_owner::handle_first_ref(const reference_counted* o) {
    if (o) {};
    assert(o);
    assert(o->use_count()==1);
    assert(not_referenced_count_>0);
 
    --not_referenced_count_;
    ++referenced_count_;
  }

} // namespace cbdam 

#endif // CBDAM_REFERENCE_COUNTED_CACHE_IPP
