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
#include <vic/geo/builder/quad_processor.hpp>
#include <sl/utility.hpp>
#include <iostream>

// GDAL include
#include <gdal_priv.h>
#include <cpl_string.h>
#include <set>

namespace vic {
  namespace geo {

    quad_processor::quad_processor() :
      last_operation_success_(true),
      min_data_value_(10) {
    }

    quad_processor::~quad_processor() {
    }
    
    GDALDataset *quad_processor::create_from_template(GDALDataset *sample) {
      GDALDataType data_type = sample->GetRasterBand(1)->GetRasterDataType();
      int sx = sample->GetRasterXSize();
      int sy = sample->GetRasterYSize();
      int bands = sample->GetRasterCount();
      GDALDriver *drv=GetGDALDriverManager()->GetDriverByName("MEM");
      GDALDataset *result=drv->Create("", sx, sy, bands, data_type, 0);
      if(!result) {
	last_operation_success_=false;
	last_error_message_=std::string("Unable to create in-memory image");
      }
      return result;
    }
    
    void quad_processor::compute_null_pixel_stats(GDALDataset *in_quad,
						  int* null_pixel_count, 
						  int* non_null_pixel_count) const {
      GDALDataType data_type = in_quad->GetRasterBand(1)->GetRasterDataType();
      if(data_type != GDT_Byte) {
	last_operation_success_=false;
	last_error_message_=std::string("Cannot handle GDAL data_type != GDT_Byte");
	*null_pixel_count = 0;
	*non_null_pixel_count = in_quad->GetRasterXSize()*in_quad->GetRasterYSize();
	return;
      }

      int sx = in_quad->GetRasterXSize();
      int sy = in_quad->GetRasterYSize();
      int bands = in_quad->GetRasterCount();
      int value_count = sx * sy * bands;
      int data_size = value_count * sizeof(unsigned char);
      int threshold = min_data_value()*bands;

      // Read image and compute stats
      unsigned char *quad0 = new unsigned char[data_size];
      in_quad->RasterIO(GF_Read, 
			0, 0, 
			sx, sy, 
			quad0, 
			sx, sy, 
			GDT_Byte,
			bands, 0, 
			0, 0, 0);
      *null_pixel_count = 0;
      *non_null_pixel_count = 0;
      for (int k = 0; k<value_count; k += bands) {
	int value=0;
	for(int b=0; b<bands; ++b) value += static_cast<int>(quad0[k + b]);	    
	if(value >= threshold) {
	  ++(*non_null_pixel_count);
	} else {
	  ++(*null_pixel_count);
	}
      }
      
      delete [] quad0;
    }

    bool quad_processor::is_null(GDALDataset *in_quad) const {
      GDALDataType data_type = in_quad->GetRasterBand(1)->GetRasterDataType();
      if(data_type != GDT_Byte) {
	last_operation_success_=false;
	last_error_message_=std::string("Cannot handle GDAL data_type != GDT_Byte");
	return false;
      }

      int sx = in_quad->GetRasterXSize();
      int sy = in_quad->GetRasterYSize();
      int bands = in_quad->GetRasterCount();
      int value_count = sx * sy * bands;
      int data_size = value_count * sizeof(unsigned char);
      int threshold = min_data_value()*bands;

      // Read image and compute stats
      unsigned char *quad0 = new unsigned char[data_size];
      in_quad->RasterIO(GF_Read, 
			0, 0, 
			sx, sy, 
			quad0, 
			sx, sy, 
			GDT_Byte,
			bands, 0, 
			0, 0, 0);
      bool result = true;
      for (int k = 0; (k<value_count) && result; k += bands) {
	int value=0;
	for(int b=0; b<bands; ++b) value += static_cast<int>(quad0[k + b]);	    
	result = (value < threshold);
      }
      
      delete [] quad0;

      return result;
    }


    void quad_processor::color_remap(GDALDataset *out, GDALDataset *in, 
				     int black_out, int black_in, 
				     int white_out, int white_in, 
				     int below_to_black, int above_to_black) {
      GDALDataType data_type = in->GetRasterBand(1)->GetRasterDataType();
      if(data_type != GDT_Byte) {
	last_operation_success_=false;
	last_error_message_=std::string("Cannot handle GDAL data_type");
	return;
      }
      int sx = in->GetRasterXSize();
      int sy = in->GetRasterYSize();
      int bands = in->GetRasterCount();
      int band_pixel_size = (GDALGetDataTypeSize(data_type) + 7) / 8;
      int pixel_size = bands * band_pixel_size;
      int line_size = sx * pixel_size;      
      unsigned char *d0 = new unsigned char[line_size];
      unsigned char lut[256];
      for (int i = 0; i < black_in; ++i) lut[i] = black_out;
      for (int i = black_in; i < white_in; ++i) lut[i] = black_out + ((white_out - black_out) * (i - black_in)) / (white_in - black_in);
      for (int i = white_in; i < 256; ++i) lut[i] = white_out;
      for (int i = 0; i < below_to_black; ++i) lut[i] = 0; 
      for (int y = 0; y < sy; ++y) {
	in->RasterIO(GF_Read, 0, y, sx, 1, d0, sx, 1, data_type, bands, 0, pixel_size, line_size, band_pixel_size);
	if (above_to_black > 0) {
	  for (int x = 0; x < (sx * bands); x += bands) {
	    int value=0;
	    for(int b=0; b<bands; ++b) value += static_cast<int>(d0[x + b]);
	    if(value >= above_to_black*bands) {
	      for(int b=0; b<bands; ++b) d0[x + b] = 0;
	    } else {
	      for(int b=0; b<bands; ++b) d0[x + b] = lut[d0[x+b]];
	    }
	  }
	} else {
	  for (int x = 0; x < (sx * bands); ++x) d0[x] = lut[d0[x]];	  
	}
	out->RasterIO(GF_Write, 0, y, sx, 1, d0, sx, 1, data_type, bands, 0, pixel_size, line_size, band_pixel_size);
      }
      delete [] d0;	
    }

    
    void quad_processor::global_remap_nodata_to_black(GDALDataset *inout, 
						      int black_out, int black_in, 
						      int white_out, int white_in,
						      int below_to_black, int above_to_black) {
      /// !!!!!!!!!!!! ASSUMES RASTER IS IN MEMORY!
      GDALDataType data_type = inout->GetRasterBand(1)->GetRasterDataType();
      if(data_type != GDT_Byte) {
	last_operation_success_=false;
	last_error_message_=std::string("Cannot handle GDAL data_type");
	return;
      }

      // compute lut
      unsigned char lut[256];
      for (int i = 0; i < black_in; ++i) {
	lut[i] = black_out;
      }
      for (int i = black_in; i < white_in; ++i) {
	float lf_i = float(black_out) + (float(white_out - black_out) * float(i - black_in)) / float(white_in - black_in);
	int li_i = sl::median(int(lf_i+0.5f), black_out, white_out-1);
	lut[i] = li_i;
      }
      for (int i = white_in; i < 256; ++i) {
	lut[i] = white_out;
      }

      int sx = inout->GetRasterXSize();
      int sy = inout->GetRasterYSize();
      int bands = inout->GetRasterCount();
      int band_pixel_size = (GDALGetDataTypeSize(data_type) + 7) / 8;
      int pixel_size = bands * band_pixel_size;
      int line_size = sx * pixel_size;      

      // mask of bits: out is true for elements adjacent to boundary and whose value is considered not valid
      int pixel_count = sx*sy;
      std::vector<bool> out(pixel_count);
      for(int i = 0; i < pixel_count; ++i) {
	out[i] = false;
      }
      unsigned char *d0 = new unsigned char[line_size];

      // scan top2bottom: mark out values
      for (int y = 0; y < sy; ++y) {
	inout->RasterIO(GF_Read, 0, y, sx, 1, d0, sx, 1, data_type, bands, 0, pixel_size, line_size, band_pixel_size);
	int line_offset = y*sx;
	int first_valid = 0;
	int last_valid = sx-1;
	// scan left2right
	for (int xx = 0; xx < sx; ++xx) {
	  int x = xx*bands;
	  int value=0;
	  for(int b=0; b<bands; ++b) value += static_cast<int>(d0[x + b]);
	  if (value >= above_to_black*bands || value <= below_to_black*bands) {
	    //    for(int b=0; b<bands; ++b) d0[x + b] = 0; // NODATA
	    out[line_offset+xx] = true;
	  } else {
	    // DATA
	    first_valid = xx;
	    break;
	  }
	}

	// scan right2left
	for (int xx = 0; xx < sx; ++xx) {
	  int x = (sx-1-xx)*bands;
	  int value=0;
	  for(int b=0; b<bands; ++b) value += static_cast<int>(d0[x + b]);
	  if (value >= above_to_black*bands || value <= below_to_black*bands) {
	    out[line_offset+sx-1-xx] = true;
	  } else {
	    // DATA
	    last_valid = sx-1-xx;
	    break;
	  }
	}

	// scan inside
	for (int xx = first_valid; xx <= last_valid; ++xx) {
	  int x = xx*bands;
	  
	  // mark as out if it is on first row || pixel above this is also marked as out.
	  if (y == 0 || out[line_offset-sx+xx]) {	  
	    int value=0;
	    for(int b=0; b<bands; ++b) value += static_cast<int>(d0[x + b]);
	    if (value >= above_to_black*bands || value <= below_to_black*bands) {
	      out[line_offset+xx] = true;
	    }
	  } 
	}
      }

      // scan bottom2top, mark out values and write
      for (int y = sy-1; y >= 0; --y) {
	inout->RasterIO(GF_Read, 0, y, sx, 1, d0, sx, 1, data_type, bands, 0, pixel_size, line_size, band_pixel_size);
	int line_offset = y*sx;
	
	for (int xx = 0; xx < sx; ++xx) {
	  int x = xx*bands;
	  if ((y == sy-1 || out[line_offset+sx+xx]) && !out[line_offset+xx]) {
	    int value=0;
	    for(int b=0; b<bands; ++b) value += static_cast<int>(d0[x + b]);
	    if (value >= above_to_black*bands || value <= below_to_black*bands) {
	      out[line_offset+xx] = true;
	    }
	  }
	 
	  if (out[line_offset+xx]) {
	    for(int b=0; b<bands; ++b) d0[x + b] = 0; // NODATA
	  } else {
	    // REMAP COLOR
	    for(int b=0; b<bands; ++b) {
	      d0[x + b] = lut[d0[x + b]];
	    }
	  }
	}

	inout->RasterIO(GF_Write, 0, y, sx, 1, d0, sx, 1, data_type, bands, 0, pixel_size, line_size, band_pixel_size);
      }
      delete[] d0;
    }

#if 1
    // Old version


    void quad_processor::coarsen(GDALDataset *out, const std::vector<GDALDataset *> &samples) {
      GDALDataType data_type = out->GetRasterBand(1)->GetRasterDataType();
      if(data_type != GDT_Byte) {
        last_operation_success_=false;
        last_error_message_=std::string("Cannot handle GDAL data_type");
        return;
      }
      if(samples.size() != 4) {
        last_operation_success_=false;
        last_error_message_=std::string("Cannot subsample: samples missing");
        return;
      }
      int sx = out->GetRasterXSize();
      int sy = out->GetRasterYSize();
      int bands = out->GetRasterCount();
      int band_pixel_size = (GDALGetDataTypeSize(data_type) + 7) / 8;
      int pixel_size = bands * band_pixel_size;
      int line_size = sx * pixel_size;

      unsigned char *d0 = new  unsigned char[line_size * 2];
      unsigned char *d1 = new  unsigned char[line_size];

      for (std::size_t ns = 0; ns < samples.size(); ++ns)
        if (!samples[ns]) {
          memset(d1, 0, line_size / 2);
          for (int y = 0; y < sy; y += 2) {
            int tx = (ns & 1) ? (sx / 2) : 0;
            int ty = y / 2 + ((ns & 2) ? (sy / 2) : 0);
            out->RasterIO(GF_Write, tx, ty, sx / 2, 1, d1, sx / 2, 1, data_type, bands, 0, pixel_size, line_size, band_pixel_size);
          }
        } else {
          for (int y = 0; y < sy; y += 2) {
            samples[ns]->RasterIO(GF_Read, 0, y, sx, 2, d0, sx, 2, data_type, bands, 0, pixel_size, line_size, band_pixel_size);
            if (bands == 3) {
              unsigned char ic[4][3];
              for (int j = 0, k = 0; j < (sx * 3); j += 6, k += 3) {
                for (int l = 0; l < 3; ++l) ic[0][l] = d0[j + l];
                for (int l = 0; l < 3; ++l) ic[1][l] = d0[j + l + 3];
                for (int l = 0; l < 3; ++l) ic[2][l] = d0[j + l + line_size];
                for (int l = 0; l < 3; ++l) ic[3][l] = d0[j + l + line_size + 3];
                for (int l = 0; l < 3; ++l) d1[k + l] = (static_cast<unsigned int>(2) + ic[0][l] + ic[1][l] + ic[2][l] + ic[3][l]) >> 2;

              }
            } else if (bands == 1) {
              for (int j = 0, k = 0; j < sx; j += 2, k += 1) {
                if ((d0[j] < min_data_value()) ||
                    (d0[j + 1] < min_data_value()) ||
                    (d0[j + line_size] < min_data_value()) ||
                    (d0[j + line_size + 1] < min_data_value())) d1[k] = 0;
                else d1[k] = (static_cast<unsigned int>(2) + d0[j] + d0[j + 1] + d0[j + line_size] + d0[j + line_size + 1]) >> 2;
              }
            }
            int tx = (ns & 1) ? (sx / 2) : 0;
            int ty = y / 2 + ((ns & 2) ? (sy / 2) : 0);
            out->RasterIO(GF_Write, tx, ty, sx / 2, 1, d1, sx / 2, 1, data_type, bands, 0, pixel_size, line_size, band_pixel_size);
          }
        }
      delete d1;
      delete d0;
    }

#else

    void quad_processor::coarsen(GDALDataset *out, const std::vector<GDALDataset *> &in_quads) {
      GDALDataType data_type = out->GetRasterBand(1)->GetRasterDataType();
      if(data_type != GDT_Byte) {
	last_operation_success_=false;
	last_error_message_=std::string("Cannot handle GDAL data_type");
	return;
      }
      if(in_quads.size() != 4) {
	last_operation_success_=false;
	last_error_message_=std::string("Cannot subsample: quads missing");
	return;
      }

      int sx = out->GetRasterXSize();
      int sy = out->GetRasterYSize();
      int bands = out->GetRasterCount();
      int pixel_count = sx*sy;
      int value_count = sx * sy * bands;
      int quad_size = value_count * sizeof(unsigned char);
            
      unsigned char *four_quads = new unsigned char[2*2*pixel_count*bands];
      std::size_t    four_quads_offset[4];
      four_quads_offset[0] = (0*sx*sy   );
      four_quads_offset[1] = (0*sx*sy+sx);
      four_quads_offset[2] = (2*sx*sy   );
      four_quads_offset[3] = (2*sx*sy+sx);

      // Read 4 quads into a single buffer
      for (int i=0; i<4; ++i) {
	if (in_quads[i]) {
	  in_quads[i]->RasterIO(GF_Read, 
				0, 0, 
				sx, sy, 
				four_quads+four_quads_offset[i],
				sx, sy, 
				GDT_Byte,
				bands, 0, 
				0,       // pix 
				2*sx,    // line
				0);      // chan
	} else {
	  for (int b=0; b<bands; ++b) {
	    memset(four_quads+four_quads_offset[i]+b*4*pixel_count, 0, pixel_count);
	  }
	}	  
      }

      // Zoom 4 quads into single half res one 
      unsigned char *zoomed_quad = new unsigned char[quad_size];
      for (int b=0; b<bands; ++b) {
	for (int y=0; y<sy; ++y) {
	  for (int x=0; x<sx; ++x) {
	    unsigned int i00 = four_quads[((2*y+0)*2*sx+(2*x+0))+b*4*pixel_count];
	    unsigned int i01 = four_quads[((2*y+0)*2*sx+(2*x+1))+b*4*pixel_count];
	    unsigned int i10 = four_quads[((2*y+1)*2*sx+(2*x+0))+b*4*pixel_count];
	    unsigned int i11 = four_quads[((2*y+1)*2*sx+(2*x+1))+b*4*pixel_count];
	    zoomed_quad[(y*sx+x)+b*pixel_count] = (static_cast<unsigned int>(2) + i00 + i01 + i10 + i11) >> 2;
	  }
	}
      }
  
      // Write result
      out->RasterIO(GF_Write, 
		    0, 0, 
		    sx, sy, 
		    zoomed_quad, 
		    sx, sy, 
		    GDT_Byte,
		    bands, 0, 
		    0,
		    0,
		    0);

      delete [] four_quads;
      delete [] zoomed_quad;
    }
#endif

    void quad_processor::coarsen(GDALDataset *out, GDALDataset *sample00, GDALDataset *sample01, GDALDataset *sample10, GDALDataset *sample11) {
      std::vector<GDALDataset *> v;
      v.push_back(sample00);
      v.push_back(sample01);
      v.push_back(sample10);
      v.push_back(sample11);
      coarsen(out, v);
    }

#if 1

    // Old version

    void quad_processor::combine(GDALDataset *out, const std::vector<GDALDataset *> &samples) {
      GDALDataType data_type = out->GetRasterBand(1)->GetRasterDataType();
      if(data_type != GDT_Byte) {
	last_operation_success_=false;
	last_error_message_=std::string("Cannot handle GDAL data_type");
	return;
      }
      if(samples.empty()) {
	last_operation_success_=false;
	last_error_message_=std::string("Cannot combine samples");
	return;
      }

      int sx = out->GetRasterXSize();
      int sy = out->GetRasterYSize();
      int bands = out->GetRasterCount();
      int band_pixel_size = (GDALGetDataTypeSize(data_type) + 7) / 8;
      int pixel_size = bands * band_pixel_size;
      int line_size = sx * pixel_size;
      int threshold = min_data_value()*bands;

      unsigned char *d0 = new  unsigned char[line_size];
      unsigned char *d1 = new  unsigned char[line_size];

      for (int y = 0; y < sy; ++y) {
	samples[0]->RasterIO(GF_Read, 0, y, sx, 1, d0, sx, 1, data_type, bands, 0, pixel_size, line_size, band_pixel_size);
	// Remove compression errors
	for (int x = 0; x < (sx * bands); x += bands) {
	  int value=0;
	  for(int b=0; b<bands; ++b) value += static_cast<int>(d0[x + b]);	    
	  if (value < threshold) {
	    for(int b=0; b<bands; ++b) d0[x + b] = 0;
	  }
	}

	// Add other samples
	for (std::size_t i = 1; i < samples.size(); ++i) {
	  samples[i]->RasterIO(GF_Read, 0, y, sx, 1, d1, sx, 1, data_type, bands, 0, pixel_size, line_size, band_pixel_size);
	  for (int x = 0; x < (sx * bands); x += bands) {
	    int value=0;
	    for (int b=0; b<bands; ++b) value += static_cast<int>(d1[x + b]);	    
	    if (value >= threshold) {
	      for (int b=0; b<bands; ++b) d0[x + b] = d1[x + b];
	    }
	  }
	}

	out->RasterIO(GF_Write, 0, y, sx, 1, d0, sx, 1, data_type, bands, 0, pixel_size, line_size, band_pixel_size);
      }

      delete d0;
      delete d1;      
    }

#else

    void quad_processor::combine(GDALDataset *out, 
				 const std::vector<GDALDataset *> &in_quads) {
      GDALDataType data_type = out->GetRasterBand(1)->GetRasterDataType();
      if(data_type != GDT_Byte) {
	last_operation_success_=false;
	last_error_message_=std::string("Cannot handle GDAL data_type");
	return;
      }
      if(in_quads.empty()) {
	last_operation_success_=false;
	last_error_message_=std::string("Cannot combine quads");
	return;
      }

      int sx = out->GetRasterXSize();
      int sy = out->GetRasterYSize();
      int bands = out->GetRasterCount();
      int value_count = sx * sy * bands;
      int data_size = value_count * sizeof(unsigned char);
      int threshold = min_data_value()*bands;

      // Read base image, removing compression errors
      unsigned char *quad0 = new unsigned char[data_size];
      in_quads[0]->RasterIO(GF_Read, 
			    0, 0, 
			    sx, sy, 
			    quad0, 
			    sx, sy, 
			    GDT_Byte,
			    bands, 0, 
			    0, 0, 0);
      for (int k = 0; k<value_count; k += bands) {
	int value=0;
	for(int b=0; b<bands; ++b) value += static_cast<int>(quad0[k + b]);	    
	if (value < threshold) {
	  for(int b=0; b<bands; ++b) quad0[k + b] = 0;
	}
      }
     
      // Combine other images
      unsigned char *quad1 = new unsigned char[data_size];
      for (std::size_t i = 1; i < in_quads.size(); ++i) {
	in_quads[i]->RasterIO(GF_Read, 
			      0, 0, 
			      sx, sy, 
			      quad1, 
			      sx, sy, 
			      GDT_Byte,
			      bands, 0, 
			      0, 0, 0);
	for (int k = 0; k<value_count; k += bands) {
	  int value=0;
	  for(int b=0; b<bands; ++b) value += static_cast<int>(quad1[k + b]);	    
	  if (value >= threshold) {
	    for(int b=0; b<bands; ++b) quad0[k + b] = quad1[k + b];
	  }
	}
      }

      // Write result
      out->RasterIO(GF_Write, 
		    0, 0, 
		    sx, sy, 
		    quad0, 
		    sx, sy, 
		    GDT_Byte,
		    bands, 0, 
		    0, 0, 0);

      // Delete temporaries
      delete [] quad0;
      delete [] quad1;      
    }

#endif

    void quad_processor::combine(GDALDataset *out, GDALDataset *sample0, GDALDataset *sample1) {
      std::vector<GDALDataset *> v;
      v.push_back(sample0);
      v.push_back(sample1);
      combine(out, v);      
    }

    
    void quad_processor::print_info(GDALDataset *sample) const {
      if(!sample) {
	last_operation_success_=false;
	last_error_message_=std::string("Sample is NULL");
	return;
      }
      GDALDataType data_type = sample->GetRasterBand(1)->GetRasterDataType();
      if(data_type != GDT_Byte) {
	last_operation_success_=false;
	last_error_message_=std::string("Cannot handle GDAL data_type");
	return;
      }
      int sx = sample->GetRasterXSize();
      int sy = sample->GetRasterYSize();
      int bands = sample->GetRasterCount();
      int band_pixel_size = (GDALGetDataTypeSize(data_type) + 7) / 8;
      int pixel_size = bands * band_pixel_size;
      int line_size = sx * pixel_size;      

      std::cout << "quad_processor - Raster Image Information" << std::endl;
      std::cout << "Size: " << sx << "X" << sy << std::endl;
      std::cout << "Number of bands: " << bands << std::endl;
      std::cout << "Band pixel size: " << band_pixel_size << std::endl;
      std::cout << "Image pixel size: " << pixel_size << std::endl;
      std::cout << "Line size: " << line_size << std::endl;
      
      switch (data_type) {
      case GDT_Byte: {
	std::cout << "Data type: 8 bit unsigned integer" << std::endl;
      } break;
      case GDT_UInt16: {
	std::cout << "Data type: 16 bit unsigned integer" << std::endl;
      } break;
      case GDT_Int16: {
	std::cout << "Data type: 16 bit integer" << std::endl;
      } break;
      case GDT_UInt32: {
	std::cout << "Data type: 32 bit unsigned integer" << std::endl;
      } break;
      case GDT_Int32: {
	std::cout << "Data type: 32 bit integer" << std::endl;
      } break;
      case GDT_Float32: {
	std::cout << "Data type: 32 bit float" << std::endl;
      } break;
      case GDT_Float64: {
	std::cout << "Data type: 64 bit float" << std::endl;
      } break;
      case GDT_CInt16: {
	std::cout << "Data type: Complex Int16" << std::endl;
      } break;
      case GDT_CInt32: {
	std::cout << "Data type: Complex Int32" << std::endl;
      } break;
      case GDT_CFloat32: {
	std::cout << "Data type: Complex Float32" << std::endl;
      } break;
      case GDT_CFloat64: {
	std::cout << "Data type: Complex Float64" << std::endl;
      } break;
      default: {
	std::cout << "Data type: not known" << std::endl;	
      } break;
      }
      std::cout << "---------------------------------------------------------" << std::endl;
    }
    
  } // namespace geo
} // namespace vic
