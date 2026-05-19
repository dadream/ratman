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
#include <vic/fetcher/text_fetcher.hpp>
#include <iostream>
#include <string>
#include <vector>


std::string print_status(int status) {
  std::string str="";
  switch(status) {
  case vic::text_fetcher::DONE:
    str="OK";
    break;
  case vic::text_fetcher::FAILED:
    str="FAILED";
    break;
  case vic::text_fetcher::NULL_DATA:
    str="NULL_DATA";
    break;
  case vic::text_fetcher::NOT_FOUND:
    str="NOT_FOUND";
    break;
  default:
    str="DFT";
  }
  return str;
}


int main() {
  std::cerr << "INIT" << std::endl;

  std::vector<std::pair<int, std::string > > url;
  url.push_back(std::make_pair(vic::text_fetcher::NOT_FOUND, "http://it.wikipedia.org/wiki/Testo"));
  url.push_back(std::make_pair(vic::text_fetcher::NOT_FOUND, "http://www.crs4.it/vic"));
  url.push_back(std::make_pair(vic::text_fetcher::NOT_FOUND, "http://www.crs5.it/vic"));
  url.push_back(std::make_pair(vic::text_fetcher::NOT_FOUND, "http://www.crs4.it/vic/news"));
  url.push_back(std::make_pair(vic::text_fetcher::NOT_FOUND, "http://sierra.crs4.it/vicnembo/lucy/12"));
  url.push_back(std::make_pair(vic::text_fetcher::NOT_FOUND, "http://www.crs4.it/vic/cgi-bin/people-page.cgi?name=katia.brigaglia"));

  vic::text_fetcher f;
  if (f.is_http_pipelining()) {
    std::cerr << "HTTP PIPELINING" << std::endl;
  } else {
    std::cerr << "NOT HTTP PIPELINING" << std::endl;
  }    
  // prefetch session (optional)
#if 0
  for (std::size_t i=0; i<url.size(); ++i) {
    f.prefetch(url[i].second);
  }
#endif

  while (true) {
    bool finished=true;
    for (std::size_t i=0; i<url.size(); ++i) {
      SL_TRACE_OUT(1) << "ANALIZING " << url[i].second << std::endl;
      SL_TRACE_OUT(1) << "ANALIZING " << url[i].first << std::endl;
      SL_TRACE_OUT(1) << "Status: " << print_status(url[i].first) << std::endl;
      finished=(finished)&&(url[i].first!=vic::text_fetcher::NOT_FOUND);
      if (url[i].first==vic::text_fetcher::NOT_FOUND) {
	SL_TRACE_OUT(1) << "FETCH " << url[i].second << std::endl;
	vic::text_fetcher::status_data_pair_t d=f.fetch(url[i].second);
	SL_TRACE_OUT(1) << "FETCH END" << std::endl;
	url[i].first=d.first;
	if (d.first == vic::text_fetcher::DONE) {
	  std::cerr << "FOUND " << url[i].second << std::endl;
	  std::cerr << "SIZE " << d.second->size() << std::endl;
	  std::cerr << std::endl;
	} 
      }
    }
    if (finished) break;
  }
  
  for (std::size_t i=0; i<url.size(); ++i) {
    std::cerr << url[i].second;
    if (url[i].first==vic::text_fetcher::DONE) {
      std::cerr << "  OK" << std::endl;
    } else {
      std::cerr << "  NOT OK" << std::endl;
    }
  }
}
