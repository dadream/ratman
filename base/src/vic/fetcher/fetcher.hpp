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
#ifndef VIC_FETCHER_HPP
#define VIC_FETCHER_HPP

//#undef NDEBUG // FIXME

#include <sl/utility.hpp>
#include <sl/clock.hpp>
#include <sl/cstdint.hpp>
#include <string>
#include <list>
#include <map>
#include <cassert>
#include <QThread>
#include <QMutex>
// libCurl
#include <curl/curl.h>

#include <vic/fetcher/thread.hpp>

#if LIBCURL_VERSION_NUM >= 0x071000 
#  define HAVE_LIBCURL_HTTP_PIPELINING 1
#else 
#  define HAVE_LIBCURL_HTTP_PIPELINING 0
#endif


namespace vic {

  template<class T>
  class fetcher;

  namespace detail {

   template<class T>
    class fetcher_thread: public QThread {
    public: 
      typedef T                                           value_t; 
    protected:
      mutable QMutex mutex_;
      fetcher<value_t>* data_fetcher_;
      bool stop_requested_;
    public:

    fetcher_thread(fetcher<T>* data_fetcher) :
      data_fetcher_(data_fetcher), stop_requested_(false) {
    }

    virtual ~fetcher_thread() {
    }
      
    bool stop_requested() const {
      bool result;
      mutex_.lock();
      result = stop_requested_;
      mutex_.unlock();
      return result;
    }

      void request_stop() {
        mutex_.lock();
        stop_requested_ = true;
        mutex_.unlock();
      }
        
     virtual void run() {
	FETCHER_SET_THREAD_PRIORITY_IDLE;
	data_fetcher_->initialize();
        while (!stop_requested()) {
          FETCHER_CPU_YIELD;
          data_fetcher_->tick();
        }
      }
 
    };
  

    std::size_t vic_fetcher_curl_callback_append_to_byte_array(void *ptr,
							       std::size_t size,
							       std::size_t nmemb,
							       void *data);
  }
  
  /**
   *
   */
  template<class T>
  class fetcher {
  protected:
    friend class detail::fetcher_thread<T>;

  public:
    enum status_t { 
      DONE = 0, NULL_DATA, NOT_FOUND, FAILED 
    };

    typedef T                                           value_t;
    typedef std::pair<status_t, value_t*>               status_data_pair_t;
    typedef std::pair<std::string, status_data_pair_t>  request_result_t;
    typedef std::list<request_result_t>                 result_buffer_t;
    typedef std::list<std::string>                      request_buffer_t;
    typedef std::vector<sl::uint8_t>                        byte_array_t;

    typedef request_buffer_t::iterator                  request_iterator_t;
    typedef request_buffer_t::const_iterator            const_request_iterator_t;

    typedef typename result_buffer_t::iterator          result_iterator_t;
    typedef typename result_buffer_t::const_iterator    const_result_iterator_t;


  protected:
    mutable QMutex                             update_mutex_;
    detail::fetcher_thread<value_t>*           update_thread_;
    result_buffer_t                            result_buffer_;
    std::size_t                                result_buffer_capacity_;
    request_buffer_t                           pending_requests_;
    request_buffer_t                           new_requests_;

  protected:
    bool                is_connected_;

  protected: // http

#if HAVE_LIBCURL_HTTP_PIPELINING
    enum { MAX_SIMULTANEOUS_TRANSFERS = 4 };
#else
    enum { MAX_SIMULTANEOUS_TRANSFERS = 1 }; // FIXME
#endif


    std::vector<CURL *>          req_connection_;
    std::vector<std::string>     req_url_;
    std::vector<byte_array_t>    req_byte_array_;
    
    CURLM *multi_connection_;
    CURLSH *curl_shared_data_;
    sl::real_time_clock clock_;
    double timeout_sec_;

  public:

    fetcher();
    virtual ~fetcher();

  public: // Connection protocol

    bool is_connected() const;

  public: // State

    bool is_http(const std::string& url) const;

    bool is_http_pipelining() const;

    std::size_t batch_count() const;
    
  public: // High level fetching protocol

    virtual void prefetch(const std::string& url);

    virtual status_data_pair_t fetch(const std::string& url);
 
    //    virtual status_data_pair_t synchronous_fetch(const key_t& k, const aabox2d_t& k_uv_box,
    //					 int timeout_s = 10);
 

    virtual void initialize();

    virtual void tick();

    std::size_t buffer_size() const;

    std::size_t buffer_capacity() const;

  public: // Low-level Request handling

    result_iterator_t find_result(const std::string& k);

    const_result_iterator_t find_result(const std::string& k) const;

    request_iterator_t find_pending(const std::string& s);

    const_request_iterator_t find_pending(const std::string& s) const;

    virtual bool is_busy() const;

    virtual bool is_serving(const std::string& k) const;

    bool is_pending(const std::string& k) const;

    virtual void clear(); // assert(!busy)
    
  public:
    
    void update_start();

    void update_stop();

  protected:

    virtual value_t* decoded(const sl::uint8_t* buf,
			     std::size_t buf_size) const = 0;
    
  protected:

    void push_result(const std::string& k,
		     const status_data_pair_t& data);

    virtual void handle_nodata_response(const std::string& k);

    virtual void handle_data_response(const std::string& k,
				      const sl::uint8_t* buf,
				      std::size_t buf_size);

    virtual void handle_error_response(const std::string& k,
				       const std::string& message);

  protected:

    virtual void send_requests();
  
    virtual void receive();

    virtual void http_handle_response(CURLcode code,
				      const std::string& k,
				      const byte_array_t& data);

    
  };
  
} // namespace cbdam 

#endif // VIC_FETCHER_HPP

#ifndef VIC_FETCHER_IPP
#define VIC_FETCHER_IPP

namespace vic {
  ///////////////////////////////////////// fetcher //////////////////////////////////////////////////

  template <class T>
  fetcher<T>::fetcher() :
    is_connected_(false) {
    result_buffer_capacity_ = 16; // FIXME

    curl_shared_data_ = curl_share_init();
    curl_share_setopt(curl_shared_data_, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    for (std::size_t i=0; i<batch_count(); ++i) {
      CURL *p=curl_easy_init();
      curl_easy_setopt(p, CURLOPT_SHARE, curl_shared_data_);
      // curl_easy_setopt(p, CURLOPT_DNS_CACHE_TIMEOUT, -1); // cache forever FIXME
      req_connection_.push_back(p);
      req_byte_array_.push_back(byte_array_t());
      req_url_.push_back(std::string());
    }
    multi_connection_=0;
    timeout_sec_=30.0;
    update_thread_=0;
    update_start();
  }

  template <class T>
  fetcher<T>::~fetcher() {
    update_stop();
    pending_requests_.clear();
    clear();

    // -- 
    for (std::size_t i=0; i<req_connection_.size(); ++i) {
      curl_easy_cleanup(req_connection_[i]);
    }
    curl_share_cleanup(curl_shared_data_);
  }

  template <class T>
  void fetcher<T>::update_start() {
    if (!update_thread_) {
      update_thread_ = new detail::fetcher_thread<T>(this);
      update_thread_->start(QThread::IdlePriority);
      update_thread_->setPriority(QThread::IdlePriority);
    }    
  }

  template <class T>
  void fetcher<T>::update_stop() {
    if (update_thread_) {
      update_thread_->request_stop();
      update_thread_->wait();
      delete update_thread_;
      update_thread_ = 0;
    }
  }


  template <class T>
  bool fetcher<T>::is_connected() const {
    return is_connected_;
  }

  template <class T>
  bool fetcher<T>::is_http(const std::string& url) const {
    return sl::matches(url, "http:*");
  }

  template <class T>
  inline bool fetcher<T>::is_http_pipelining() const {
#ifdef HAVE_LIBCURL_HTTP_PIPELINING
    return true;
#else
    return false;
#endif
  }

  template <class T>
  inline std::size_t fetcher<T>::batch_count() const {
    return MAX_SIMULTANEOUS_TRANSFERS;
  }

  template <class T>  
  inline typename fetcher<T>::const_request_iterator_t fetcher<T>::find_pending(const std::string& s) const{
    for (const_request_iterator_t it = pending_requests_.begin(); 
	 it != pending_requests_.end();
	 ++it) {
      if (*it == s) return it;
    }
    return pending_requests_.end();
  }

  template <class T>  
  inline typename fetcher<T>::request_iterator_t fetcher<T>::find_pending(const std::string& s) {
    for (request_iterator_t it = pending_requests_.begin(); 
	 it != pending_requests_.end();
	 ++it) {
      if (*it == s) return it;
    }
    return pending_requests_.end();
  }

  template <class T>  
  inline bool fetcher<T>::is_serving(const std::string& k) const {
    update_mutex_.lock();
    bool result = 
      (find_pending(k) != pending_requests_.end()) ||
      (find_result(k) != result_buffer_.end());
    update_mutex_.unlock();
   
    return result;
  }


  template <class T>  
  inline void fetcher<T>::prefetch(const std::string& k) {
    update_mutex_.lock();
    if (find_result(k) == result_buffer_.end()) {
      if (find_pending(k) == pending_requests_.end()) {
	new_requests_.push_back(k);
      }
    }
    update_mutex_.unlock();
  }
 
  template <class T>  
  inline typename fetcher<T>::status_data_pair_t  fetcher<T>::fetch(const std::string& k) {
    status_data_pair_t result = std::make_pair(NOT_FOUND, (T*)0);
    update_mutex_.lock();
    result_iterator_t it = find_result(k);
    if (it != result_buffer_.end()) {
      // Found, return it and remove from result buffer
      // Note that we have to remove from result buffer to
      // avoid deleting the object if someone takes a pointer
      // to it. We assume that the result buffer is just
      // a temporary storage area holding pointers to 
      // objects that nobody else references
      result = it->second;
      result_buffer_.erase(it);
    } else {
      if (find_pending(k) == pending_requests_.end()) {
        new_requests_.push_back(k);
      }
    }
    update_mutex_.unlock();
    return result;    
  }
 
 
  template <class T>  
  inline void  fetcher<T>::initialize() {
    clock_.restart();
  }


  template <class T>  
  inline void  fetcher<T>::tick() {
      receive();  
      if (!is_busy()) {
      // Move first batch_count() requests to pending
      update_mutex_.lock();
      for (request_buffer_t::iterator it = new_requests_.begin();
	   it != new_requests_.end() && pending_requests_.size() < batch_count();
	   ++it) {
        pending_requests_.push_back(*it);
      }
      new_requests_.clear();
      update_mutex_.unlock();
      send_requests();
    }
  }

  template <class T>  
  inline std::size_t  fetcher<T>::buffer_size() const {
    update_mutex_.lock();
    std::size_t result=result_buffer_.size();
    update_mutex_.unlock();
    return result;
  }
  
  template <class T>  
  inline std::size_t  fetcher<T>::buffer_capacity() const {
    return result_buffer_capacity_;
  }

  template <class T>  
  inline typename fetcher<T>::result_iterator_t fetcher<T>::find_result(const std::string& k) {
    for (result_iterator_t it = result_buffer_.begin(); 
	 it != result_buffer_.end();
	 ++it) {
      if (it->first == k) return it;
    }
    return result_buffer_.end();
  }

 template <class T>  
 inline typename fetcher<T>::const_result_iterator_t fetcher<T>::find_result(const std::string& k) const {
   for (const_result_iterator_t it = result_buffer_.begin(); 
	it != result_buffer_.end();
	++it) {
     if (it->first == k) return it;
   }
   return result_buffer_.end();
 }

  template <class T>  
  inline bool fetcher<T>::is_busy() const {
    return pending_requests_.size() != 0;
  }

  template <class T>
  inline bool fetcher<T>::is_pending(const std::string& k) const {
    return find_pending(k) != pending_requests_.end();
  }
 
  template <class T>
  inline void fetcher<T>::clear() {
    assert(!is_busy());

    for(typename result_buffer_t::iterator it = result_buffer_.begin();
        it != result_buffer_.end();
        ++it) {
      SL_TRACE_OUT(1) << "delete " << it->first << std::endl;
      if (it->second.second) delete it->second.second;
      it->second.second = 0;
      SL_TRACE_OUT(1) << "done" << std::endl;
    }
        
    result_buffer_.clear();
    pending_requests_.clear();

    if (multi_connection_) {
      curl_multi_cleanup(multi_connection_);
      multi_connection_=0;
    }
  } 


   template <class T>
  void fetcher<T>::send_requests() {
    assert(pending_requests_.size() <= batch_count());
    assert(multi_connection_==0);
    multi_connection_ = curl_multi_init();
#if HAVE_LIBCURL_HTTP_PIPELINING
    curl_multi_setopt(multi_connection_, CURLMOPT_PIPELINING, 1);
#endif
    int i=0;
    for(request_iterator_t it = pending_requests_.begin();
        it != pending_requests_.end();
	++it) {
      req_url_[i] = *it;

      SL_TRACE_OUT(1) << "URL: " << req_url_[i] << std::endl;

      curl_easy_setopt(req_connection_[i], CURLOPT_URL, req_url_[i].c_str());
      curl_easy_setopt(req_connection_[i], CURLOPT_WRITEFUNCTION, detail::vic_fetcher_curl_callback_append_to_byte_array);
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
    receive();
  }

  template <class T>
  void fetcher<T>::http_handle_response(CURLcode code,
						const std::string& k,
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
  void fetcher<T>::push_result(const std::string& k,
				       const status_data_pair_t& data) {
    update_mutex_.lock();
    result_buffer_.push_back(std::make_pair(k, data));
    if (result_buffer_.size() > buffer_capacity()) {
      value_t * oldest_value = result_buffer_.front().second.second;
      if (oldest_value) {
	delete oldest_value;
      }
      result_buffer_.pop_front();
    }
    update_mutex_.unlock();
  }


  template <class T>
  void fetcher<T>::handle_data_response(const std::string& k,
						const sl::uint8_t* buf,
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
  void fetcher<T>::handle_nodata_response(const std::string& k) {
    SL_TRACE_OUT(1) << "RECEIVED: k=" << k << " status= NULL" << std::endl;
    push_result(k, std::make_pair(NULL_DATA, (T*)0));
  }
 
  template <class T>
  void fetcher<T>::handle_error_response(const std::string& k, const std::string& message) {
    SL_TRACE_OUT(1) << "Error for key = " << k[0] << " " << k[1] << " " << k[2] << ": " << message << std::endl;
    push_result(k, std::make_pair(FAILED, (T*)0));
  }


  template <class T>
  void fetcher<T>::receive() {
    if (multi_connection_) {
      int running_handles = 0;
	
      CURLMcode performcode = CURLM_CALL_MULTI_PERFORM;
      while (performcode == CURLM_CALL_MULTI_PERFORM) {
	performcode=curl_multi_perform(multi_connection_, &running_handles);
      }
      bool timeout = (clock_.elapsed().as_milliseconds()>(1000.0*timeout_sec_));
      //if (timeout) std::cerr <<"############# TIMEOUT!" << std::endl;

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

		SL_TRACE_OUT(1) << req_url_[i] << ": " << req_url_[i] << ": CODE=" << retcode << std::endl;

		http_handle_response(retcode, 
				     req_url_[i],
				     req_byte_array_[i]);
	      }
	    }
	  }
	}

	SL_TRACE_OUT(1) << "Checking transfer finalization" << std::endl;
	// Mark failed 
	for (std::size_t i=0; i<N; ++i) {
	  if (!transfer_checked[i]) {
	    handle_error_response(req_url_[i], "HTTP transfer not finalized");
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
	  handle_error_response(req_url_[i], "HTTP timeout");
	}

	//cleanup = true;
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

#endif // VIC_FETCHER_IPP
