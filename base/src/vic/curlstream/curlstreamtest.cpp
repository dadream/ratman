#include "curlstream.hpp"
#include "url.hpp"
#include <iostream>
#include <curl/curl.h>

void test_url(const std::string& str_base, const std::string& rel) {
  vic::url url_base = vic::url(str_base).base();
  vic::url url_rel(rel);
  vic::url url_combined = url_base + url_rel;

  std::cerr << "===============================================" << std::endl;
  std::cerr << str_base << " + " << rel << " => " << url_combined.url_string() << std::endl;
  std::cerr << "===============================================" << std::endl;
}

int main(int argc,
	 const char* argv[]) {
  if (argc!=2) {
    std::cerr << "Usage: " << argv[0] << " <url>" << std::endl;
  }
  std::string url =argv[1];

  test_url("http://www.crs4.it/vic/", "index.html");
  test_url("http://www.crs4.it/vic/", "/etc/index.html");
  test_url("http://www.crs4.it/vic/", "./index.html");
  test_url("http://www.crs4.it/vic", "index.html");
  test_url("http://www.crs4.it/vic", "/etc/index.html");
  test_url("http://www.crs4.it/vic", "./index.html");

  exit(1);

  /*FIXME*/curl_global_init(CURL_GLOBAL_ALL);
  
  vic::icurlstream ifile;
  ifile.open(url.c_str());
  if (!ifile) {
    std::cerr << "Failed to open " << url << std::endl;
  } else {
    while (ifile.good()) {
      std::string line;
      std::getline(ifile,line);
      //ifile >> line;
      std::cout << line << std::endl;
    }
    if (ifile.fail() &&!ifile.eof()) {
      std::cout << "Read error!" << std::endl;
    }
    ifile.close();
    std::cout << "Done." << std::endl;
  }

  /*FIXME*/curl_global_cleanup();
}
