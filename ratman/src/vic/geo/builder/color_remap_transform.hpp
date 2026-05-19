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
#ifndef COLOR_REMAP_TRANSFORM_HPP
#define COLOR_REMAP_TRANSFORM_HPP

#include <vic/geo/builder/geo_utility.hpp>
#include <sl/serializer.hpp>
#include <gdal_alg.h>
#include <string>

namespace vic {
  namespace geo {
    class color_remap_transform {
    public:
      typedef color_remap_transform this_t;
      typedef CPLErr (*color_transform_callback_t)(void *p_kern, void *p_arg);
    protected:
      int black_out_;
      int black_in_;
      int white_out_;
      int white_in_;
      int below_to_black_;
      int above_to_black_;
      unsigned char lut_[256];
    protected:

      static CPLErr gdal_color_transform_callback(void *pKern, void *pArg);

      CPLErr gdal_color_transform(void *pKern) const;

    protected:
      void lut_rebuild();

    public:
      color_remap_transform(int black_out=0, int black_in=0, 
			    int white_out=255, int white_in=255, 
			    int below_to_black = 0, 
			    int above_to_black = -1);

      virtual ~color_remap_transform();
  
    public: // Serialization
    
      void store_to(sl::output_serializer& s) const;
    
      void retrieve_from(sl::input_serializer& s);
      
    public:

      inline color_transform_callback_t get_trasformation() const { 
	return gdal_color_transform_callback; 
      }

      inline this_t* to_pointer() const { 
	return const_cast<this_t*>(this);
      }

      inline bool is_identity() const {
	return	
	  (black_out_ == 0) &&
	  (black_in_ == 0) &&
	  (white_out_ == 255) &&
	  (white_in_ == 255) &&
	  (below_to_black_ == 0) &&
	  (above_to_black_ <0 || above_to_black_ >255);
      }

    };

  } // namespace geo
} // namespace vic

#endif
