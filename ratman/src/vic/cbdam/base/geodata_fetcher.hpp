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
#ifndef CBDAM_GEODATA_FETCHER_HPP
#define CBDAM_GEODATA_FETCHER_HPP

//#undef NDEBUG // FIXME

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/grid_point.hpp>
#include <sl/utility.hpp>
#include <sl/clock.hpp>
#include <string>
#include <list>
#include <map>
#include <cassert>

// libCurl
#include <curl/curl.h>

#if LIBCURL_VERSION_NUM >= 0x071000 
#  define HAVE_LIBCURL_HTTP_PIPELINING 1
#else 
#  define HAVE_LIBCURL_HTTP_PIPELINING 0
#endif


namespace cbdam {
  
  namespace detail {
    
    std::size_t geodata_fetcher_curl_callback_append_to_byte_array(void *ptr,
								   std::size_t size,
								   std::size_t nmemb,
								   void *data);
  }
  
  /**
   *
   */
  template<class T>
  class geodata_fetcher {
  public:
    enum status_t { 
      DONE = 0, NULL_DATA, NOT_FOUND, FAILED 
    };

    typedef T                                           value_t;
    typedef grid_point_t                                key_t;
    typedef std::pair<status_t, value_t*>               status_data_pair_t;

    class result_entry {
    protected:
      key_t			key_;
      status_data_pair_t	status_data_pair_;
      uint32_t			time_stamp_;

    public:
      result_entry(const key_t& k, const status_data_pair_t& sdp, uint32_t ts) : 
	key_(k), status_data_pair_(sdp), time_stamp_(ts) { 
      }
      
      const key_t& key() const { return key_; }
      const status_data_pair_t&	status_data_pair() const { return status_data_pair_; };
      uint32_t time_stamp() const { return time_stamp_; }
    };

    typedef std::list<result_entry>			result_buffer_t;
    typedef std::pair<key_t, aabox2d_t>                 key_box_pair_t;
    typedef std::map<key_t, aabox2d_t>                  key_box_map_t;
    typedef std::map<key_t, uint32_t>			null_data_cache_t;	// diamond id, last frame request
 
    typedef typename result_buffer_t::iterator          result_iterator_t;
    typedef typename result_buffer_t::const_iterator    const_result_iterator_t;

    /**
        request_result_t     v;
	key_t                v.first  ;
	result_timestamp_t   v.second;
	status_data_pair_t   v.second.first
	timestamp_t          v.second.second
        status_t	     v.second.first.first
        value_t*             v.second.first.second

	std::make_pair(key, std::make_pair(std::make_pair(status,value*), timestamp));
             v
            / \
        key  / \
            /\  timestamp
      status  value*
    */
  protected:
    
    result_buffer_t           result_buffer_;
    std::size_t               result_buffer_capacity_;
    key_box_map_t             pending_requests_;
    std::list<key_box_pair_t> new_requests_;
    null_data_cache_t	      null_data_cache_;
    std::size_t		      null_data_cache_max_size_;
    std::size_t		      null_data_current_fetch_time_;

    uint32_t		      time_stamp_;

  protected:

    std::string         base_url_;
    std::string         srs_;
    aabox2d_t           uv_box_;
    std::string         about_;
    bool                is_connected_;

  protected: // http

#if HAVE_LIBCURL_HTTP_PIPELINING
    enum { MAX_SIMULTANEOUS_TRANSFERS = 4 };
#else
    enum { MAX_SIMULTANEOUS_TRANSFERS = 4 }; // FIXME
#endif

    typedef std::vector<uint8_t> byte_array_t;

    std::vector<CURL *>          req_connection_;
    std::vector<key_t>           req_key_;
    std::vector<std::string>     req_url_;
    std::vector<byte_array_t>    req_byte_array_;
    
    CURLM *multi_connection_;
    CURLSH *curl_shared_data_;
    sl::real_time_clock clock_;
    double timeout_sec_;

  public:

    geodata_fetcher(const std::string& url,
		    const std::string& default_srs = "EPSG:4326",
		    const aabox2d_t& default_uv_box = aabox2d_t(point2d_t(-180.0,-90.0),
								point2d_t( 180.0, 90.0)),
								  
		    const std::string& default_about = "");

    virtual ~geodata_fetcher();

  public: // Connection protocol

    virtual void connect();

    virtual void disconnect();

    bool is_connected() const;

  public: // State

    const std::string& base_url() const;

    const std::string& srs() const;

    const aabox2d_t& uv_box() const;

    const std::string& about() const;

    bool is_http() const;

    std::size_t batch_count() const;
    
    bool data_available() const;

  public: // High level fetching protocol

    virtual void prefetch(const key_t& k, const aabox2d_t& k_uv_box);

    virtual status_data_pair_t fetch(const key_t& k, const aabox2d_t& k_uv_box);
 
    virtual status_data_pair_t synchronous_fetch(const key_t& k, const aabox2d_t& k_uv_box,
						 int timeout_s = 10);
 
    virtual void tick();

    std::size_t buffer_size() const;

    std::size_t buffer_capacity() const;

  public: // Low-level Request handling

    result_iterator_t find_result(const key_t& k);

    const_result_iterator_t find_result(const key_t& k) const;

    virtual bool is_busy() const;

    virtual bool is_serving(const key_t& k) const;

    virtual bool is_out_of_bounds(const key_t& k, const aabox2d_t& k_uv_box) const;

    void insert_request(const key_t& k, const aabox2d_t& k_uv_box);
    
    virtual void send_requests();

    virtual void receive();
    
    bool is_pending(const key_t& k) const;

    virtual void clear(); // assert(!busy)
    
  protected:

    virtual value_t* decoded(const uint8_t* buf,
			     std::size_t buf_size) const = 0;
    
    virtual void purge_out_of_bound_requests();

  protected:

    void push_result(const key_t& k,
		     const status_data_pair_t& data);

    virtual void handle_nodata_response(const key_t& k);

    virtual void handle_data_response(const key_t& k,
				      const uint8_t* buf,
				      std::size_t buf_size);

    virtual void handle_error_response(const key_t& k,
				       const std::string& message);

    void null_data_cache_cleanup();

  protected:

    virtual void direct_connect();

    virtual void direct_disconnect();

    virtual void direct_send_requests();

    virtual void direct_receive();

  protected:

    virtual void http_connect();

    virtual void http_disconnect();

    virtual std::string http_url_string(const key_t&k, const aabox2d_t& uv_box) const;

    virtual void http_send_requests();
  
    virtual void http_receive();

    virtual void http_handle_response(CURLcode code,
				      const key_t& k,
				      const byte_array_t& data);

    
  };

} // namespace cbdam 

#endif // CBDAM_GEODATA_FETCHER_HPP

#ifndef CBDAM_GEODATA_FETCHER_IPP
#define CBDAM_GEODATA_FETCHER_IPP

namespace cbdam {
  ///////////////////////////////////////// geodata_fetcher //////////////////////////////////////////////////

  template <class T>
  inline geodata_fetcher<T>::geodata_fetcher(const std::string& url,
					     const std::string& default_srs,
					     const aabox2d_t& default_uv_box,								  
					     const std::string& default_about) :
    base_url_(url), srs_(default_srs), uv_box_(default_uv_box), about_(default_about), is_connected_(false) {

    SL_TRACE_OUT(-1) << 
      "Creating service for: " << url << " BATCH_COUNT=" << batch_count() <<
      std::endl;

    result_buffer_capacity_ = 16; // FIXME

    curl_shared_data_ = curl_share_init();
    curl_share_setopt(curl_shared_data_, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    for (std::size_t i=0; i<batch_count(); ++i) {
      CURL *p=curl_easy_init();
      curl_easy_setopt(p, CURLOPT_SHARE, curl_shared_data_);
      // curl_easy_setopt(p, CURLOPT_DNS_CACHE_TIMEOUT, -1); // cache forever FIXME
      req_connection_.push_back(p);
      req_key_.push_back(grid_point_t());
      req_byte_array_.push_back(byte_array_t());
      req_url_.push_back(std::string());
    }
    multi_connection_=0;
    timeout_sec_=10.0;

    null_data_cache_max_size_ = 16*1024;
    null_data_current_fetch_time_ = 0;

    time_stamp_ = 0;
  }

  template <class T>
  inline geodata_fetcher<T>::~geodata_fetcher() {
    pending_requests_.clear();
    null_data_cache_.clear();
    clear();

    // -- 
    for (std::size_t i=0; i<req_connection_.size(); ++i) {
      curl_easy_cleanup(req_connection_[i]);
    }
    curl_share_cleanup(curl_shared_data_);
  }

  template <class T>
  const std::string& geodata_fetcher<T>::base_url() const {
    return base_url_;
  }

  template <class T>
  const std::string& geodata_fetcher<T>::about() const {
    return about_;
  }

  template <class T>
  const std::string& geodata_fetcher<T>::srs() const {
    return srs_;
  }

  template <class T>
  const aabox2d_t& geodata_fetcher<T>::uv_box() const {
    return uv_box_;
  }

  template <class T>
  bool geodata_fetcher<T>::is_connected() const {
    return is_connected_;
  }

  template <class T>
  inline void geodata_fetcher<T>::connect() {
    if (!is_connected()) {
      if (is_http()) {
	http_connect();
      } else {
	direct_connect();
      }
    }
  }

  template <class T>
  inline void geodata_fetcher<T>::disconnect() {
    if (is_connected()) {
      if (is_http()) {
	http_disconnect();
      } else {
	direct_disconnect();
      }
    }
  }

  template <class T>
  bool geodata_fetcher<T>::is_http() const {
    return sl::matches(base_url(), "http:*");
  }

  template <class T>
  inline std::size_t geodata_fetcher<T>::batch_count() const {
    return MAX_SIMULTANEOUS_TRANSFERS;
  }

  template <class T>
  inline bool geodata_fetcher<T>::data_available() const {
    return !result_buffer_.empty();
  }

  template <class T>  
  inline bool geodata_fetcher<T>::is_serving(const key_t& k) const {
    return 
      (pending_requests_.find(k) != pending_requests_.end()) ||
      (find_result(k) != result_buffer_.end());
  }


  template <class T>  
  inline void geodata_fetcher<T>::prefetch(const key_t& k, const aabox2d_t& k_uv_box) {
    if (find_result(k) == result_buffer_.end()) {
      if (null_data_cache_.find(k) == null_data_cache_.end()) {
      // Not found, add to new request if not already there
	if (pending_requests_.find(k) == pending_requests_.end()) {
	  new_requests_.push_back(std::make_pair(k, k_uv_box));
	}
      }
    }
  }
 
  template <class T>  
  inline typename geodata_fetcher<T>::status_data_pair_t  geodata_fetcher<T>::fetch(const key_t& k, const aabox2d_t& k_uv_box) {
    result_iterator_t it = find_result(k);
    if (it != result_buffer_.end()) {
      // Found, return it and remove from result buffer
      // Note that we have to remove from result buffer to
      // avoid deleting the object if someone takes a pointer
      // to it. We assume that the result buffer is just
      // a temporary storage area holding pointers to 
      // objects that nobody else references
      status_data_pair_t result = it->status_data_pair();
      result_buffer_.erase(it);
      SL_TRACE_OUT(1) << k << " result status " << result.first << std::endl;
      return result;
    } else {
      // Request it and return not found
      null_data_cache_t::iterator nd_it = null_data_cache_.find(k);
      if (nd_it != null_data_cache_.end()) {
	// we already know it's null data: update its access time, and return null data
	nd_it->second = null_data_current_fetch_time_;
	++null_data_current_fetch_time_;
	SL_TRACE_OUT(1) << " fetch " << k << " null data requested, null data cache sz " << null_data_cache_.size() << std::endl;
	return std::make_pair(NULL_DATA, (T*)0);
      } else if (pending_requests_.find(k) == pending_requests_.end()) {
        new_requests_.push_back(std::make_pair(k, k_uv_box));
      }
      SL_TRACE_OUT(1) << k << " result status NOT FOUND" << std::endl;
      return std::make_pair(NOT_FOUND, (T*)0);
    }
  }
 
  template <class T>  
  inline typename geodata_fetcher<T>::status_data_pair_t geodata_fetcher<T>::synchronous_fetch(const key_t& k, const aabox2d_t& k_uv_box,
											       int timeout_s) {
    sl::real_time_clock clock;
    clock.restart();

    // Wait for outstanding requests
    while (is_busy()) {
      receive();
      if (clock.elapsed().as_seconds() > timeout_s) {
	SL_TRACE_OUT(-1) << "operation timed out after " << clock.elapsed().as_seconds() << " sec" << std::endl;
	return std::make_pair(FAILED, (T*)0);
      }
    }

    // Send request
    insert_request(k, k_uv_box);
    send_requests();

    // Wait for reply
    while (is_busy()) {
      receive();
      if (clock.elapsed().as_seconds() > timeout_s) {
	SL_TRACE_OUT(-1) << "operation timed out after " << clock.elapsed().as_seconds() << " sec" << std::endl;
	return std::make_pair(FAILED, (T*)0);
      }
    }

    // Decode result
    result_iterator_t it = find_result(k);
    if (it != result_buffer_.end()) {
      // Found, return it and 
      status_data_pair_t result = it->status_data_pair();
      result_buffer_.erase(it);
      return result;
    } else {
      // Should not happen!
      return std::make_pair(FAILED, (T*)0);
    }
  }
 
  template <class T>  
  inline void  geodata_fetcher<T>::tick() {
    ++time_stamp_;

    receive();  
    if (!is_busy()) {
      if (new_requests_.size()) SL_TRACE_OUT(1) << " PREPARING NEW REQUESTS: new sz:" << new_requests_.size() << ", result sz: " << result_buffer_.size() << std::endl;
      // Move first batch_count() requests to pending
      for (std::list<key_box_pair_t>::const_iterator it = new_requests_.begin();
	   it != new_requests_.end() && pending_requests_.size() < batch_count();
	   ++it) {
        pending_requests_.insert(*it);
      }
      new_requests_.clear();
      if (pending_requests_.size()) SL_TRACE_OUT(1) << "MOVED TO PENDING REQUESTS: pending sz: " << pending_requests_.size() << ", result sz: " << result_buffer_.size() << std::endl;
      send_requests();
    }

    // End of frame, forget
    new_requests_.clear();

    null_data_cache_cleanup();

    //    result_buffer_cleanup();
    // remove all results older than 10 ticks
    int c = 0;
    while (!result_buffer_.empty() &&
	   result_buffer_.back().time_stamp() < time_stamp_ - 10) {
      value_t* p = result_buffer_.back().status_data_pair().second;
      if (p) {
	SL_TRACE_OUT(1) << "erasing " << c << " time_stamp " << result_buffer_.back().time_stamp() << " vs " << time_stamp_ << " res_buf size " << result_buffer_.size() <<  std::endl;
	delete p;
      }
      result_buffer_.pop_back();
      ++c;
    }
  }

  template <class T>  
  inline std::size_t  geodata_fetcher<T>::buffer_size() const {
    return result_buffer_.size();
  }
  
  template <class T>  
  inline std::size_t  geodata_fetcher<T>::buffer_capacity() const {
    return result_buffer_capacity_;
  }
 template <class T>  
 inline typename geodata_fetcher<T>::result_iterator_t geodata_fetcher<T>::find_result(const key_t& k) {
   for (result_iterator_t it = result_buffer_.begin(); 
	it != result_buffer_.end();
	++it) {
     if (it->key() == k) return it;
   }
   return result_buffer_.end();
 }

 template <class T>  
 inline typename geodata_fetcher<T>::const_result_iterator_t geodata_fetcher<T>::find_result(const key_t& k) const {
   for (const_result_iterator_t it = result_buffer_.begin(); 
	it != result_buffer_.end();
	++it) {
     if (it->key() == k) return it;
   }
   return result_buffer_.end();
 }

  template <class T>  
  inline bool geodata_fetcher<T>::is_busy() const {
    return pending_requests_.size() != 0;
  }

  template <class T>
  inline bool geodata_fetcher<T>::is_out_of_bounds(const key_t& /*k*/, const aabox2d_t& k_uv_box) const {
    return
      (k_uv_box[1][0]<uv_box_[0][0] ||
       k_uv_box[0][0]>uv_box_[1][0] ||
       k_uv_box[1][1]<uv_box_[0][1] ||
       k_uv_box[0][1]>uv_box_[1][1]);
  }

  template <class T>
  inline void geodata_fetcher<T>::insert_request(const key_t& k, const aabox2d_t& k_uv_box) {
    if (pending_requests_.size() < batch_count()) {
      pending_requests_.insert(std::make_pair(k, k_uv_box));
    }
  }

  template <class T>
  inline void geodata_fetcher<T>::purge_out_of_bound_requests() {
    for (typename key_box_map_t::iterator it = pending_requests_.begin();
	 it != pending_requests_.end();
	 /**/) {
      if (is_out_of_bounds(it->first, it->second)) {

	SL_TRACE_OUT(1) << "Request " << it->first << " out of bounds:" << 
	  "(" << it->second[0][0] << " " << it->second[0][1] << ")" <<
	  "(" << it->second[1][0] << " " << it->second[1][1] << ")" <<
	  std::endl;

	typename key_box_map_t::iterator del_it = it;
	handle_nodata_response(it->first);
	++it;
	pending_requests_.erase(del_it);
      } else {
	++it;
      }
    }
  }   

  template <class T>
  inline void geodata_fetcher<T>::send_requests() {
    purge_out_of_bound_requests();

    if (!pending_requests_.empty()) {
      if (!is_connected_) {
	connect();
      }
      
      if (!is_connected_) {
	for (typename key_box_map_t::iterator it = pending_requests_.begin();
	     it != pending_requests_.end();
	     /**/) {
	  handle_error_response(it->first, "Unable to connect");
	}
	pending_requests_.clear();
      } else {
	if (is_http()) {
	  http_send_requests();
	} else {
	  direct_send_requests();
	}
      }
    }
  }

  template <class T>
  inline void geodata_fetcher<T>::receive() {
    if (is_http()) {
      http_receive();
    } else {
      direct_receive();
    }
  }

  template <class T>
  inline bool geodata_fetcher<T>::is_pending(const key_t& k) const {
    return pending_requests_.find(k) != pending_requests_.end();
  }
 
  template <class T>
  inline void geodata_fetcher<T>::clear() {
    assert(!is_busy());

    for(typename result_buffer_t::iterator it = result_buffer_.begin();
        it != result_buffer_.end();
        ++it) {
      SL_TRACE_OUT(1) << "delete " << it->key() << std::endl;
      if (it->status_data_pair().second) delete it->status_data_pair().second;
      SL_TRACE_OUT(1) << "done" << std::endl;
    }
        
    result_buffer_.clear();
    pending_requests_.clear();

    if (multi_connection_) {
      curl_multi_cleanup(multi_connection_);
      multi_connection_=0;
    }
  } 


  // ----------------- direct implementation

  template <class T>
  inline void geodata_fetcher<T>::direct_connect() {
    SL_TRACE_OUT(-1) << "Service has no direct implementation - ignoring connection request" << std::endl;
    is_connected_ = false;
  }

  template <class T>
  inline void geodata_fetcher<T>::direct_disconnect() {
    SL_TRACE_OUT(-1) << "Service has no direct implementation - ignoring disconnection request" << std::endl;
    is_connected_ = false;
  }

  template <class T>
  inline void geodata_fetcher<T>::direct_send_requests() {
    SL_TRACE_OUT(-1) << "Service has no direct implementation - ignoring requests" << std::endl;
    pending_requests_.clear();
  }

  template <class T>
  inline void geodata_fetcher<T>::direct_receive() {
    SL_TRACE_OUT(-1) << "Service has no direct implementation - ignoring requests" << std::endl;
    pending_requests_.clear();
  }

  // ----------------- http implementation

  template <class T>
  inline void geodata_fetcher<T>::http_connect() {
    // Default is connectionless
    is_connected_ = true;
  }

  template <class T>
  inline void geodata_fetcher<T>::http_disconnect() {
    // Default is connectionless
    is_connected_ = false;
  }
  
  template <class T>
  std::string geodata_fetcher<T>::http_url_string(const key_t& /*k*/, const aabox2d_t& /*uv_box*/) const {
    return "NO PROTOCOL SPECIFIED";
  }

  template <class T>
  void geodata_fetcher<T>::http_send_requests() {
    assert(pending_requests_.size() <= batch_count());
    assert(multi_connection_==0);
    multi_connection_ = curl_multi_init();
#if HAVE_LIBCURL_HTTP_PIPELINING
    curl_multi_setopt(multi_connection_, CURLMOPT_PIPELINING, 1);
#endif
    int i=0;
    for(std::map<key_t, aabox2d_t>::iterator it = pending_requests_.begin();
        it != pending_requests_.end();
	++it) {
      req_key_[i] = it->first;

      req_url_[i] = http_url_string(it->first, it->second);

      SL_TRACE_OUT(1) << "URL: " << req_url_[i] << std::endl;

      curl_easy_setopt(req_connection_[i], CURLOPT_URL, req_url_[i].c_str());
      curl_easy_setopt(req_connection_[i], CURLOPT_WRITEFUNCTION, detail::geodata_fetcher_curl_callback_append_to_byte_array);
      curl_easy_setopt(req_connection_[i], CURLOPT_WRITEDATA, (void *)&(req_byte_array_[i]));
      curl_easy_setopt(req_connection_[i], CURLOPT_FAILONERROR, 1);
      curl_easy_setopt(req_connection_[i], CURLOPT_VERBOSE, 0); // FIXME
      curl_easy_setopt(req_connection_[i], CURLOPT_USERAGENT, "vicgeo");

      req_byte_array_[i].resize(0);

      curl_multi_add_handle(multi_connection_, req_connection_[i]);
      ++i;
    }
    clock_.restart();

    // Sends 
    http_receive();
  }

  template <class T>
  void geodata_fetcher<T>::http_handle_response(CURLcode code,
						const key_t& k,
						const byte_array_t& data) {
    // Default: map not found and empy to NULL, OK to data, others to error
    if ((code == CURLE_HTTP_NOT_FOUND) || 
	(code == CURLE_OK && data.empty())) {
      handle_nodata_response(k);
    } else if (code == CURLE_OK) {
      handle_data_response(k, &(data[0]), data.size());
    } else {
      handle_error_response(k, std::string("HTTP error: curlcode= ")+sl::to_string(code));
    }
  }

  template <class T>
  void geodata_fetcher<T>::push_result(const key_t& k,
				       const status_data_pair_t& data) {
    result_buffer_.push_back(result_entry(k, data, time_stamp_));
    if (result_buffer_.size() > buffer_capacity()) {
      value_t * oldest_value = result_buffer_.front().status_data_pair().second;
      if (oldest_value) {
	delete oldest_value;
      }
      result_buffer_.pop_front();
    }
  }


  template <class T>
  void geodata_fetcher<T>::handle_data_response(const key_t& k,
						const uint8_t* buf,
						std::size_t buf_size) {
    T* a = decoded(buf, buf_size);
    if (a) {
      SL_TRACE_OUT(1) << "RECEIVED: k=" << k << " status= DONE" << std::endl;
      push_result(k, std::make_pair(DONE, a));
    } else {
      // Decoding error
      SL_TRACE_OUT(1) << "RECEIVED: k=" << k << " status= FAILED" << std::endl;
      push_result(k, std::make_pair(FAILED, (T*)0));
    }
  }

  template <class T>
  void geodata_fetcher<T>::handle_nodata_response(const key_t& k) {
    SL_TRACE_OUT(1) << "RECEIVED: k=" << k << " status= NULL" << std::endl;
    push_result(k, std::make_pair(NULL_DATA, (T*)0));
    null_data_cache_.insert(std::make_pair(k, null_data_current_fetch_time_));
    ++null_data_current_fetch_time_;
  }
 
  template <class T>
  void geodata_fetcher<T>::handle_error_response(const key_t& k, const std::string& message) {
    SL_TRACE_OUT(1) << "Error for key = " << k[0] << " " << k[1] << " " << k[2] << ": " << message << std::endl;
    push_result(k, std::make_pair(FAILED, (T*)0));
  }

  template <class T>
  void geodata_fetcher<T>::null_data_cache_cleanup() {
    // cleanup null diamonds
    std::size_t null_data_cache_size = null_data_cache_.size();
    if (null_data_cache_size > null_data_cache_max_size_) {
      std::size_t lifetime;
      for(lifetime = 128; 
	  (lifetime >=1) && (null_data_cache_size > 0.7 * null_data_cache_max_size_);
	  lifetime /= 2) {
        for(null_data_cache_t::iterator it = null_data_cache_.begin();
	    it != null_data_cache_.end() && (null_data_cache_size > 0.7 * null_data_cache_max_size_);
	    ) {
          null_data_cache_t::iterator it_del = it;
          ++it;
          if (it_del->second + lifetime <= null_data_current_fetch_time_) {
            null_data_cache_.erase(it_del);
            --null_data_cache_size;
          }
        }
      }
      
      SL_TRACE_OUT(1) << "CACHE: NULL DIAMONDS = " << null_data_cache_size << " vs." << null_data_cache_max_size_ << " last lifetime " << lifetime << std::endl;
    }
    assert(null_data_cache_size == null_data_cache_.size());
 
  }

  template <class T>
  void geodata_fetcher<T>::http_receive() {
    if (multi_connection_) {
      int running_handles = 0;
	
      CURLMcode performcode = CURLM_CALL_MULTI_PERFORM;
      while (performcode == CURLM_CALL_MULTI_PERFORM) {
	performcode=curl_multi_perform(multi_connection_, &running_handles);
      }
      bool timeout = (clock_.elapsed().as_milliseconds()>(1000.0*timeout_sec_));
      
      bool cleanup = false;

      if ((performcode == CURLM_OK) && (running_handles == 0)) {
	// Ok - check results

	const std::size_t N = pending_requests_.size();

	std::vector<bool> transfer_checked(N, false);

	SL_TRACE_OUT(1) << "Checking transfer codes" << std::endl;
	// See how transfer went
	CURLMsg *msg; // for picking up messages with the transfer status 
	int msgs_left; // how many messages are left 
	while ((msg = curl_multi_info_read(multi_connection_, &msgs_left))) {
	  if (msg->msg == CURLMSG_DONE) {
	    for (std::size_t i=0; i<N; ++i) {
	      if (msg->easy_handle == req_connection_[i]) {
		transfer_checked[i] = true;

		CURLcode retcode = msg->data.result;

		SL_TRACE_OUT(1) << req_key_[i] << ": " << req_url_[i] << ": CODE=" << retcode << std::endl;

		http_handle_response(retcode, 
				     req_key_[i],
				     req_byte_array_[i]);
	      }
	    }
	  }
	}

	SL_TRACE_OUT(1) << "Checking transfer finalization" << std::endl;
	// Mark failed 
	for (std::size_t i=0; i<N; ++i) {
	  if (!transfer_checked[i]) {
	    handle_error_response(req_key_[i], "HTTP transfer not finalized");
	  }
	}
      
	// Signal cleanup 
	cleanup = true;
      } else if (performcode == CURLM_OK && !timeout) {
	// Continue 
	// do nothing...
	cleanup = false;
      } else {
	// ERROR: Timeout or abort
	std::size_t N= pending_requests_.size();
	for (std::size_t i=0; i<N; ++i) {
	  handle_error_response(req_key_[i], "HTTP timeout");
	}

	cleanup = true;
      }

      // Cleanup on end or error
      if (cleanup) {
	SL_TRACE_OUT(1) << "Connection cleanup" << std::endl;
	// Destroy multi connection
	for (std::size_t i=0; i<pending_requests_.size(); ++i) {
	  curl_multi_remove_handle(multi_connection_, req_connection_[i]);
	}
	curl_multi_cleanup(multi_connection_);
	multi_connection_ = 0;

	// Mark requests served
	pending_requests_.clear();
	SL_TRACE_OUT(1) << "Done" << std::endl;
      }
    }
  }
  
} // namespace cbdam 

#endif // CBDAM_GEODATA_FETCHER_IPP
