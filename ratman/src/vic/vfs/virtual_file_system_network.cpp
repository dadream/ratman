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
#include <vic/vfs/virtual_file_system_network.hpp>
#undef min
#undef max

// ==========================================================================
#ifdef WIN32
#include <windows.h>
#undef min
#undef max

static int gettimeofday (struct timeval *tv, void* /*tz*/) {
  union {
    LONGLONG ns100; /*time since 1 Jan 1601 in 100ns units */
    FILETIME ft;
  } now;

  GetSystemTimeAsFileTime (&now.ft);
  tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
  tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);
  return (0);
}

#else
#include <sys/time.h>

#endif
 
static inline double real_time_s() {
  struct timeval tv;
  gettimeofday(&tv,0);
  return (1.0*tv.tv_sec) + (1E-6*tv.tv_usec); 
}

// ==========================================================================

namespace vic {
  namespace vfs {

    virtual_file_system_network::virtual_file_system_network() {
#if HAVE_LIBCURL_HTTP_PIPELINING
      std::cerr << "###### CREATING NETWORKING SUBSYSTEM WITH HTTP PIPELINING SUPPORT" << std::endl;
#else
      std::cerr << "###### CREATING NETWORKING SUBSYSTEM WITHOUT HTTP PIPELINING SUPPORT" << std::endl;
#endif
      curl_shared_data_ = curl_share_init();
      curl_share_setopt(curl_shared_data_, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);

      for (int i=0; i<MAX_SIMULTANEOUS_TRANSFERS; ++i) {
        curl_connection_[i] = curl_easy_init();
        curl_easy_setopt(curl_connection_[i], CURLOPT_SHARE, curl_shared_data_);
        curl_easy_setopt(curl_connection_[i], CURLOPT_DNS_CACHE_TIMEOUT, -1); // cache forever
      }
    }
    
    virtual_file_system_network::~virtual_file_system_network() {
      for (int i=0; i<MAX_SIMULTANEOUS_TRANSFERS; ++i) {
        curl_easy_cleanup(curl_connection_[i]);
      }
      curl_share_cleanup(curl_shared_data_);
    }

    FILE* virtual_file_system_network::open(const std::string& url, const char* mode) {
      // Quick and dirty
      // FIXME Remove and replace with curlstream!
      FILE* result = fopen(url.c_str(), "rb");
      if (result) {
	// Assume local file
	SL_TRACE_OUT(1) << "FILE get(" << url << ")" << std::endl;
	SL_TRACE_OUT(1) << "  FILE get success" << std::endl;
      } else {
	SL_TRACE_OUT(1) << "HTTP get(" << url << ")" << std::endl;
	// Try over net
	byte_array_t content;
	curl_easy_setopt(curl_connection_[0], CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_connection_[0], CURLOPT_WRITEFUNCTION, curl_callback_append_to_byte_array);
	curl_easy_setopt(curl_connection_[0], CURLOPT_WRITEDATA, (void *)&content);
	curl_easy_setopt(curl_connection_[0], CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(curl_connection_[0], CURLOPT_VERBOSE, 0);
	CURLcode retcode = curl_easy_perform(curl_connection_[0]);
	if (retcode) {
	  SL_TRACE_OUT(-1) << "Error: easy_perform error " << curl_easy_strerror(retcode) << " when opening " << url << std::endl;
	} else {
	  SL_TRACE_OUT(1) << "HTTP get success" << std::endl;
	  result = tmpfile();
	  fwrite(&(content[0]), content.size(), 1, result);
	  rewind(result);
	}
      }
      return result;
    }

    void virtual_file_system_network::close(FILE* fp) {
      fclose(fp); // deleted when tmp
    }

    void virtual_file_system_network::fetch(const std::string& url, 
					    const key_t& key, 
					    byte_array_t& byte_array) {
      if (sl::matches(url,"http:*")) {
	SL_TRACE_OUT(1) << "HTTP fetch(" << url << " key(" << key[0] << " " << key[1] << " " << key[2] <<"))" << std::endl;
	std::string url_request;
	url_request = url;
	url_request += ".vicrepo";
	url_request += "?i=" + sl::to_string(key[0]);
	url_request += "&j=" + sl::to_string(key[1]);
	url_request += "&k=" + sl::to_string(key[2]);
	curl_easy_setopt(curl_connection_[0], CURLOPT_URL, url_request.c_str());
	curl_easy_setopt(curl_connection_[0], CURLOPT_WRITEFUNCTION, curl_callback_append_to_byte_array);
	curl_easy_setopt(curl_connection_[0], CURLOPT_WRITEDATA, (void *)&byte_array);
	curl_easy_setopt(curl_connection_[0], CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(curl_connection_[0], CURLOPT_VERBOSE, 0);
        curl_easy_setopt(curl_connection_[0], CURLOPT_USERAGENT, "vicsingle");
	byte_array.resize(0);
	CURLcode retcode = curl_easy_perform(curl_connection_[0]);
	if (retcode) {
	  std::cerr << "Fetch error: easy_perform error " << curl_easy_strerror(retcode) << " when fetching from " << url << std::endl;
	  byte_array.clear();
	  byte_array.push_back(0); // ## FIXME: This is a quick'n'dirty way to signal error
	}
	SL_TRACE_OUT(1) << "  ==> SZ= " << byte_array.size() << std::endl;
      } else {
	// Assume local
	SL_TRACE_OUT(1) << "FILE fetch(" << url << " key(" << key[0] << " " << key[1] << " " << key[2] <<"))" << std::endl;
	vfs_local_.fetch(url, key, byte_array);

	SL_TRACE_OUT(1) << "  ==> SZ= " << byte_array.size() << std::endl;
      }
    }

    void  virtual_file_system_network::multiple_fetch(const std::vector<std::string>& urls, 
                                                      const std::vector<key_t>& keys, 
                                                      std::vector<byte_array_t>& byte_arrays) {
#if 1
      if (!(urls.empty()) && sl::matches(urls[0],"http:*")) {
        // Fetch urls in groups of MAX_SIMULTANEOUS_TRANSFERS streams
        
        std::size_t n = urls.size();
        //std::cerr << " ====== MULTI FETCH(" << n << ") STARTED " << std::endl;
        for(std::size_t i = 0; i <n; i+=MAX_SIMULTANEOUS_TRANSFERS) {
          group_fetch(i,
                      urls,
                      keys,
                      byte_arrays);
        }
        //std::cerr << " ====== MULTI FETCH(" << n << ") DONE " << std::endl;
      } else {
	// Assume local
        vfs_local_.multiple_fetch(urls,keys,byte_arrays);
      }
#else
      std::size_t n = urls.size();
      for(std::size_t i = 0; i <n; ++i) {
        fetch(urls[i],keys[i],byte_arrays[i]);
      }
#endif
    }

    void  virtual_file_system_network::group_fetch(std::size_t i0,
                                                   const std::vector<std::string>& urls, 
                                                   const std::vector<key_t>& keys, 
                                                   std::vector<byte_array_t>& byte_arrays) {
      
      // Fetch urls [i0,min(size,i0+MAX_SIMULTANEOUS_TRANSFERS)[
      std::size_t n= urls.size();
      const std::size_t i_end = std::min(i0+std::size_t(MAX_SIMULTANEOUS_TRANSFERS), n);

      //std::cerr << "  GROUP FETCH: "  << "Start fetch: " << i0 << "..." << i_end-1 << " [" << (i_end-i0) << "/" << n << "]" << std::endl;
      
      // Create multi stack
      CURLM *multi_connection = curl_multi_init();
#if HAVE_LIBCURL_HTTP_PIPELINING
      curl_multi_setopt(multi_connection, CURLMOPT_PIPELINING, 1); // FIXME !!!
#endif

      bool transfer_checked[MAX_SIMULTANEOUS_TRANSFERS];
      std::string url_request[MAX_SIMULTANEOUS_TRANSFERS];
      
      // Init connections and add them to stack
      for (std::size_t i=i0; i<i_end; ++i) {
	url_request[i-i0] = urls[i];
	url_request[i-i0] += ".vicrepo";
	url_request[i-i0] += "?i=" + sl::to_string(keys[i][0]);
	url_request[i-i0] += "&j=" + sl::to_string(keys[i][1]);
	url_request[i-i0] += "&k=" + sl::to_string(keys[i][2]);
	curl_easy_setopt(curl_connection_[i-i0], CURLOPT_URL, url_request[i-i0].c_str());
	curl_easy_setopt(curl_connection_[i-i0], CURLOPT_WRITEFUNCTION, curl_callback_append_to_byte_array);
	curl_easy_setopt(curl_connection_[i-i0], CURLOPT_WRITEDATA, (void *)&(byte_arrays[i]));
	curl_easy_setopt(curl_connection_[i-i0], CURLOPT_FAILONERROR, 1);
	curl_easy_setopt(curl_connection_[i-i0], CURLOPT_VERBOSE, 0); // FIXME
        curl_easy_setopt(curl_connection_[i-i0], CURLOPT_USERAGENT, "vicmulti");
	byte_arrays[i].resize(0);

        curl_multi_add_handle(multi_connection, curl_connection_[i-i0]);
        transfer_checked[i-i0] = false;

        SL_TRACE_OUT(1) << "GROUP FETCH: "  << "Start fetch: URL " << i << " " << url_request << std::endl;
      }

      const double start_time_s = real_time_s();
      const double timeout_s = 10.0;

#if 1
      // Busy wait implementation
      bool still_running = true;
      while (still_running) {
	int running_handles = 0;
	CURLMcode performcode = CURLM_CALL_MULTI_PERFORM;
	while (performcode == CURLM_CALL_MULTI_PERFORM) {
	  performcode=curl_multi_perform(multi_connection, &running_handles);
	}
	bool timeout = ((real_time_s()-start_time_s)>(timeout_s));
	still_running = 
	  (running_handles>0) && 
	  (!timeout) &&
	  (performcode == CURLM_OK);
	if (performcode != CURLM_OK) {
	  std::cerr << "  GROUP_FETCH: multi_perform: Err= " << curl_multi_strerror(performcode) << std::endl;
	} else if (timeout) {
	  std::cerr << "  GROUP_FETCH: multi_perform: Timeout" << std::endl;
	}
      }
#else
      // Init transfer
      int still_running = 1;
      while (still_running && ((real_time_s()-start_time_s)<(timeout_s))) {

        // Do next chunk of work
	CURLMcode performcode = CURLM_CALL_MULTI_PERFORM;
	while (performcode == CURLM_CALL_MULTI_PERFORM) {
	  performcode=curl_multi_perform(multi_connection, &still_running);
	}
	if (performcode != CURLM_OK) {
	  std::cerr << "  GROUP_FETCH: multi_perform: Err= " << curl_multi_strerror(performcode) << std::endl;
	}

	// Check whether we still have something to do
	if (still_running) {
	  // === Get file descriptors from the transfers 
	  int maxfd;
	  fd_set fdread,fdwrite,fdexcep;
	  FD_ZERO(&fdread); FD_ZERO(&fdwrite); FD_ZERO(&fdexcep);
	  CURLMcode fdsetcode = curl_multi_fdset(multi_connection, &fdread, &fdwrite, &fdexcep, &maxfd);
         
	  if (fdsetcode != CURLM_OK || maxfd<0) {
	    std::cerr << "  GROUP_FETCH: fdset: N=" << maxfd << " - Err = " << curl_multi_strerror(fdsetcode) << std::endl;
	    still_running = (fdsetcode == CURLM_OK);
	  } else {
	    // set a suitable timeout and select of file descriptors
	    struct timeval timeout;
	    timeout.tv_sec  = 1;
	    timeout.tv_usec = 0;
	    int rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
	    
	    switch(rc) {
	    case -1:
	      // select error
	      std::cerr << "  GROUP FETCH: "  << " Select error" << std::endl;
	      /* select error */
	      // IGNORE: it might be a signal. Wait until a curl_multi_perform error or
	      // we expire allowed time
	      still_running = true; // == Will exit with a timeout
	      break;
	    case 0:
	      // timeout, abort transfer
	      std::cerr << "  GROUP FETCH: "  << " Timeout..." << std::endl;
	      still_running = true; // == Will exit with a timeout
	      break;
	    default:
	      still_running = true;
	    }
	  }
	}
      }
#endif 
      // See how transfer went
      CURLMsg *msg; // for picking up messages with the transfer status 
      int msgs_left; // how many messages are left 
      while ((msg = curl_multi_info_read(multi_connection, &msgs_left))) {
        if (msg->msg == CURLMSG_DONE) {
          for (std::size_t i=i0; i<i_end; ++i) {
            if (msg->easy_handle == curl_connection_[i-i0]) {
              transfer_checked[i-i0] = true;
              CURLcode retcode = msg->data.result;
              if (retcode != CURLE_OK) {
                std::cerr << "  GROUP FETCH: "  << "Error: " << curl_easy_strerror(retcode) << " when fetching from idx=" << i << std::endl; 
                byte_arrays[i].clear();
                byte_arrays[i].push_back(0); // ## FIXME: This is a quick'n'dirty way to signal error
              } else {
                SL_TRACE_OUT(0) << "GROUP FETCH: "  << "OK when fetching from idx=" << i << std::endl; 
              }
            }
          }
        }
      }

      // See whether some connection did not have messages in message queue
      for (std::size_t i=i0; i<i_end; ++i) {
        if (!transfer_checked[i-i0]) {
          std::cerr << "  GROUP FETCH: "  << "Error: transfer not finalized when fetching from idx=" << i << std::endl; // "=>" << urls[i] << std::endl;
            byte_arrays[i].clear();
            byte_arrays[i].push_back(0); // ## FIXME: This is a quick'n'dirty way to signal error
        }
      }
      
      // Cleanup
      bool transfer_ok = true;
      for (std::size_t i=i0; i<i_end; ++i) {
        curl_multi_remove_handle(multi_connection, curl_connection_[i-i0]);

        // Debug
        bool ok_i = byte_arrays[i].size() != 1;
	transfer_ok = transfer_ok && ok_i;
        //std::cerr << "  GROUP FETCH: "  << " Fetch " << i << "/" << byte_arrays.size() << " => " << (ok_i ? "OK" : "FAIL - ") << (ok_i ? "" : url_request[i-i0].c_str()) << std::endl;
	if (!ok_i) std::cerr << "  GROUP FETCH: "  << " Fetch " << i << "/" << byte_arrays.size() << " => " << "FAIL - " << url_request[i-i0].c_str() << std::endl;
      }
      
      curl_multi_cleanup(multi_connection);

      // FIXME - Retry to get failed urls
      if (!transfer_ok) {
	std::cerr << "  GROUP FETCH: FAILED => RETRY" << std::endl;
	for (std::size_t i=i0; i<i_end; ++i) {
	  bool ok_i = byte_arrays[i].size() != 1;
	  if (!ok_i) {
	    curl_easy_setopt(curl_connection_[0], CURLOPT_FRESH_CONNECT, 1);
	    fetch(urls[i], keys[i], byte_arrays[i]);
	    curl_easy_setopt(curl_connection_[0], CURLOPT_FRESH_CONNECT, 0); // FIXME
	    ok_i = byte_arrays[i].size() != 1;
	    std::cerr << "  GROUP FETCH RETRY: "  << " Fetch " << i << "/" << byte_arrays.size() << " => " << (ok_i ? "OK" : "FAIL") << std::endl;
	  }
	}
      } 
    }
    
    void virtual_file_system_network::write(const std::string& url, 
					    const key_t& key, 
					    const byte_array_t& byte_array,
					    uint32_t expected_average_data_size) {
      if (sl::matches(url,"http:*")) {
	SL_FAIL("Http write not implemented!");
      } else {
	// Assume local
	vfs_local_.write(url, key, byte_array, expected_average_data_size);
      }
    }

    std::size_t virtual_file_system_network::curl_callback_append_to_byte_array(void *ptr,
										std::size_t size,
										std::size_t nmemb,
										void *data) {
      byte_array_t *ba = (byte_array_t*)data;
      
      const char *chunk = (const char*)ptr;
      const std::size_t chunk_size = size * nmemb; 
      const std::size_t old_ba_size = ba->size();
      const std::size_t new_ba_size = old_ba_size + chunk_size;
      ba->resize(new_ba_size);
      memcpy(&((*ba)[old_ba_size]), chunk, chunk_size);
      return chunk_size;
    }

  } // namespace vfs
} // namespace vic
