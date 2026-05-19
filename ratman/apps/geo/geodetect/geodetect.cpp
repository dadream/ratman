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
#include <iostream>
#include <ogr_srs_api.h>
#include <gdal_priv.h>

#include <sl/axis_aligned_box.hpp>
#include <sl/fixed_size_point.hpp>

static sl::point2d get_corner(GDALDataset* tile, double x, double y) {
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

int main(int argc, char *argv[]) {
  if (argc != 4) {
    std::cerr << "Usage: " << std::endl;
    std::cerr << "   " << basename(argv[0])
	      << " <filename>" 
 	      << " <x> <y>" 
	      << std::endl;
    exit(1);
  }
  GDALAllRegister();
  char *fname=argv[1];
  GDALDataset* tile = reinterpret_cast<GDALDataset*>(GDALOpen(fname, GA_ReadOnly));
  if (tile == NULL) {
    std::cerr << "GDALOpen failed - "
	      << CPLGetLastErrorNo()
	      << " " << CPLGetLastErrorMsg()
	      << std::endl;
  } else {
    double x=atof(argv[2]);
    double y=atof(argv[3]);
    sl::point2d p; p = x, y;
    sl::aabox2d box;
    box.to(get_corner( tile, 0.0, 0.0 ));
    box.merge(get_corner(tile, GDALGetRasterXSize(tile), GDALGetRasterYSize(tile)));
    if (box.contains(p)) {
      std::cout << fname << std::endl;
    }
    GDALClose(tile);
  }

}
