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
#ifndef RATMAN_TCP_SERVER_HPP
#define RATMAN_TCP_SERVER_HPP


#include <QObject>
#include <QtNetwork>

namespace ratman {

  static const int TCP_ID=19696;
  static const int TCP_PORT=50001;

  /**
   * tcp server
   */
  class tcp_server : public QObject {

    Q_OBJECT

  public:

    enum request_t {
      SERVER_EMPTY,
      SERVER_LONLAT,
      SERVER_FILE,
      SERVER_ERR,
    };
  
  public:
    tcp_server( QObject* parent = 0 );
    virtual ~tcp_server();
 
    inline bool error() {
      return error_;
    }

    inline const QString& error_msg() const {
      return error_msg_;
    }

    inline double lon() const {
      return lon_;
    }

    inline double lat() const {
      return lat_;
    }
    
    inline const QString& filename() const {
      return fname_;
    }

    inline request_t request_type() const {
      return request_type_;
    }
    
  public slots:
    void accept_connection();
 
  signals:
  void ready_param();
     
  private slots:
    void read_from_client();
    void write_to_client(const QString& msg);
    void display_error(QAbstractSocket::SocketError socket_error);

  protected:
    void parsing_s3d();
        
  protected:
    QTcpServer* server_;
    QTcpSocket* server_connection_;
    double lat_;
    double lon_;
    QString fname_;
    request_t request_type_;
    QString error_msg_;
    bool error_;
  };
}

#endif // TCP_SERVER_HPP
