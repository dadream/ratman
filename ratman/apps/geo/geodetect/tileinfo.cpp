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
#include "tileinfo.hpp"

// ------ opendir, readdir
#include <sys/types.h>
#include <dirent.h>
#include <ogr_srs_api.h>

namespace vic {
  namespace geo {

    tileinfo::tileinfo(const std::string& dirname, const std::string& pattern) {
      process_directory(dirname, pattern);
    }

    tileinfo::~tileinfo() {
    }

    std::size_t tileinfo::search(double x, double y) {
      std::size_t result = std::size_t(-1);
      sl::point2d p; p = x, y;
      for(std::size_t i=0; i<boxes_.size() && result == std::size_t(-1); ++i) {
	if (boxes_[i].contains(p)) {
	  result=i;
	}
      }
      return result;
    }

    sl::point2d tileinfo::get_corner(GDALDataset* tile, double x, double y) const {
      double      geo_x, geo_y;
      const char  *psz_proj=0;
      double      geo_mat[6];
      OGRCoordinateTransformationH hTransform = 0;

      /* -------------------------------------------------------------------- */
      /*      Transform the point into georeferenced coordinates.             */
      /* -------------------------------------------------------------------- */
      if( GDALGetGeoTransform( tile, geo_mat ) == CE_None ) {
	psz_proj = GDALGetProjectionRef(tile);	
	geo_x = geo_mat[0] + geo_mat[1] * x + geo_mat[2] * y;
	geo_y = geo_mat[3] + geo_mat[4] * x + geo_mat[5] * y;
      } else {
	geo_x = x;
	geo_y = y;
      }

      /* -------------------------------------------------------------------- */
      /*      Setup transformation to lat/long.                               */
      /* -------------------------------------------------------------------- */
      if( psz_proj != NULL && strlen(psz_proj) > 0 ) {
	OGRSpatialReferenceH hProj;
	OGRSpatialReferenceH hLatLong = NULL;
	hProj = OSRNewSpatialReference( psz_proj );
	if( hProj != NULL ) {
	  hLatLong = OSRCloneGeogCS( hProj );
	}
	if( hLatLong != NULL ) {
	  CPLPushErrorHandler( CPLQuietErrorHandler );
	  hTransform = OCTNewCoordinateTransformation( hProj, hLatLong );
	  CPLPopErrorHandler(); 
	  OSRDestroySpatialReference( hLatLong );
	}
	if( hProj != NULL ) {
	  OSRDestroySpatialReference( hProj );
	}
      }
      
      /* -------------------------------------------------------------------- */
      /*      Transform to latlong and report.                                */
      /* -------------------------------------------------------------------- */
      if( hTransform != NULL ) {
	OCTTransform(hTransform,1,&geo_x,&geo_y,NULL);
	OCTDestroyCoordinateTransformation( hTransform );
      }
   
      return sl::point2d(geo_x, geo_y);
    }

    void tileinfo::process_tile(const std::string& fname) {
      std::cout << "TILEINFO: Processing: " << fname << std::endl;
      GDALDataset* tile = reinterpret_cast<GDALDataset*>(GDALOpen(fname.c_str(), GA_ReadOnly));
      if (tile == NULL) {
        std::cerr << "GDALOpen failed - " 
		  << CPLGetLastErrorNo()
		  << " " << CPLGetLastErrorMsg() 
		  << std::endl;
      } else {
	// upper-left
	// get_corner( tile, 0.0, 0.0 );
	// lower-left
	// get_corner( tile, 0.0, GDALGetRasterYSize(tile));
	// upper-right
	// get_corner( tile, GDALGetRasterXSize(tile), 0.0 );
	// lower-right
	// get_corner( tile, GDALGetRasterXSize(tile), GDALGetRasterYSize(tile) );  
	aabox2d_t box;
	box.to(get_corner( tile, 0.0, 0.0 ));
	box.merge(get_corner(tile, GDALGetRasterXSize(tile), GDALGetRasterYSize(tile)));
	fnames_.push_back(fname);
	boxes_.push_back(box);
	GDALClose(tile);
      }
    }

    void tileinfo::process_directory(const std::string& dirname,
				     const std::string& pattern) {
      DIR* dir = opendir(dirname.c_str());
      if (!dir) {
	std::cerr << "TILEINFO: Unable to open directory: " << dirname << std::endl;
      } else {
	std::cout << "TILEINFO: Scanning directory: " << dirname << " for files matching " << pattern << std::endl;
	struct dirent* d;
	while ((d=readdir(dir))!=0) {
	  std::string fname = std::string(d->d_name);
	  if (sl::matches(fname, pattern)) {
	    // FIXME CREATE reader...
	    std::string fullname = dirname + "/" + fname;
	    process_tile(fullname);
	  }
	}
	closedir(dir);
      }
    }



  } // namespace geo
} // namespace vic
