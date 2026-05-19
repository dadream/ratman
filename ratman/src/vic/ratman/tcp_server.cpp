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
#include <vic/ratman/tcp_server.hpp>
#include <vic/ratman/s3d_parser.hpp>
#include <sl/assert.hpp>

#include <stdio.h>
#include <iostream>

#include <QTextStream>


namespace ratman {

  // Utilities
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

  // tcp_server
 
  tcp_server::tcp_server( QObject* parent) : 
    QObject(parent),
    lat_(0),
    lon_(0),
    fname_(""),
    request_type_(SERVER_EMPTY),
    error_msg_(""),
    error_(false) {
    server_connection_=0;
    server_ = new QTcpServer();
    connect(server_, SIGNAL(newConnection()), this, SLOT(accept_connection()));
    
    // start client/server
    if (server_->listen(QHostAddress::Any, ratman::TCP_PORT)) {      
      SL_TRACE_OUT(1) << "tcp_server: server listening... Port: " << server_->serverPort() << std::endl;
    } else {
      error_msg_="tcp_server:: server cannot start";
      error_=true;
    }
    if (server_->isListening()) {
      SL_TRACE_OUT(1) << "tcp_server: server still listening..." << std::endl;
    }
  }
  
  
  tcp_server::~tcp_server() {
    SL_TRACE_OUT(1) << "tcp_server: server closed!" << std::endl;
  }


  void tcp_server::accept_connection() {
    SL_TRACE_OUT(1) << "ACCEPTED NEW CONNECTION" << std::endl;
    server_connection_ = server_->nextPendingConnection();
    connect(server_connection_, SIGNAL(readyRead()),
	    this, SLOT(read_from_client()));
    connect(server_connection_, SIGNAL(error(QAbstractSocket::SocketError)),
	    this, SLOT(display_error(QAbstractSocket::SocketError)));
  }
  
  void tcp_server::write_to_client(const QString& msg) {
    if (server_connection_) {     
      QTextStream os(server_connection_);
      os.setCodec("UTF-8");//UnicodeUTF8 
      os << msg;
      os.flush();
      if (server_connection_->state() == QAbstractSocket::ConnectedState) {
	server_connection_->disconnectFromHost();
	server_connection_->waitForDisconnected(1000);
      }
      if (server_connection_) delete server_connection_;
      server_connection_=0;   
    }
  }

 
  void tcp_server::read_from_client() {
    if ( server_connection_ ) {
      QString hostname=server_connection_->peerAddress ().toString();
      SL_TRACE_OUT(1) << "CONNECTION ACCEPTED From " << hostname.toStdString() << " Port: " << server_connection_->peerPort() << std::endl;
      char buf[1024];
      server_connection_->readLine(buf, sizeof(buf));
      QString str = QString(buf);
      QStringList cmd = str.split(";");
      for (int i=0; i<cmd.size(); ++i) {
	SL_TRACE_OUT(1) << "'" << cmd[i].toStdString() << "'" << std::endl;
	if (cmd[i] == "id") {
	  QString ids;
	  ids.sprintf("id=%d;", ratman::TCP_ID);
	  write_to_client(ids);
	  request_type_=SERVER_EMPTY;
	} else {
	  QString param = get_parameter(cmd[i]);
	  QString value = get_value(cmd[i]);
	  if (param == "file") {
	    fname_=value;
	    parsing_s3d();
	    if (error_) {
	      request_type_=SERVER_ERR;
	      write_to_client("err;");
	    } else {
	      request_type_=SERVER_FILE;
	      write_to_client("ok;");
	    }
	  } else if (param == "lonlat") {
	    QStringList lonlat = value.split(",");
	    if (lonlat.size() == 2) {
	      lon_ = lonlat[0].toDouble();
	      lat_ = lonlat[1].toDouble();
	      request_type_=SERVER_LONLAT;
	      write_to_client("ok;");
	    } else {
	      error_msg_="lonlat parameters missing";
	      error_=true;
	      request_type_=SERVER_ERR;
	      write_to_client("err;");
	    }
	  } 
	} 
      }
    } else {
      SL_TRACE_OUT(1) << "No Pending request" << std::endl;
    }
    if (request_type_ != SERVER_ERR) {
      emit ready_param();
    }
  }
  
  void tcp_server::display_error(QAbstractSocket::SocketError socket_error) {
    if (socket_error == QTcpSocket::RemoteHostClosedError)
      return;
    error_=false;
    error_msg_=server_connection_->errorString();
    server_->close();
  }
 
  void tcp_server::parsing_s3d() {
    s3d_parser parser;
    parser.read_and_parse(fname_);
    lon_=parser.lon();
    lat_=parser.lat();
    error_msg_=parser.error_msg();
    error_=parser.error();
  }
 
}

