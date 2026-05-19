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

// Apache's compatibility warnings are of no concern to us.
#undef strtoul

// System include files
#include <string>
#include <vector>
#include <stdarg.h>

// Vic include files
#include <sl/fixed_size_array.hpp>
#include <vic/vfs/repository.hpp>

// ===============================================================
// Forward declaration of global module definition structure
// ===============================================================

extern "C" module AP_MODULE_DECLARE_DATA vicrepo_module;

// ===============================================================
// Implementation
// ===============================================================

namespace mod_vicrepo {

  static const std::size_t MAX_CONNECTION_COUNT = 8; // FIXME? MB

  typedef sl::fixed_size_array<3,int32_t> grid_point_t;
  typedef vic::vfs::repository repository_t;
 
  static std::vector<repository_t*> connection_cache;

  /// Remove all open repositories from cache
  static void connection_cache_cleanup() {
    while (!connection_cache.empty()) {
      repository_t* c = connection_cache.back();
      connection_cache.pop_back();
      c->close();
      delete c;
    }
  }

  static std::size_t connection_cache_access(const std::string& file_name) {
    std::size_t result = std::size_t(-1); // not found
    for (std::size_t i=0; 
	 (result == std::size_t(-1)) && (i<connection_cache.size()); ++i) {
      if (connection_cache[i]->file_name() == file_name) {
	result = i;
      }
    }
    if (result == std::size_t(-1)) {
      // open
      repository_t* new_repo = new repository_t();
      new_repo->open_read(file_name);
      if (!new_repo->is_open()) {
	// File not found
	delete new_repo; new_repo = 0;
      } else {
	connection_cache.push_back(new_repo);
	result = connection_cache.size()-1;
      }
    }
    // Reorder
    if ((result != std::size_t(-1)) && (result != 0)) {
      repository_t* accessed = connection_cache[result];
      for (std::size_t i=0; i<result; ++i) {
	std::size_t j=result-1-i;
	connection_cache[j+1] = connection_cache[j];
      }
      connection_cache[0] = accessed;
      result = 0;
    }

    while (connection_cache.size() > MAX_CONNECTION_COUNT) {
      repository_t* c = connection_cache.back();
      connection_cache.pop_back();
      c->close();
      delete c;
    }

    return result;
  }


  static int request_get_arg( request_rec *r, char const *opt ) {
    char buffer[128] = "", *inptr, *outptr = buffer;
  
    if ( ( inptr = strstr( r->args, opt ) ) == NULL )
      return 0;
    
    inptr += strlen(opt);
    while ( *inptr != '&' && *inptr != '\0' )
      *outptr++ = *inptr++;
    *outptr++ = '\0';
    
    if ( strlen(buffer) == 0 )
      return 1;
    
    return atoi( buffer );
  }

  static std::string request_get_file_name( request_rec *r ) {
    char buffer[1024], *ptr;
    
    strcpy( buffer, r->filename );
    ptr = buffer + strlen(buffer) - 3;
    if ( strcmp( ptr, "/vicrepo" ) == 0 )
      *ptr = '\0';
  
    strcat( buffer, r->path_info );
    
    return std::string(buffer );
  }

  static void log_error( request_rec *r, const char *msg, ... ) {
    va_list ap;
    
    // Log error
    fprintf( stderr, "mod_vicrepo: " );
    va_start( ap, msg );
    vfprintf( stderr, msg, ap );
    va_end( ap );
    fprintf( stderr, "\n" );
    
    r->content_type = "text/html";
    
    // ?? FIXME
    ap_rprintf(r, "<HTML><BODY>Error: %s</BODY></HTML>", msg);
  }

  static void send_debug_info(request_rec *r) {
    const char* hostname;
  
    r->content_type = "text/html";
    hostname = ap_get_remote_host(r->connection, 
				  r->per_dir_config, REMOTE_NAME, 0);

    ap_rputs("<HTML>\n", r);
    ap_rputs("<HEADER>\n", r);
    ap_rputs("<TITLE>CRS4 VicRepo Apache mod debug output</TITLE>\n", r);
    ap_rputs("</HEADER>\n", r);
    ap_rputs("<BODY>\n", r);
    ap_rprintf(r, "<H1>CRS4 VicRepo Apache mod debug output</H1>\n");
    ap_rprintf(r, "<p><PRE>\n");
    ap_rprintf(r, "Server pid   = %d\n", getpid());
    ap_rprintf(r, "\n");
    ap_rprintf(r, "unparsed_uri = %s\n", r->unparsed_uri);
    ap_rprintf(r, "uri          = %s\n", r->uri);
    ap_rprintf(r, "filename     = %s\n", r->filename);
    ap_rprintf(r, "path_info    = %s\n", r->path_info);
    ap_rprintf(r, "args         = %s\n\n", r->args);
    ap_rprintf(r, "hostname     = %s\n\n", hostname);
    ap_rprintf(r, "\n");
    ap_rprintf(r, "dataset      = %s\n", request_get_file_name( r ).c_str() );
    ap_rprintf(r, "i            = %d\n", request_get_arg( r, "i=" ) );
    ap_rprintf(r, "j            = %d\n", request_get_arg( r, "j=" ) );
    ap_rprintf(r, "k            = %d\n", request_get_arg( r, "k=" ) );
    ap_rprintf(r, "debug        = %d\n", request_get_arg( r, "debug=" ) );
    ap_rprintf(r, "\n");
    ap_rprintf(r, "Open connections:\n");
    for (std::size_t i=0; i<connection_cache.size(); ++i) {
      ap_rprintf(r, "  [%d] %s\n", (int)i, connection_cache[i]->file_name().c_str());
    }    
    ap_rputs("</PRE></BODY>\n", r);
    ap_rputs("</HTML>\n", r);
  }

}

// ===============================================================
// Entry points
// ===============================================================

static apr_status_t vicrepo_cleanup(void *cfgdata) {
  // cleanup code here, if needed
  mod_vicrepo::connection_cache_cleanup();
  return APR_SUCCESS;
}

static void vicrepo_child_init(apr_pool_t *p, server_rec *s) {
  // Register cleanup function
  apr_pool_cleanup_register(p, NULL, vicrepo_cleanup, vicrepo_cleanup);
  mod_vicrepo::connection_cache_cleanup();
}

static int vicrepo_handler(request_rec *r) {
  if (strcmp(r->handler, "vicrepo")) {
    return DECLINED; // Module declines to handle
  } else if (mod_vicrepo::request_get_arg(r, "check=")) {
    r->content_type = "text/html";
    ap_rputs("<HTML><BODY>mod_vicrepo installed</BODY></HTML>\n", r);
    return OK; 
  } else if (mod_vicrepo::request_get_arg(r, "debug=")) {
    r->content_type = "text/html";
    mod_vicrepo::send_debug_info(r);
    return OK; 
  } else {
    std::string file_name = sl::pathname_without_extension(mod_vicrepo::request_get_file_name(r));
    int i = mod_vicrepo::request_get_arg(r, "i=");
    int j = mod_vicrepo::request_get_arg(r, "j=");
    int k = mod_vicrepo::request_get_arg(r, "k=");
    
    std::size_t connection_idx = mod_vicrepo::connection_cache_access(file_name);
    if (connection_idx == std::size_t(-1)) {
      mod_vicrepo::log_error(r, "Could not connect to dataset");
      return HTTP_NOT_FOUND; // Module has handled this stage
    } else {
      uint32_t       data_sz = 0;
      const uint8_t *data = mod_vicrepo::connection_cache[connection_idx]->get_data(mod_vicrepo::grid_point_t(i,j,k), data_sz);

      r->content_type = "text/html";
#if 1
      ap_rwrite(data, data_sz, r); 
#else
      ap_rprintf(r, "<HTML><BODY>mod_vicrepo returning tile file_name=%s idx=(%d,%d,%d) => sz=%d</BODY></HTML>\n", file_name.c_str(), i, j, k, data_sz);
#endif

      return OK; // Module has handled this stage
    }
  }
}

// ===============================================================
// Hook registration
// ===============================================================

static void vicrepo_register_hooks(apr_pool_t *p) {
  ap_hook_handler(vicrepo_handler, NULL, NULL, APR_HOOK_MIDDLE);
  ap_hook_child_init(vicrepo_child_init, NULL, NULL, APR_HOOK_MIDDLE);
}

// ===============================================================
// Global module definition structure
// ===============================================================

// We have to use C style linkage for the API functions that will be
// linked to apache.
extern "C" {
  // Dispatch list for API hooks 
  module AP_MODULE_DECLARE_DATA vicrepo_module = {
    STANDARD20_MODULE_STUFF, 
    NULL,                  /* create per-dir    config structures */
    NULL,                  /* merge  per-dir    config structures */
    NULL,                  /* create per-server config structures */
    NULL,                  /* merge  per-server config structures */
    NULL,                  /* table of config file commands       */
    vicrepo_register_hooks   /* register hooks                      */
  };
};
