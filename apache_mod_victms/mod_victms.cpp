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
// Apache stuff
//  Its important to use "" instead of <> and to have the -I flags in
//  the right order in the Makefile because there is an Apache alloc.h
//  that is completely different from the system alloc.h.
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_core.h"
#include "http_log.h"
#include "http_protocol.h"
#include "http_request.h"
#include "http_main.h"
#include "util_script.h"
#include "util_md5.h"
#include "apr_date.h" 
#include "util_time.h"

// Apache's compatibility warnings are of no concern to us.
#undef strtoul

// System include files
#include <string>
#include <vector>
#include <stdarg.h>
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iomanip>


// Local include

#include "document.hpp"
#include "tilemap_config.hpp"
#include "victms_conventions.hpp"

// ===============================================================
// Helpers
// ===============================================================

template<class OutIt>
void split(const std::string& s, const std::string& sep, OutIt dest) {
  std::string::size_type left = s.find_first_not_of( sep );
  std::string::size_type right = s.find_first_of( sep, left );
  while( left < right ) {
    *dest = s.substr( left, right-left );
    ++dest;
    left = s.find_first_not_of( sep, right );
    right = s.find_first_of( sep, left );
  }
} 

// ===============================================================
// Forward declaration of global module definition structure
// ===============================================================

extern "C" module AP_MODULE_DECLARE_DATA victms_module;

// ===============================================================
// Implementation
// ===============================================================

namespace mod_victms {



  static std::vector<vic::geo::base::tilemap_config>* tilemaps=0;


  static void config_load(const std::string& root_path) {
    if (tilemaps) delete tilemaps;
    tilemaps=new std::vector<vic::geo::base::tilemap_config>;
    std::string config_fname=root_path+"/"+"config.xml";
    std::ifstream in(config_fname.c_str(),std::ios::in);
    if (in.is_open()) {
      vic::xml::document doc;
      doc.parse(in);
      in.close();
      if(!doc.error()) {
	vic::xml::node_iterator ptr=doc.first_root("victms");
	if (!ptr.is_null()) {
	  for(vic::xml::node_iterator child_it=ptr.down();
	      !child_it.is_null();
	      child_it = child_it.next()) {
	    vic::geo::base::tilemap_config tc;
	    if(tc.parse(child_it)) {
	      tilemaps->push_back(tc);
	    } else {
	      fprintf(stderr, "mod_victms: Error parsing config file\n"); 
	    }
	  }
	} else {
	  fprintf(stderr, "mod_victms: Error parsing config file\n"); 
	}
      } else {
	fprintf(stderr, "mod_victms: %s\n", doc.error_msg().c_str()); 
      }    
    } else {
      fprintf(stderr, "mod_victms: cannot open config file!\n"); 
    }
    fflush(stderr);
  }

  static vic::geo::base::tilemap_config* config_find(const std::string& name) {
    vic::geo::base::tilemap_config* result = 0;
    if (tilemaps) {
      for (std::size_t i=0; i<tilemaps->size() && !result; ++i) {
	if ((*tilemaps)[i].name() == name) {
	  result = &((*tilemaps)[i]);
	}
      }
    }
    return result;
  }



  static std::string request_get_file_name( request_rec *r ) {
    std::string result=std::string(r->filename);
    if (r->path_info) {
      result += std::string(r->path_info);
    }
    return result;
  }

   static std::string request_get_root_dir( request_rec *r ) {
     std::string fname=request_get_file_name(r);
     std::size_t pos = fname.rfind("/victms");
     if ( pos == std::string::npos) {
       return fname;
     } else {
       return fname.substr(0,pos)+"/victms";
     }
   }

  static void log_error( request_rec *r, const char *msg, ... ) {
    va_list ap;
    
    // Log error
    fprintf( stderr, "mod_victms: " );
    va_start( ap, msg );
    vfprintf( stderr, msg, ap );
    va_end( ap );
    fprintf( stderr, "\n" );
    r->content_type = "text/html";
    ap_rprintf(r, "<HTML><BODY>Error: %s</BODY></HTML>", msg);
  }

  static std::string root_url(request_rec* r) {
    const char* server = ap_get_server_name(r);
    return
      "http://" + std::string(server ? server : "NULL") + 
      "/victms/";
  }

  static std::string service_url(request_rec* r) {
    std::string version = "1.0.0";
    return
      root_url(r)+
      version + "/";
  }

  static int root_resource(request_rec* r,
			   const std::string& pathbase) {
    r->content_type = "text/xml";
    ap_rprintf(r,
	       "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n"
	       "<Services>\n"
	       "  <TileMapService title=\"VICTMS server\" version=\"1.0.0\" href=\"%s\" />\n"
	       "</Services>\n",
	       service_url(r).c_str());

    return OK; 
  }

  static int service_resource(request_rec* r,
			      const std::string& pathbase,
			      const std::string& version) {
				     
    r->content_type = "text/xml";
    std::ostringstream out;
    out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
    out << "<TileMapService version=\"" << version << "\" services=\"" << root_url(r) <<"\">" << std::endl;
    out << "  <Title>VICTMS server: Service Resource</Title>" << std::endl;
    out << "  <Abstract>VICTMS server: Service Resource</Abstract>" << std::endl;
    out << "  <TileMaps>" << std::endl;
    if (tilemaps) {
      for (std::size_t i=0; i<tilemaps->size(); ++i) {
	out << "    <TileMap" << std::endl;
	out << "      title=\"" << (*tilemaps)[i].name() << "\"" << std::endl;
	out << "      srs=\"" << (*tilemaps)[i].srs() << "\"" << std::endl;
	out << "      profile=\"" << (*tilemaps)[i].profile() << "\"" << std::endl;
	out << "      href=\"" << service_url(r) << (*tilemaps)[i].name() << "/\" />" << std::endl;
      }
    }
    out << "  </TileMaps>" << std::endl;
    out << "</TileMapService>" << std::endl;

    ap_rputs(out.str().c_str(), r);
    
    return OK; 
  }

  static int tilemap_resource(request_rec* r,
			      const std::string& pathbase,
			      const std::string& version,
			      const std::string& tilemap) {
    vic::geo::base::tilemap_config* tc=config_find(tilemap);
    if (tc) {
      std::string tileset_url=service_url(r)+tilemap+"/";
      r->content_type = "text/xml";
      std::ostringstream out;
      out << "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>" << std::endl;
      out << "<TileMap version=\"" << version << "\" tilemapservice=\"" << service_url(r) <<"\">" << std::endl;
      out << "  <Title>" << tilemap << "</Title>" << std::endl;
      out << "  <Abstract>VICTMS server: TileMap Resource for " << tilemap << "</Abstract>" << std::endl;
      out << "  <SRS>" << tc->srs() <<"</SRS>" << std::endl;
      out << "  <BoundingBox minx=\"" << tc->bbox_lo(0) << "\" "
	  << "miny=\"" << tc->bbox_lo(1) << "\" "
	  << "maxx=\"" << tc->bbox_hi(0) << "\" "
	  << "maxy=\"" << tc->bbox_hi(1) << "\" />" << std::endl;
      out << "  <Origin x=\"" << tc->bbox_lo(0) << "\" y=\"" << tc->bbox_lo(1) << "\" />" << std::endl;
      out << "  <TileFormat width=\"" << tc->img_width() << "\" "
	  << "height=\"" << tc->img_height() << "\" "
	  << "mime-type=\"" << tc->mime() << "\" "
	  << "extension=\"" << tc->extension() << "\" />" << std::endl;
      out << "  <TileSets profile=\"" << tc->profile() << "\">" << std::endl;
      for (std::size_t i=0; i<=std::size_t(tc->max_level()); ++i) {
	out << "    <TileSet href=\"" << tileset_url << i << "/\" "
	    << "units-per-pixel=\"" << std::setprecision(10) << tc->units_per_pixel(i) << "\" order=\"" << i << "\" />" << std::endl;
      }
      out << "  </TileSets>" << std::endl;
      out << "</TileMap>" << std::endl;
      ap_rputs(out.str().c_str(), r);
      return OK; 
    } else {
      mod_victms::log_error(r, "Tilemap \"%s\" not found", tilemap.c_str());
      return HTTP_NOT_FOUND;
    }
  }


  static int tile(request_rec* r,
		  const std::string& pathbase,
		  const std::string& version,
		  const std::string& tilemap,
		  const int& level,
		  const int& x,
		  const int& y,
		  const std::string& ext) {

    const long long SEC_PER_HOUR = 3600;
    const long long USEC_PER_HOUR = 1000000 * SEC_PER_HOUR;
    const long long USEC_PER_WEEK = 7 * 24 * USEC_PER_HOUR;

    vic::geo::base::tilemap_config* tc=config_find(tilemap);
    if (!tc) {
      mod_victms::log_error(r, "Tilemap \"%s\" not found", tilemap.c_str());
      return HTTP_BAD_REQUEST; // Module has handled this stage
    } else {
      std::string filename = vic::geo::base::victms_conventions::quad_filename(pathbase+"/"+version+"/"+tilemap,
									       level, x, y, ext);
      // Readable? 
      apr_file_t *fp;
      apr_status_t open_status = apr_file_open(&fp, 
					       filename.c_str(), 
					       APR_READ, 
					       APR_OS_DEFAULT, 
					       r->pool);
      if (open_status != APR_SUCCESS) {
	mod_victms::log_error(r, "Could not open file \"%s\"", filename.c_str());
	return HTTP_NOT_FOUND;
      }

      // Get file type and size
      apr_size_t fsize = 0;
      apr_finfo_t finfo;
      apr_status_t info_status = apr_file_info_get(&finfo, APR_FINFO_SIZE, fp);
      if (info_status == APR_SUCCESS) {
	fsize = finfo.size;
      } else {
	mod_victms::log_error(r, 
			      "cannot get file size for file \"%s\"",
			      filename.c_str());
	apr_file_close(fp);
	return HTTP_NOT_FOUND;
      }
    
      /* Send HTTP header */
      r->content_type = tc->mime().c_str();
      ap_set_content_length(r, fsize);

      // Tile Map Server Cacheability

      char *date = (char *)(apr_palloc(r->pool, APR_RFC822_DATE_LEN));
      // set the Expires header to one week in the future
      apr_time_t expiring_date = r->request_time + USEC_PER_WEEK;
      ap_recent_rfc822_date(date, expiring_date);
      apr_table_addn(r->headers_out, "Expires", date);

      apr_table_addn(r->headers_out, "Cache-Control", "public, max-age=604800");
      
      ap_update_mtime(r,finfo.mtime);
      ap_set_last_modified(r);

      r->status = HTTP_OK;
      r->status_line = "200 OK";
      /*ap_send_http_header(r);*/
      
      /* Write the content */
      apr_size_t sent=0;
      apr_status_t send_status = ap_send_fd(fp, r, 0, fsize, &sent);
      if (send_status != APR_SUCCESS || sent != fsize) {
	mod_victms::log_error(r, "short write (%d/%d) for file \"%s\"",
			      sent, fsize, filename.c_str());
      }
      apr_file_close(fp);
      
      return OK; 
    }  
  }

} // End namespace

// ===============================================================
// Entry points
// ===============================================================

static apr_status_t victms_cleanup(void *cfgdata) {
  // cleanup code here, if needed
  if (mod_victms::tilemaps) delete mod_victms::tilemaps;
  mod_victms::tilemaps=0;
  return APR_SUCCESS;
}

static void victms_child_init(apr_pool_t *p, server_rec *s) {
  // Register cleanup function
  apr_pool_cleanup_register(p, NULL, victms_cleanup, victms_cleanup);
}

static int victms_handler(request_rec *r) {
  if (strcmp(r->handler, "victms")) {
    return DECLINED; // Module declines to handle
  } else {
    // .../victms/1.0.0/global_mosaic/0/0/0.jpg
    if (!mod_victms::tilemaps) {
      mod_victms::config_load(mod_victms::request_get_root_dir(r));
    }

    std::string fn = mod_victms::request_get_file_name(r);
    std::vector<std::string> fn_tokens;
    split(fn, "/", std::back_inserter(fn_tokens));
    std::string              pathbase = "/";
    std::vector<std::string> parameters;
    std::size_t token_size  = fn_tokens.size();
    std::size_t endpath_pos = token_size;
    for (std::size_t i=0;
	 (i<token_size) && (endpath_pos >= token_size);
	 ++i) {
      pathbase+= fn_tokens[i] + "/";
      endpath_pos = (fn_tokens[i] == "victms") ? i : endpath_pos;
    }
    for (std::size_t i=endpath_pos+1;
	 i<token_size; 
	 ++i) {
      parameters.push_back(fn_tokens[i]);
    }
    if (parameters.size() == 0) {
      return mod_victms::root_resource(r, pathbase);
    } else if (parameters.size() == 1) {
      return mod_victms::service_resource(r, pathbase, parameters[0]);
    } else if (parameters.size() == 2) {
      return mod_victms::tilemap_resource(r, pathbase, parameters[0], parameters[1]);
    } else if (parameters.size() == 5) {
      int level, x, y;
      char ext[512];
      if ((sscanf(parameters[2].c_str(), "%d", &level) == 1) &&
	  (sscanf(parameters[3].c_str(), "%d", &x) == 1) &&
	  (sscanf(parameters[4].c_str(), "%d.%s", &y, ext) == 2)) {
	return mod_victms::tile(r, pathbase, 
				parameters[0], 
				parameters[1], 
				level,
				x, 
				y,
				std::string(ext));
      } else {
	// ERROR
	mod_victms::log_error(r, "Error when parsing TMS request");
	return HTTP_BAD_REQUEST; // Module has handled this stage
      }
    } else {
      // ERROR
      mod_victms::log_error(r, "Error when parsing TMS request");
      return HTTP_BAD_REQUEST; // Module has handled this stage
    }
  }
}

// ===============================================================
// Hook registration
// ===============================================================

static void victms_register_hooks(apr_pool_t *p) {
  ap_hook_handler(victms_handler, NULL, NULL, APR_HOOK_MIDDLE);
  ap_hook_child_init(victms_child_init, NULL, NULL, APR_HOOK_MIDDLE);
}

// ===============================================================
// Global module definition structure
// ===============================================================

// We have to use C style linkage for the API functions that will be
// linked to apache.
extern "C" {
  // Dispatch list for API hooks 
  module AP_MODULE_DECLARE_DATA victms_module = {
    STANDARD20_MODULE_STUFF, 
    NULL,                  /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    NULL,                  /* table of config file commands       */
    victms_register_hooks   /* register hooks                      */
  };
};
