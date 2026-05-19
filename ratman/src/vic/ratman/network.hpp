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
#ifndef VIC_RATMAN_NETWORK_HPP
#define VIC_RATMAN_NETWORK_HPP

#include <iostream>

class QImage;

namespace ratman {

  /**
   * Network: fetches generic data over the network.
   */
  class network {
  public: 
    typedef network this_t;
  private: // disable copy
    network(const network&);
    network operator=(const network&);
  protected:
    network(); // protected
  public:

    static this_t* instance();
    
  public:

    /// Return a new stream open for reading
    std::istream* istream_open(const std::string& url) const;

    /// Close and delete the stream open by istream_open
    void istream_close(std::istream* strm) const;

    /// Return a new qimage from the network. Null if not found
    QImage* qimage_fetch(const std::string& url) const;
  };

}

#endif
