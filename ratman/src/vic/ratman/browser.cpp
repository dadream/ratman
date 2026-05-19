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
#include <vic/ratman/browser.hpp>
#include <iostream>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN 1
#include "windows.h"
#include "shellapi.h"
#endif /* WIN32 */

namespace ratman {

#ifdef WIN32
  int browser::open_url(const char* url) {
    wchar_t url_w[4096];
    std::size_t N=strlen(url);
    if (N>4095)N=4095;
    for (std::size_t i=0; i<N; ++i) url_w[i] = url[i];
    url_w[N] = '\0';
    int ret = (int)ShellExecute( NULL, TEXT("open"), url_w, NULL, TEXT(".\\"), SW_SHOWMAXIMIZED );
    if (ret<=32) {
      return -1;
    } else {
      return 0;
    }
  }
#else 
  int browser::open_url(const char* url) {

    /*
     * Launch browser
     *
     * Respect $BROWSER
     * (http://www.catb.org/~esr/BROWSER/index.html)
     */

    char* pch = getenv("BROWSER");
    std::string browsers;
    if (pch) {
      /* found $BROWSER */
      browsers = std::string(pch);
    } else {
      /* use default */
#ifdef __APPLE__
      browsers = std::string("open %s");
#else
      /* other unix */
      browsers = std::string("firefox -remote 'openUrl(%s)':firefox \"%s\" &:mozilla -remote 'openUrl(%s)':mozilla \"%s\" &:opera -remote 'openUrl(%s)':opera \"%s\" &:netscape -remote 'openUrl(%s)':netscape \"%s\" &" );
#endif
    }
    /* loop through commands */

    std::size_t start_pos=0;
    while (start_pos<browsers.size()) {
      std::size_t end_pos=browsers.find(':',start_pos);
      std::string head = (end_pos == browsers.npos) ? browsers.substr(start_pos,browsers.size()-start_pos) : browsers.substr(start_pos, end_pos-start_pos);
      start_pos = end_pos+1;

      
      if ( head.find("%s") == head.npos) head += " \"%s\"";
      char cmd[4096];
      snprintf(cmd, 4096, head.c_str(), url);
      std::cerr << "EXECUTING: " << cmd << std::endl;
      int rc = system(cmd);
      std::cerr << " => " << rc << std::endl;
      if (rc == 0) return 0;
    }

    return -1;
  }
#endif
}
