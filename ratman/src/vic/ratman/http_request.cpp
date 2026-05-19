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
#include <vic/ratman/http_request.hpp>
#include <string.h>
#include <curl/curl.h>
#include <curl/easy.h>
#include <iostream>

namespace ratman {

  static std::size_t curl_callback_append_to_string(void *ptr,
						    std::size_t size,
						    std::size_t nmemb,
						    void *data) {
    QString *str = (QString *)data;
    
    char *chunk = (char*)ptr;
    std::size_t chunk_size = size * nmemb;
    for (std::size_t i=0; i<chunk_size; ++i) {
      str->append(chunk[i]);
    }
    return chunk_size;
  }

  http_request::http_request(const QString& url, unsigned id) {
    id_=id;
    error_=false;
    url_ = new char[url.size()+2];
    QByteArray qb=url.toLocal8Bit();
    strcpy(url_,qb.constData());
  }
  
  http_request::~http_request() {
    if(url_) delete [] url_;
    url_=0;
  }
  
  void http_request::get_url() {
    CURL *curl = curl_easy_init();
    if(!curl) {
      error_=true;
      error_msg_ = QString("Error: Cannot initialize URL session");
      return;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url_);
    char error_msg_raw[CURL_ERROR_SIZE];
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, error_msg_raw);  
    
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_callback_append_to_string);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&output_data_);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    CURLcode err_value;
    mutex_.lock(); {
      err_value=curl_easy_perform(curl);
    } mutex_.unlock();
    if(err_value) {
      error_=true;
      error_msg_ = QString(error_msg_raw);
      std::cerr << "ERRORE " << error_msg_.toStdString() << std::endl;
    }
    curl_easy_cleanup(curl);
  }
  
  void http_request::run() {
    // set background priority
    cbdam::background_thread::run();

    get_url();
  }

}
