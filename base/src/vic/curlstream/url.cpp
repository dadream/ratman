#include "url.hpp"
#include <iostream>
#include <ctype.h> // tolower

namespace vic {

  url::url(const std::string& str) {
    url_ = str;
    std::string tmpurl = str;
    // Extract protocol
    protocol_.assign("");
    std::string::size_type colon_double_slash = tmpurl.find("://");
    if (colon_double_slash != std::string::npos) {
      protocol_ = tmpurl.substr(0, colon_double_slash);
      tmpurl.erase(0, colon_double_slash+3);
    }

    if (protocol_.empty()) {
      // Assume everything is a path
      path_ = tmpurl;
      userpass_ = "";
      port_ = "";
      host_ = "";
    } else {
      // Path
      path_.assign("/");
      std::string::size_type slash = tmpurl.find('/');
      if (slash != std::string::npos) {
	path_ = tmpurl.substr(slash);
	tmpurl.erase(slash);
      }
      // Userpass
      userpass_.assign("");
      std::string::size_type at = tmpurl.rfind('@');
      if (at != std::string::npos) {
	userpass_ = tmpurl.substr(0, at);
	tmpurl.erase(0, at+1);
      }
      // Port
      port_.assign("");
      std::string::size_type colon = tmpurl.rfind(':');
      if (colon != std::string::npos) {
	port_ = tmpurl.substr(colon+1);
      tmpurl.erase(colon);
      }
      // Host is what remains
      host_ = tmpurl;
    }

#if 0
    std::cerr << "URL PARSING:";
    std::cerr << "url=" << url_string() << std::endl;
    std::cerr << "  protocol=" << protocol() << std::endl;
    std::cerr << "  host=" << host() << std::endl;
    std::cerr << "  port=" << port() << std::endl;
    std::cerr << "  path=" << path() << std::endl;
    std::cerr << "  auth=" << auth() << std::endl;
#endif
  }

  url url::base() const {
    
    std::string basepath = path_;
    std::string::size_type slash = basepath.rfind('/');
    if (slash == std::string::npos) {
      basepath += '/';
    } else {
      basepath = basepath.substr(0,slash);
      basepath += '/';
    }
    // protocol://username:password@server:port/path
    std::string result;
    if (!protocol().empty()) {
      result += protocol() + "://";
      if (!auth().empty()) result += auth() + "@";
      if (!host().empty()) result += host();
      if (!port().empty()) result += ":" + port();
      result += "/";
    }
    result += basepath;

    return url(result);
  }

  url url::operator +(const url& other) const {
    // 
    if ((other.protocol().empty()) && 
	((other.path().length()==0) ||
	 (other.path()[0] != '/'))) {
      // other is relative, sum to base
      return url(url_string() + other.path());
    } else {
      return other;
    }
  }

}
