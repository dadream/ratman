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
#include <vic/ratman/tcp_client.hpp>

#include <iostream>
#include <QTextStream>

namespace ratman {

  static QString get_parameter(const QString& token) {
    QString result="";
    int pos1 = token.indexOf("=",0);
    if(pos1>=0) result=token.mid(0,pos1);
    return result;
  }

  static QString get_value(const QString& token) {
    QString result="";
    int pos0 = token.indexOf("=");
    if(pos0>=0) {
      int l = token.length()-pos0-1;
      result=token.right(l);
    }
    return result;
  }


  tcp_client::tcp_client(const QString &msg, const QString &hostname, quint16 port, QObject *parent) : 
    cbdam::background_thread(parent),
    hostname_(hostname),
    port_(port),
    msg_(msg),
    server_found_(false),
    error_msg_(""),
    error_(false){
    start();
  }


  tcp_client::~tcp_client() {
  }

  
  void tcp_client::run() {
    // set background priority
    cbdam::background_thread::run();

    const int Timeout = 5 * 1000;
    QTcpSocket connection;
    connection.connectToHost(hostname_, port_);
    if (!connection.waitForConnected(Timeout)) {
      error_msg_=connection.errorString();
      error_=true;
      return;
    }

    connection.write(msg_.toStdString().c_str());
    if (!connection.waitForBytesWritten(Timeout)) {
      error_msg_=connection.errorString();
      error_=true;
      return;
    }

    if (!connection.waitForReadyRead(Timeout)) {
      error_msg_=connection.errorString();
      error_=true;
      return;
    }

    char buf[1024];
    connection.readLine(buf, sizeof(buf));

    QString output(buf);
      
    QStringList cmd = output.split(";");
    for (int i=0; i<cmd.size(); ++i) {
      QString param = get_parameter(cmd[i]);
      if (param == "id") {
	int value = get_value(cmd[i]).toInt();
	server_found_ = (value==TCP_ID);
      }
    }
  }
}
