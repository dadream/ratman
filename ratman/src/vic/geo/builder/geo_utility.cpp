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
#define _POSIX_C_SOURCE 199309 
#include <time.h>

#include <vic/geo/builder/geo_utility.hpp>

#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

// GDAL include
#include <ogr_spatialref.h>

namespace vic {
  namespace geo {

    // geo_utility

    std::string geo_utility::proj2srs(const std::string& proj) {
      char* wkt;
      OGRSpatialReference sr;
      std::string result;
      if (sr.SetFromUserInput(proj.c_str()) == OGRERR_NONE) {
	sr.exportToWkt(&wkt);
	result = std::string(wkt);
	OGRFree(wkt);
      }      
      return result;
    }

    sl::fixed_size_array<6,double> geo_utility::gdal_array(const sl::matrix3d& m) {
      sl::fixed_size_array<6,double> result;
      result[0]=m(0,2);
      result[1]=m(0,0);
      result[2]=m(0,1);
      result[3]=m(1,2);
      result[4]=m(1,0);
      result[5]=m(1,1);
      return result;
    }

    std::string geo_utility::clean_path(const std::string& path) {
      std::string result;
      if (!path.empty()) {
	int len=path.length();
	bool found_separator=false;
	std::string tmp;
	for (int i=0; i<len; ++i) {
	  if (path[i] == '/') {
	    if (!found_separator) {
	      found_separator=true;
	      tmp += path[i];
	    }
	  } else {
	    result += tmp + path[i];
	    tmp="";
	    found_separator=false;
	  }	     
	}
      }
      return result;
    }

    static bool robust_mkdir(const std::string& ppath) {
      mode_t mode = 0777 & ~umask(0);
      mode_t dir_mode = mode | S_IWUSR | S_IXUSR;      

      bool result = false;
      std::size_t attempts = 0;
      while (!result && (attempts<5)) {
	result = true;
	errno = 0;
	if (mkdir(ppath.c_str(), dir_mode)) {
	  // Directory not created - this is a 
	  // success only if path was existent
	  // as a directory
	  result = false;
	  if (errno == EEXIST) {
	    struct stat statbuf;
	    if (!stat(ppath.c_str(), &statbuf)) {
	      if (S_ISDIR(statbuf.st_mode)) {
		result = true;
	      } else {
		SL_TRACE_OUT(1) << "Exists, but not dir: " << ppath << std::endl;
	      }
	    } else {
	      SL_TRACE_OUT(1) << "Cannot stat: " << ppath << std::endl;
	    }
	  } else {
	    SL_TRACE_OUT(1) << "Cannot mkdir: " << ppath << ": " << strerror(errno) << std::endl;
	  }
	}
	++attempts;
	if (!result) {
	  // Not a success - might be because
	  // a parallel process is locking the
	  // same directory - sleep for 
	  // a while and retry
	  // FIXME: Should check errno and retry only if
	  //        really needed... 
	  struct timespec time;
	  time.tv_sec = 0;
	  time.tv_nsec = 100*1000*1000;
	  nanosleep(&time, 0);	    
	}
      }
      if (attempts>1 && result == true) {
	std::cerr << "WARNING: mkdir: " << ppath << " #RETRY=" << attempts << " " << (result ? "ok" : "FAIL") << std::endl;
      }
      return result;
    }

    static bool bottom_up_mkpath(const std::string& cpath) {
      // Remove trailing '/'
      std::size_t sz = cpath.size();
      if (cpath[sz-1]=='/') --sz; 

      // If root dir, assume existing... 
      if (sz == 0) {
	return true;
      } else {
	// Try making current path
	std::string current_path = cpath.substr(0, sz);
	if (robust_mkdir(current_path)) {
	  return true;
	} else {
	  // .. if not recursively make parent, then current
	  int p = current_path.rfind('/');
	  if (p > 0) {
	    if (bottom_up_mkpath(current_path.substr(0, p))) {
	      return robust_mkdir(current_path);
	    }
	  }
	}
      }
      return false;
    }

    static bool top_down_mkpath(const std::string& cpath) {
      int pt=0;
      while (pt>=0) {
	pt=cpath.find('/',pt+1);
	std::string ppath= cpath.substr(0,pt);
	
	bool success = robust_mkdir(ppath);
	if (!success) return false;
      }
      return true;
    }

    bool geo_utility::has_dir(const std::string& path) {
      bool result = false;

      struct stat statbuf;
      if (!stat(path.c_str(), &statbuf)) {
	if (S_ISDIR(statbuf.st_mode)) {
	  result = true;
	}
      }

      return result;
    }

    bool geo_utility::mkpath(const std::string& path) {
      return has_dir(path) || top_down_mkpath(clean_path(path));
    }

    // geo_matrix 

    geo_matrix::geo_matrix() {
      mat_.to_identity();
      mat_inv_.to_identity();
    }

    geo_matrix::geo_matrix(double x0, double y0, double rx, double ry) {   
      mat_=sl::matrix3d(rx,  0.0, x0,
			0.0, ry,  y0,
			0.0, 0.0, 1.0);
      mat_inv_=mat_.inverse();
    }

    geo_matrix::geo_matrix(double *mat) {
      mat_=sl::matrix3d(mat[1],  mat[2], mat[0],
			mat[4], mat[5],  mat[3],
			0.0, 0.0, 1.0);
      mat_inv_=mat_.inverse();
    }

    geo_matrix::~geo_matrix() {
    }
       
 
  } // namespace geo
} // namespace vic
