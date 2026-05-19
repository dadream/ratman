#ifndef VIC_URL_HPP
#define VIC_URL_HPP

#include <string>

namespace vic {
  /**
   * Minimal url parser:
   * Syntax: protocol://username:password@server:port/path
   */
  class url {
  protected:
    std::string url_;
    std::string protocol_;
    std::string host_;
    std::string port_;
    std::string path_;
    std::string userpass_;
  public:

    url(const std::string& str = "");

    const std::string& url_string() const { return url_; }

    const std::string& protocol() const { return protocol_; }
    const std::string& host() const { return host_; }
    const std::string& port() const { return port_; }
    const std::string& path() const { return path_; }
    const std::string& auth() const { return userpass_; }

    url base() const;

    url operator +(const url& other) const;  
  };
}

#endif

