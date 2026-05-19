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
#ifndef CBDAM_CBDAM_DIAMOND_FETCHER_HPP
#define CBDAM_CBDAM_DIAMOND_FETCHER_HPP

#include <vic/cbdam/base/geodata_fetcher.hpp>
#include <vic/cbdam/base/repository_parameters.hpp>
#include <sl/quantized_array_codec.hpp>
#include <vic/vfs/repository.hpp>

namespace cbdam {

  class cbdam_diamond_fetcher: public geodata_fetcher<sl::dense_array<int32_t,2,void> > {
  public:
    typedef sl::dense_array<int32_t,2,void>            array2_t;
    typedef geodata_fetcher<array2_t>                  super_t;
    typedef super_t::key_t                             key_t;
    typedef super_t::status_t                          status_t;
    typedef super_t::value_t                           value_t;

  protected:
    mutable sl::quantized_array_codec	codec_; // FIXME CHECK
    repository_parameters	repository_parameters_;
    vic::vfs::repository        data_repo_;
   
  public:
    cbdam_diamond_fetcher(const std::string& url,
			    const std::string& default_about = "CBDAM Elevation Layer");

    virtual ~cbdam_diamond_fetcher();

    const repository_parameters& get_repository_parameters() const {
      return repository_parameters_;
    }

  protected: // Data decoding

    virtual value_t* decoded(const uint8_t* buf,
			     std::size_t buf_size) const;

  protected: // Direct I/O protocol implementation

    virtual void direct_connect();

    virtual void direct_disconnect();

    virtual void direct_send_requests();

    virtual void direct_receive();

  protected: // HTTP protocol implementation
    
    virtual void http_connect();

    virtual void http_disconnect();

    virtual std::string http_url_string(const key_t& k, const aabox2d_t& uv_box) const;

  };
  
} // namespace cbdam 

#endif // CBDAM_CBDAM_DIAMOND_FETCHER_HPP
