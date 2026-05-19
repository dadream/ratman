#include "curlstream.hpp"
#include <errno.h>
#include <curl/curl.h>
#include "url.hpp"
#include <cassert>
#include <zlib.h>
#include <iostream>
#include <string.h>
#include <cstdlib>

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

  namespace detail {

    static const char gz_magic[2] = {(char)0x1f, (char)0x8b}; /* gzip magic header */

    /* gzip flag byte */
    static const char ASCII_FLAG=0x01; /* bit 0 set: file probably ascii text */
    static const char HEAD_CRC=0x02; /* bit 1 set: header CRC present */
    static const char EXTRA_FIELD=0x04; /* bit 2 set: extra field present */
    static const char ORIG_NAME=0x08; /* bit 3 set: original file name present */
    static const char COMMENT=0x10; /* bit 4 set: file comment present */
    static const char RESERVED=0xE0; /* bits 5..7: reserved */

    enum fcurl_type_e { 
      CFTYPE_NONE=0, 
      CFTYPE_FILE=1, 
      CFTYPE_CURL=2 
    };

    enum fcurl_state { 
      CFSTATE_IDLE=0, 
      CFSTATE_STANDBY=1, 
      CFSTATE_UNCOMPRESSED=2, 
      CFSTATE_COMPRESSED=3,
      CFSTATE_ERROR=4
    };
    
    class  URL_FILE {
    public:
      enum fcurl_type_e type_;      /* type of handle */
      union {
	CURL *curl_;
	gzFile file_;
      } handle_;                    /* handle */
 
      enum fcurl_state state_;
      z_stream z_stream_;           /* libz stream */

      //      char z_buffer_[CURL_MAX_WRITE_SIZE];    /* i/o buffer */

      char* in_buffer_;    /* i/o buffer */
      size_t in_buffer_len_;             /* current data index in in_buffer_*/
      size_t in_buffer_end_;             /* current data index in in_buffer_*/
      size_t in_buffer_pt_;             /* current data index in in_buffer_*/
      char* out_buffer_;    /* i/o buffer */
      size_t out_buffer_len_;             /* current data index in in_buffer_*/

      int z_flags_;
      int z_nflag_;
      int z_skip_header_;
      int error_;

      char* buffer_;       /* buffer to store cached data*/
      size_t buffer_len_;              /* end of data in buffer*/
      size_t buffer_end_;              /* end of data in buffer*/

      int still_running_;           /* Is background url fetch still in progress */
    public:
      URL_FILE() : 
	type_(CFTYPE_NONE),
	state_(CFSTATE_IDLE),
	in_buffer_(0),
	in_buffer_len_(0),
	in_buffer_end_(0),
	in_buffer_pt_(0),
	out_buffer_(0),
	out_buffer_len_(0),
	z_flags_(0),
	z_nflag_(0),
	z_skip_header_(0),
	error_(0),
	buffer_(0),
	buffer_len_(0),
	buffer_end_(0),
	still_running_(0) {
	handle_.curl_=0;
        z_stream_.zalloc = (alloc_func)0;
        z_stream_.zfree = (free_func)0;
        z_stream_.opaque = (voidpf)0;
        z_stream_.next_in = Z_NULL;
        z_stream_.next_out = Z_NULL;
        z_stream_.avail_in = z_stream_.avail_out = 0;
	in_buffer_len_=CURL_MAX_WRITE_SIZE*2;
	in_buffer_=(char *)malloc(in_buffer_len_);
	out_buffer_len_=CURL_MAX_WRITE_SIZE*64;
	out_buffer_=(char *)malloc(out_buffer_len_);
      }

      void cleanup() {
	if (out_buffer_) free(out_buffer_);
	out_buffer_=0;
	out_buffer_len_=0;
	if (in_buffer_) free(in_buffer_);
	in_buffer_=0;
	if (buffer_) free(buffer_);
	buffer_ = 0;
	in_buffer_end_=0;
	in_buffer_len_=0;
	in_buffer_pt_=0;
      }

      ~URL_FILE() {
	cleanup();
      }

    };
    
  }

  static size_t append_items(char** dest_buffer, size_t& len, char* source_buffer, size_t size) {
    size_t result=size;
    char *d=*dest_buffer;
    for(size_t i=0; i<size; ++i) {
      d[i+len]=source_buffer[i];
    }
    //    memcpy(*dest_buffer+len, source_buffer, result);
    len += size;
    return result;
  }

  static size_t add_items(char** dest_buffer, size_t& len, size_t& end, char* source_buffer, size_t size, int& err) {
    char *newbuff=0;
    size_t result=size;
    int rembuff=len - end;//remaining space in buffer
    if (((int)size) > rembuff) {
      newbuff=(char*)realloc(*dest_buffer,len + (size - rembuff));
      if (newbuff==0) {
	std::cerr << "Callback buffer grow failed" << std::endl;
	err=-1;
	result=rembuff;
      } else {
	/* realloc suceeded increase buffer size*/
	len+=size - rembuff;
	*dest_buffer=newbuff;
      }
    }
    memcpy(*dest_buffer+end, source_buffer, result);
    end += size;
    return result;
  }

  size_t curlstreambuf::curl_write_callback(char *buffer,
					    size_t size,
					    size_t nitems,
					    void *userp) {

    detail::URL_FILE *url = (detail::URL_FILE *)userp;
    size *= nitems;
    if(url->state_==detail::CFSTATE_UNCOMPRESSED) {
      return add_items(&url->buffer_, url->buffer_len_, url->buffer_end_, buffer, size, url->error_);      
    }
    if(url->state_==detail::CFSTATE_ERROR) {
      return size;
    }
    size_t lsize = append_items(&url->in_buffer_, url->in_buffer_end_, buffer, size);
    if ( url->state_==detail::CFSTATE_COMPRESSED ) {
      url->z_stream_.next_in=(Bytef*)(url->in_buffer_);
      url->z_stream_.avail_in=url->in_buffer_end_;
    }
    if( url->state_==detail::CFSTATE_IDLE ) {     
      if(url->in_buffer_end_>=10) {
	if ( url->in_buffer_[0] == detail::gz_magic[0] && 
	     url->in_buffer_[1] == detail::gz_magic[1]) {
	  url->state_=detail::CFSTATE_COMPRESSED;
	  url->z_stream_.next_in=(Bytef*)(url->in_buffer_);
	  url->z_stream_.avail_in=url->in_buffer_end_;
	  url->z_stream_.next_out=(Bytef*)(url->out_buffer_);
	  url->z_stream_.avail_out=url->out_buffer_len_;
	  url->error_ = inflateInit2(&(url->z_stream_), MAX_WBITS+16);
	  if(url->error_ != Z_OK) {
	    //	    url->error_=-1;
	    url->state_=detail::CFSTATE_ERROR;
	    std::cerr << "ERROR: structure inizialization failed!" << url->error_ << std::endl;
	    std::cerr << "ZLib internal error msg: " << url->z_stream_.msg << std::endl;
	  }	  
	} else {
	  lsize = add_items(&url->buffer_, url->buffer_len_, url->buffer_end_, url->in_buffer_, url->in_buffer_end_, url->error_);
	  url->state_=detail::CFSTATE_UNCOMPRESSED;
	  return lsize;
	}
      } else {
	return lsize;
      }
    }
    if ( url->state_==detail::CFSTATE_COMPRESSED ) {
      url->error_=Z_OK;
      while(url->z_stream_.avail_in != 0 && url->error_ != Z_STREAM_END) {
	url->error_ = inflate(&(url->z_stream_), Z_NO_FLUSH);
	if(url->error_ < 0) {
	  std::cerr << "ERROR " << zError(url->error_) << std::endl;
	  break;
	}
      }
      url->in_buffer_end_=url->z_stream_.avail_in;
      url->z_stream_.next_in=(Bytef*)url->in_buffer_;
      if(url->out_buffer_len_-url->z_stream_.avail_out>0) {
	add_items(&url->buffer_, url->buffer_len_, url->buffer_end_, url->out_buffer_, 
		  url->out_buffer_len_-url->z_stream_.avail_out, url->error_);
	url->z_stream_.next_out=(Bytef*)url->out_buffer_;
	url->z_stream_.avail_out=url->out_buffer_len_;	
      }
      if(url->error_ != Z_OK && url->error_ != Z_STREAM_END) {
	std::cerr << "ERROR: stream not good! " << url->error_ << std::endl;
	if(url->z_stream_.msg) std::cerr << "ZLib internal error msg: " << url->z_stream_.msg << std::endl;
	url->state_=detail::CFSTATE_ERROR;
      }
    }
    return lsize;
  }
  
  int curlstreambuf::curl_multi_check_transfer(CURLM* handle) {
    int msgsInQueue = 0;
    for (CURLMsg* msg = NULL; (msg = curl_multi_info_read(handle, &msgsInQueue)) != NULL; ) {
#if 0  
    if (msg->msg != CURLMSG_DONE) {
	// nothing to check

      } else 
#endif
	if (msg->data.result == CURLE_OK) {
	// ok
      } else {
	// error!!
	return (int)msg->data.result;
      }
    }
    return (int)CURLE_OK;
  }

  // use to attempt to fill the read buffer up to requested number of bytes
  int curlstreambuf::curl_fill_buffer(CURLM* handle, detail::URL_FILE *file, int want, unsigned int timeout_usec) {

    /* only attempt to fill buffer if transactions still running and buffer
     * doesnt exceed required size already
     */
    if ((!file->still_running_) || (file->buffer_end_ > (size_t)want)) return 0;

    const double start_time_s = real_time_s();

    /* attempt to fill buffer */
    do {
      /* set a suitable timeout to fail on */
      struct timeval timeout;
      timeout.tv_sec  = timeout_usec/1000/1000; 
      timeout.tv_usec = timeout_usec-(unsigned int)(timeout.tv_sec)*1000*1000;
      
      /* get file descriptors from the transfers */
      fd_set fdread, fdwrite, fdexcep;
      FD_ZERO(&fdread); FD_ZERO(&fdwrite); FD_ZERO(&fdexcep);
      int maxfd;
      curl_multi_fdset(handle, &fdread, &fdwrite, &fdexcep, &maxfd);
      int rc = select(maxfd+1, &fdread, &fdwrite, &fdexcep, &timeout);
      
      switch(rc) {
      case -1:
	/* select error */
	// IGNORE: it might be a signal. Wait until a curl_multi_perform error or
	// we expire allowed time
	if ((real_time_s()-start_time_s)>(1.E-6*timeout_usec)) return -1;
	break;

      case 0:
	/* TIMEOUT!*/
	return -1;
	break;
	
      default:
	/* timeout or readable/writable sockets */
	/* note we *could* be more efficient and not wait for
	 * CURLM_CALL_MULTI_PERFORM to clear here and check it on re-entry
	 * but that gets messy */
	{
	  CURLMcode curl_multi_result = curl_multi_perform(handle, &file->still_running_);
	  CURLcode code = ((CURLcode)(curl_multi_check_transfer(handle)));
	  bool ok = (code == CURLE_OK);
	  
	  while (ok && curl_multi_result == CURLM_CALL_MULTI_PERFORM) {
	    curl_multi_result = curl_multi_perform(handle, &file->still_running_);
	    code = ((CURLcode)(curl_multi_check_transfer(handle)));
	    ok = (code == CURLE_OK);
	  }

	  if (!ok) return -1; // transfer error
	}
	break;
      }
    } while (file->still_running_ && (file->buffer_end_ < (size_t)want));

    return 1;
  }

  /* use to remove want bytes from the front of a files buffer */
  int curlstreambuf::curl_use_buffer(detail::URL_FILE *file,int want) {
    /* sort out buffer */
    if ((file->buffer_end_ - want) <=0) {
      /* ditch buffer - write will recreate */
      if (file->buffer_) {
	free(file->buffer_);	
      }
      file->buffer_=0;
      file->buffer_end_=0;
      file->buffer_len_=0;
    } else {
      /* move rest down make it available for later */
      memmove(file->buffer_,
	      &file->buffer_[want],
	      (file->buffer_end_ - want));
      
      file->buffer_end_ -= want;
    }
    return 0;
  }

  static bool is_plain_file(const char *url) {
    return 
      (strstr(url, "://") == 0);
  }	

  static bool is_read_only(const char *operation) {
    return 
      (strncmp(operation, "r", 1) == 0) &&
      (strncmp(operation, "rw", 2) != 0) &&
      (strncmp(operation, "r+", 2) != 0);
  }

  detail::URL_FILE *curlstreambuf::url_fopen(const char *url,const char *operation) {
    if (!url) return 0;

    if (url_) {
      free((void *)url_); url_ = 0;
    }
    url_=strdup(url);

    detail::URL_FILE *result = new detail::URL_FILE();  
    if (!result) return 0;

    if (is_plain_file(url_)) {
      result->type_ = detail::CFTYPE_FILE; 
      result->handle_.file_=gzopen(url_,operation);
      if (!result->handle_.file_) {
	delete result;
	result = 0;
      }
    } else if (is_read_only(operation)) {
      result->type_ = detail::CFTYPE_CURL; /* marked as URL */
      result->handle_.curl_ = curl_easy_init();

      curl_easy_setopt(result->handle_.curl_, CURLOPT_URL, url_);
      curl_easy_setopt(result->handle_.curl_, CURLOPT_WRITEDATA, result);
      curl_easy_setopt(result->handle_.curl_, CURLOPT_VERBOSE, 0);
      curl_easy_setopt(result->handle_.curl_, CURLOPT_WRITEFUNCTION, curl_write_callback);
      curl_easy_setopt(result->handle_.curl_, CURLOPT_FAILONERROR, 1);

      assert(curl_multi_handle_);

      curl_multi_add_handle(curl_multi_handle_, result->handle_.curl_);

      /* lets start the fetch */
      CURLMcode curl_multi_result = curl_multi_perform(curl_multi_handle_, &result->still_running_);
      CURLcode code = ((CURLcode)(curl_multi_check_transfer(curl_multi_handle_)));
      bool ok = (code == CURLE_OK);
      
      while (ok && curl_multi_result == CURLM_CALL_MULTI_PERFORM) {
	curl_multi_result = curl_multi_perform(curl_multi_handle_, &result->still_running_);
	code = ((CURLcode)(curl_multi_check_transfer(curl_multi_handle_)));
	ok = (code == CURLE_OK);
      }
      if (!ok || ((result->buffer_end_ == 0) && (!result->still_running_))) {
	/* if still_running_ is 0 now, we should return NULL */
	
	/* make sure the easy handle is not in the multi handle anymore */
	curl_multi_remove_handle(curl_multi_handle_, result->handle_.curl_);
	
	/* cleanup */
	curl_easy_cleanup(result->handle_.curl_);
	
	delete result;
	
	result = 0;
      }
    } else {
      // write on url not implemented
      result = 0;
    }
    return result;
  }

  int curlstreambuf::url_fclose(detail::URL_FILE **fileptr) {
    int ret=0; /* default is good return */

    if (!fileptr || !(*fileptr)) {
      ret = EOF;
      errno=EBADF;
    } else {
      detail::URL_FILE *file = *fileptr;
      switch(file->type_) {
      case detail::CFTYPE_FILE: {
	ret=gzclose(file->handle_.file_); /* passthrough */
      } break;
	
      case detail::CFTYPE_CURL: {
	/* make sure the easy handle is not in the multi handle anymore */
	curl_multi_remove_handle(curl_multi_handle_, file->handle_.curl_);
	
	/* cleanup */
	curl_easy_cleanup(file->handle_.curl_);
      }	break;
	
      default: {
	/* unknown or supported type - oh dear */
	ret=EOF;
	errno=EBADF;
      } break;
      }
   
      // Delete file structure allocated in url_fopen
      delete file;
      *fileptr = 0;
    }    
 
    return ret;
  }
 

  int curlstreambuf::url_fwrite(const  void  *ptr,  size_t sz,  detail::URL_FILE *file) {
    int result = -1; // Error by default
    
    switch(file->type_) {
    case detail::CFTYPE_FILE:
      {
	size_t nwritten=gzwrite(file->handle_.file_,ptr,sz);
	if (nwritten ==sz) {
	  // OK
	  result = (int)nwritten;
	} 
      }
      break;
      
    case detail::CFTYPE_CURL:
      // NOT IMPLEMENTED!!!
      errno=EBADF;
      break;

    default: /* unknown or supported type - oh dear */
      errno=EBADF;
      break;
      
    }
    return result;
  }

  int curlstreambuf::url_fread(void *ptr, size_t sz, detail::URL_FILE *file) {
    int result = -1;
    switch(file->type_) {
    case detail::CFTYPE_FILE:
      {
	//	clearerr(file->handle_.file_);
	int nread = gzread(file->handle_.file_,ptr,sz);
	if (nread == -1) {
	  result = -1;
	} else {
	  result = nread;
	}
      }
      break;
      
    case detail::CFTYPE_CURL:
      {
	int retcode = curl_fill_buffer(curl_multi_handle_,file,sz);

	// Error
	if (retcode <0) return -1;

	/* check if theres data in the buffer - if not fill_buffer()
	 * either errored or EOF */
	if (!file->buffer_end_)
	  return 0;
	
	/* ensure only available data is considered */
	if (file->buffer_end_ < (size_t)sz)
	  sz = file->buffer_end_;
	
	/* xfer data to caller */
	memcpy(ptr, file->buffer_, sz);
	
	curl_use_buffer(file,sz);
	
	result = (int)sz;    /* number of items - nb correct op - checked
			      * with glibc code*/
      }
      break;
      
    default: /* unknown or supported type - oh dear */
      result= -1;
      errno=EBADF;
      break;
      
    }
    return result;
  }

  //-------------------------------------------------
  // curlstreambuf -- method implementation

  curlstreambuf::curlstreambuf() {
    curl_multi_handle_ = curl_multi_init();
    url_=0;
    file_=0;
    opened_=false;
    mode_=0;
    setp( buffer_, buffer_ + (bufferSize-1));
    setg( buffer_ + 4,     // beginning of putback area
	  buffer_ + 4,     // read position
	  buffer_ + 4);    // end position     
  }

  curlstreambuf::~curlstreambuf() { 
    close(); 
    curl_multi_cleanup(curl_multi_handle_);
    curl_multi_handle_=0;
    if (url_) free((void *)url_);
    url_=0;
  }
  
  curlstreambuf* curlstreambuf::open( const char* name, int open_mode) {
    if ( is_open())
      return (curlstreambuf*)0;
    mode_ = open_mode;
    // no append nor read/write mode
    if ((mode_ & std::ios::ate) || (mode_ & std::ios::app)
        || ((mode_ & std::ios::in) && (mode_ & std::ios::out)))
      return (curlstreambuf*)0;
    char  fmode[10];
    char* fmodeptr = fmode;
    if ( mode_ & std::ios::in)
      *fmodeptr++ = 'r';
    else if ( mode_ & std::ios::out)
      *fmodeptr++ = 'w';
    *fmodeptr++ = 'b';
    *fmodeptr = '\0';
    file_ = url_fopen( name, fmode);
    if (file_ == 0)
      return (curlstreambuf*)0;
    opened_ = true;
    return this;
  }

  curlstreambuf* curlstreambuf::close() {
    if (is_open()) {
      sync();
      opened_ = 0;
      if (url_fclose(&file_) == 0) {
	// Good return
	return this;
      }
    }
    // Error return
    return (curlstreambuf*)0;
  }

  int curlstreambuf::underflow() { // used for input buffer only
    if ( gptr() && ( gptr() < egptr()))
      return * reinterpret_cast<unsigned char *>( gptr());

    if ( ! (mode_ & std::ios::in) || ! opened_)
      return EOF;

    // Josuttis' implementation of inbuf
    int n_putback = gptr() - eback();
    if ( n_putback > 4)
      n_putback = 4;
    memcpy( buffer_ + (4 - n_putback), gptr() - n_putback, n_putback);

    int retcode = url_fread(buffer_+4, bufferSize-4, file_);
    if (retcode <= 0) // FIXME: ERROR or EOF
      return EOF;
    int num = retcode;

    // reset buffer pointers
    setg( buffer_ + (4 - n_putback),   // beginning of putback area
          buffer_ + 4,                 // read position
          buffer_ + 4 + num);          // end of buffer

    // return next character
    return * reinterpret_cast<unsigned char *>( gptr());    
  }

  int curlstreambuf::flush_buffer() {
    // Separate the writing of the buffer from overflow() and
    // sync() operation.
    int w = pptr() - pbase();
    if ( (int)url_fwrite( pbase(), w, file_ ) != w)
      return EOF;
    pbump( -w);
    return w;
  }
  
  int curlstreambuf::overflow( int c) { // used for output buffer only
    if ( ! ( mode_ & std::ios::out) || ! opened_)
      return EOF;
    if (c != EOF) {
      *pptr() = c;
      pbump(1);
    }
    if ( flush_buffer() == EOF)
      return EOF;
    return c;
  }
  
  int curlstreambuf::sync() {
    // Changed to use flush_buffer() instead of overflow( EOF)
    // which caused improper behavior with std::endl and flush(),
    // bug reported by Vincent Ricard.
    if ( pptr() && pptr() > pbase()) {
      if ( flush_buffer() == EOF)
	return -1;
    }
    return 0;
  }

  // -----------------------------------------------------------

  curlstreambase::curlstreambase( const char* name, int mode) {
    init( &buf_);
    open( name, mode);
  }
  
  curlstreambase::~curlstreambase() {
    buf_.close();
  }
  
  void curlstreambase::open( const char* name, int open_mode) {
    if ( ! buf_.open( name, open_mode))
      clear( rdstate() | std::ios::badbit);
  }
  
  void curlstreambase::close() {
    if ( buf_.is_open())
      if ( ! buf_.close())
	clear( rdstate() | std::ios::badbit);
  }

}
