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
#ifndef RATMAN_TCP_CLIENT_HPP
#define RATMAN_TCP_CLIENT_HPP

#include <vic/cbdam/base/background_thread.hpp>
#include <QtNetwork>
#include <vic/ratman/tcp_server.hpp>

namespace ratman {

  /**
   * tcp client
   */
  class tcp_client : public cbdam::background_thread {

    Q_OBJECT
  
  public:

    tcp_client(const QString& msg, const QString &hostname="localhost", quint16 port=TCP_PORT, QObject* parent = 0 );
    virtual ~tcp_client();

    virtual void run();
    
    inline bool server_found() {
      return server_found_;
    }

    inline bool error() {
      return error_;
    }

    inline const QString& error_msg() const {
      return error_msg_;
    }

  protected:
    QString hostname_;
    quint16 port_;
    QString msg_;
    bool server_found_;
    QString error_msg_;
    bool error_;
 
  };
}

#endif // TCP_CLIENT_HPP
