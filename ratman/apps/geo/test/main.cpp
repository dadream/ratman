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
#include <string>
#include <vic/geo/srs/spatial_reference.hpp>


int main(int argc, char *argv[]) {
  vic::geo::srs::spatial_reference EPSG3003("EPSG:3003");

  std::cerr << EPSG3003.description() << std::endl;
  
  sl::point3d uvh(1236626.0, 4175366.0, 0.0);
  sl::point3d lonlat = EPSG3003.to_WGS84_lonlat(uvh);

  std::cerr << "UVH   : " << uvh[0] << " " << uvh[1] << " " << uvh[2] << std::endl;
  std::cerr << "LONLAT: " << lonlat[0] << " " << lonlat[1] << " " << lonlat[2] << std::endl;
    
  uvh = EPSG3003.from_WGS84_lonlat(lonlat);
  std::cerr << "UVH'  : " << uvh[0] << " " << uvh[1] << " " << uvh[2] << std::endl;

}
