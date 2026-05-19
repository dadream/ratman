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
#include <vic/ratman/network.hpp>
#include <vic/curlstream/curlstream.hpp>
#include <QImage>
#include <QByteArray>
#include <iostream>

namespace ratman {

  network::network() {
  }

  network::this_t* network::instance() {
    static this_t* result = 0;
    if (!result) {
      result = new this_t;
    }
    return result;
  }

  std::istream* network::istream_open(const std::string& url) const {
    vic::icurlstream* ifile = new vic::icurlstream();
    ifile->open(url.c_str());
    return ifile;
  }
  
  void network::istream_close(std::istream* strm) const {
    if(strm) {
      vic::icurlstream* ifile = dynamic_cast<vic::icurlstream*>(strm);
      if (ifile) { 
	ifile->close();
      }
      delete strm;
    }
  }

  QImage* network::qimage_fetch(const std::string& url) const {
    QImage* result=0;

    vic::icurlstream ifile;
    ifile.open(url.c_str());
    if (!ifile) {
      std::cerr << "qimage_fetch: Failed to open " << url << std::endl;
    } else {
      QByteArray data;
      while (ifile.good()) {
	char c;
	ifile.get(c);
	if (ifile.good()) data.append(c);
      }
      if (ifile.fail() &&!ifile.eof()) {
	std::cerr << "qimage_fetch: Read error for " << url << std::endl;
      } else {
	std::cerr << "qimage_fetch: Creating image from data_sz=" << data.size() << std::endl; 
	result = new QImage(QImage::fromData(data));
	std::cerr << "qimage_fetch: Image " << result->width() << "x" << result->height() << std::endl;
      }
      ifile.close();
    }
    return result;
  }
  

}
