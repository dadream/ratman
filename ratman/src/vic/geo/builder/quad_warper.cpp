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
#include <vic/geo/builder/quad_warper.hpp>
#include <vic/geo/builder/geo_transform.hpp>

// GDAL include
#include <gdal_priv.h>
#include <cpl_string.h>

namespace vic {
  namespace geo {

    quad_warper::quad_warper() :
      last_operation_success_(true),
      need_destroy_(false),
      opts_(0),
      oper_(0) {
    }

    quad_warper::~quad_warper() {
    }

    void quad_warper::create(GDALDataset *src, GDALDataset *dst, 
			     const geo_transform& geo_xform, 
			     const color_remap_transform& color_xform){
      geo_transform_=geo_xform;
      color_remap_transform_=color_xform;

      int src_bands = src->GetRasterCount();
      int dst_bands = dst->GetRasterCount();
      
      if (need_destroy_) destroy();
      need_destroy_ = true;

      opts_ = GDALCreateWarpOptions();
      if (!opts_) {
	last_operation_success_=false;
	last_error_message_=std::string("Unable to create warp options");
	return;
      }

      opts_->dfWarpMemoryLimit = 256.0*1024.0*1024.0; // Default 64MB is really small! 


      /*
       * This option forces the destination image to be initialized to the indicated 
       * value (for all bands) or indicates that it should be initialized to the 
       * NO_DATA value in padfDstNoDataReal/padfDstNoDataImag. If this value 
       * isn't set the destination image will be read and overlayed.
       */
      opts_->papszWarpOptions = CSLSetNameValue(opts_->papszWarpOptions, 
						"INIT_DEST", "0"); // Init to blank output quad

      /*
       * This is a number of extra pixels added around the source window for a 
       * given request, and by default it is 1 to take care of rounding error. 
       * Setting this larger will incease the amount of data that needs to be 
       * read, but can avoid missing source data.
       */
      opts_->papszWarpOptions = CSLSetNameValue(opts_->papszWarpOptions, 
						"SOURCE_EXTRA", "1"); 
      /*
       * Modifies the density of the sampling grid. The default number of 
       * steps is 21. Increasing this can increase the computational cost, 
       * but improves the accuracy with which the source region is computed.
       */
      opts_->papszWarpOptions = CSLSetNameValue(opts_->papszWarpOptions, 
						"SAMPLE_STEPS", "21"); 

      opts_->hSrcDS = src;
      opts_->hDstDS = dst;
      opts_->pTransformerArg = geo_transform_.to_pointer();
      opts_->pfnTransformer = geo_transform_.get_trasformation();
      if (!color_remap_transform_.is_identity()) {
	opts_->pfnPreWarpChunkProcessor=color_remap_transform_.get_trasformation();
	opts_->pPreWarpProcessorArg=color_remap_transform_.to_pointer();
      }

      opts_->nBandCount = dst_bands;

      opts_->panSrcBands = new int[opts_->nBandCount];
      if (dst_bands == 3) {
        if (src_bands == 3) {
	  for (int i = 0; i < 3; ++i) opts_->panSrcBands[i] = i + 1;
	} else if (src_bands == 1) {
	  for (int i = 0; i < 3; ++i) opts_->panSrcBands[i] = 1;
	} else {
	  last_operation_success_=false;
	  last_error_message_=std::string("Unable to create warper object: unsupported band count");
	  return;
	}
      } else if (dst_bands == 1) {
	if (src_bands == 1) {
	  opts_->panSrcBands[0] = 1;
	} else {
	  last_operation_success_=false;
	  last_error_message_=std::string("Unable to create warper object: unsupported band count");
	  return;
	}
      } else {
	last_operation_success_=false;
	last_error_message_=std::string("Unable to create warper object: unsupported band count");
	return;
      }
      
      opts_->panDstBands = new int[opts_->nBandCount];
      for (int i = 0; i < opts_->nBandCount; ++i) opts_->panDstBands[i] = i + 1;
      
      opts_->pfnProgress = GDALDummyProgress;
      opts_->eResampleAlg = GRA_Bilinear; // FIXME Other choices: GRA_Cubic; GRA_NearestNeighbour;
      
      oper_ = new GDALWarpOperation;
      if (oper_->Initialize(opts_) != CE_None) {
	last_operation_success_=false;
	last_error_message_=std::string("Unable to create warper object: GDALCreateWarpOperation failed");
	return;	
      }
    }

    void quad_warper::set_src_geo_matrix(const geo_matrix& mat) {
      geo_transform_.set_src_geo_matrix(mat);
    }

    void quad_warper::set_dst_geo_matrix(const geo_matrix& mat) {  
      geo_transform_.set_dst_geo_matrix(mat);
    }

    void quad_warper::destroy() {
      if (oper_) delete oper_;
      oper_ = 0;
      if (opts_) {
	if (opts_->panSrcBands) delete opts_->panSrcBands;
	opts_->panSrcBands = 0;
	if (opts_->panDstBands) delete opts_->panDstBands;
	opts_->panDstBands = 0;
	GDALDestroyWarpOptions(opts_);
      }
      opts_ = 0;
      need_destroy_ = false;
    }

    void quad_warper::chunk_and_warp_image(int x, int y, int sx, int sy) {
#if 1
      if (oper_->ChunkAndWarpImage(x, y, sx, sy) != CE_None) {
	last_operation_success_=false;
	last_error_message_=std::string("ChunkAndWarpImage failed");
	return;		
      }
#else
      // Seems not working with gdal 1.3.2
      if (oper_->ChunkAndWarpMulti(x, y, sx, sy) != CE_None) {
	last_operation_success_=false;
	last_error_message_=std::string("ChunkAndWarpImage failed");
	return;		
      }
#endif
    }
        
  } // namespace geo
} // namespace vic
