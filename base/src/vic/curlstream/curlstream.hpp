//+++HDR+++
//======================================================================
//
//   This file is part of the CRS4/ViC software.
//   Copyright (C) 2009 by CRS4, Pula, Italy.
//
//   For more information, visit the CRS4 Visual Computing Group
//   web pages at http://www.crs4.it/vic/
//
//   This file is provided AS IS with NO WARRANTY OF ANY KIND, 
//   INCLUDING THE WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS 
//   FOR A PARTICULAR PURPOSE.
//
//======================================================================
//---HDR---//
#ifndef CURLSTREAMBUF_HPP
#define CURLSTREAMBUF_HPP

#include <iostream>
#include <string>
#include <cstdio>

typedef void CURLM; // forward declaration 

namespace vic {

  // -----------------------------------------------------------

  namespace detail {
    class URL_FILE;
  }

  // -----------------------------------------------------------

  class curlstreambuf: public std::streambuf {
  protected:
    CURLM *curl_multi_handle_;

    static size_t curl_write_callback(char *buffer,
				      size_t size,
				      size_t nitems,
				      void *userp);

    static int curl_multi_check_transfer(CURLM* handle);
    static int curl_fill_buffer(CURLM* handle, detail::URL_FILE *file, int want, unsigned int timeout_usec = 30*1000*1000); // FIXME
    static int curl_use_buffer(detail::URL_FILE *file, int want);

    detail::URL_FILE *url_fopen(const char *url,const char *operation);
    int url_fclose(detail::URL_FILE **fileptr);

    // -1 if error
    int url_fwrite(const  void  *ptr, size_t sz,  detail::URL_FILE *file);
    int url_fread(void *ptr, size_t sz, detail::URL_FILE *file);

  protected:
    static const int bufferSize = 47+256;    // size of data buff
    // totals 512 bytes under g++ for igzstream at the end.
    char buffer_[bufferSize]; // data buffer
    bool opened_;             // open/close state of stream
    int mode_;               // I/O mode
    detail::URL_FILE *file_;
    const char* url_;

  public:
    
    curlstreambuf();
    ~curlstreambuf();

    bool is_open() { return opened_; }
    curlstreambuf* open( const char* name, int open_mode);
    curlstreambuf* close();
    
    virtual int     overflow(int c = EOF);
    virtual int     underflow();
    virtual int     sync();
    virtual int     flush_buffer();

  };

  // -----------------------------------------------------------

  class curlstreambase : virtual public std::ios {
  protected:
    curlstreambuf buf_;
  public:
    curlstreambase() { init(&buf_); }
    curlstreambase( const char* name, int open_mode);
    ~curlstreambase();
    void open( const char* name, int open_mode);
    void close();
    curlstreambuf* rdbuf() { 
      return &buf_; 
    }
  };

  // -----------------------------------------------------------
  
  class icurlstream : public curlstreambase, public std::istream {
  public:
    icurlstream() : std::istream( &buf_) {}
    icurlstream(const char* name, int open_mode = std::ios::in)
      : curlstreambase(name, open_mode), std::istream( &buf_) {}
    curlstreambuf* rdbuf() { return curlstreambase::rdbuf(); }
    void open(const char* name, int open_mode = std::ios::in) {
      curlstreambase::open(name, open_mode);
    }
  };

  // -----------------------------------------------------------
  
  class ocurlstream : public curlstreambase, public std::ostream {
  public:
    ocurlstream() : std::ostream( &buf_) {}
    ocurlstream(const char* name, int mode = std::ios::out)
      : curlstreambase( name, mode), std::ostream( &buf_) {}
    curlstreambuf* rdbuf() { return curlstreambase::rdbuf(); }
    void open(const char* name, int open_mode = std::ios::out) {
      curlstreambase::open( name, open_mode);
    }
  };

}

#endif // CURLSTREAMBUF_HPP
