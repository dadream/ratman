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
#include <GL/glew.h>
#include <QApplication>
#include <QProcess>
#include <QtOpenGL>
#include <QBuffer>
#include <QByteArray>
#include <QMessageBox>
#include <QTranslator>
#include <QLocale>
#include <QProgressDialog>

#ifdef _WIN32
#undef min
#undef max
#endif

#include <vic/curlstream/curlstream.hpp>
#include <vic/ratman/s3d_parser.hpp>
#include <vic/ratman/decorated_terrain_view.hpp>

#include <sstream>
#include <sl/assert.hpp>

#include "version.hpp"
#include "config.hpp"
#include "xml_config_parser.hpp"
#include "appwindow.hpp"

#ifdef QTSINGLEAPPLICATION
#include <QtSingleApplication>
#else
#include <vic/ratman/tcp_client.hpp>
#endif

int main(int argc, char *argv[]) {

#ifdef QTSINGLEAPPLICATION
  QtSingleApplication app("RATMAN", argc, argv);
#else
  QApplication app(argc, argv);
#endif

  app.connect( &app, SIGNAL(lastWindowClosed()), &app, SLOT(quit()) );

  std::string filename;
  std::string s3d_url;
  double lon=-1000.0;
  double lat=-1000.0;
  bool lonlat_req=false;
  for (int i=1; i<argc; ++i) {
    std::string arg = std::string(argv[i]);
    if (arg == "--help") {
      std::cout << "Usage: " << std::endl;
      std::cout << "        " << argv[0] << " <options> [<s3d_file>]" << std::endl;
      std::cout << "Options: " << std::endl;
      std::cout << "  --home_url <url>      configuration file's url" << std::endl;
      std::cout << "  --lonlat <lon> <lat>  goto lon,lat" << std::endl;
      std::cout << "  --help                usage help" << std::endl;
      std::cout << "  --version             print version" << std::endl;
      std::cout << "<s3d_file>              optional s3d file" << std::endl;
      
      exit(0);
    } else if (arg == "--home_url") {
       ++i; if (i==argc) {
	QString msg = QApplication::tr("Missing command line parameter after '--home-url'");
	QMessageBox::critical( 0, QApplication::tr("Load error" ), msg, QMessageBox::Abort,0 );
	qFatal(qPrintable(msg));
      }
      filename = std::string(argv[i]);
    } else if (arg == "--lonlat") {
      ++i; if (i==argc) {
	QString msg = QApplication::tr("Missing command line parameter '--lonlat'");
	QMessageBox::critical( 0, QApplication::tr("Load error" ), msg, QMessageBox::Abort,0 );
	qFatal(qPrintable(msg));
      }
      if (sscanf(argv[i], "%lf", &lon) != 1) {
	QString msg = QApplication::tr("Longitude is not a number");
	QMessageBox::critical( 0, QApplication::tr("Load error" ), msg, QMessageBox::Abort,0 );
	qFatal(qPrintable(msg));
      }
      ++i; if (i==argc) {
	QString msg = QApplication::tr("Missing command line parameter");
	QMessageBox::critical( 0, QApplication::tr("Load error" ), msg, QMessageBox::Abort,0 );
	qFatal(qPrintable(msg));
      }
      if (sscanf(argv[i], "%lf", &lat) != 1) {
	QString msg = QApplication::tr("Latitude is not a number");
	QMessageBox::critical( 0, QApplication::tr("Load error" ), msg, QMessageBox::Abort,0 );
	qFatal(qPrintable(msg));
      }
      lonlat_req=true;
    } else if (arg == "--version") {
      std::cout << "RATMAN 3D - " << ratman::version::author_str() << std::endl;
      std::cout << "Version: " << ratman::version::version_str() 
		<< " - " <<  ratman::version::date_str() << std::endl;
      exit(0);
    } else {
      s3d_url = std::string(argv[i]);
    }
  }

  // Parsing s3d file
  if (!s3d_url.empty()) {
    ratman::s3d_parser parser;
    parser.read_and_parse(s3d_url.c_str());
    lon=parser.lon();
    lat=parser.lat();
  }
  QString goto_msg;
  QString goto_tcp_msg;
  if (lon != -1000.0 && lat != -1000.0) {
    goto_msg.sprintf("%f %f",lon,lat);
    goto_tcp_msg.sprintf("lonlat=%f,%f",lon,lat);
  }

#ifdef QTSINGLEAPPLICATION
  if (app.isRunning()) {
    if (!goto_msg.isEmpty()) {
      app.sendMessage(goto_msg);
    }
    return 0;
  }
  
  app.initialize();
#else
  ratman::tcp_client mainclient("id;");
  while (mainclient.isRunning ()) {
  } 
  if (mainclient.server_found() && !mainclient.error()) {
    if (!goto_tcp_msg.isEmpty()) {
      ratman::tcp_client client(goto_tcp_msg);
      while (client.isRunning ()) {
      }
    }
    return 0;
  }
#endif
  

  if (filename.empty()) {
    ratman::config* cfg=ratman::config::instance();
    if (cfg->error()) {
      std::cerr << cfg->error_msg().toStdString() << std::endl;
      exit(1);
    }
    filename = cfg->index_file_url().toStdString();
    
    QCoreApplication::addLibraryPath(cfg->qt_plugins_dir());

    if(cfg->language_ext() != "en") {
      QTranslator *qtTranslator = new QTranslator();
      qtTranslator->load(cfg->translations_qt_file(),cfg->translations_dir());
      app.installTranslator(qtTranslator);
      QTranslator *myappTranslator = new QTranslator();
      myappTranslator->load(cfg->translations_file(),cfg->translations_dir());
      app.installTranslator(myappTranslator);
    }
  }

  // test if OpenGL is available on this system
  if ( !QGLFormat::hasOpenGL() )  {
    QString msg = QApplication::tr("OpenGL not supported on this system. Unable to run application.");
    QMessageBox::critical( 0, QApplication::tr("Graphics error" ), msg, QMessageBox::Abort,0 );
    qFatal(qPrintable(msg));
  }

  ////////////// LOAD
  QString msg = QApplication::tr("Loading '%1'...").arg(QString::fromLatin1(filename.c_str()));
  QProgressDialog progress(msg, QApplication::tr("Abort"), 0, 100);
    
  ratman::xml_config_parser config_parser(filename.c_str());

  QXmlSimpleReader reader;
  reader.setContentHandler(&config_parser);
  reader.setErrorHandler(&config_parser);
  
  QByteArray data;
  vic::icurlstream ifile;
  ifile.open(filename.c_str());
  if (!ifile) {
    QString msg = QApplication::tr("Failed to open '%1'").arg(QString::fromLatin1(filename.c_str()));
    QMessageBox::critical( 0, QApplication::tr("Load error" ), msg, QMessageBox::Abort,0 );
    qFatal(qPrintable(msg));
  } else {
    progress.setValue(10); // signal loaded
    while (ifile.good()) {
      app.processEvents();
      if (progress.wasCanceled()) {
	exit(0); // FIXME
      }

      std::string line;
      std::getline(ifile,line);
      if(!line.empty()) {
	data.append(QString(line.c_str()));
	data.append('\n');
      }
    }
    progress.setValue(30); // signal loaded
    app.processEvents();
    if (progress.wasCanceled()) {
      exit(0); // FIXME
    }

    if (ifile.fail() &&!ifile.eof()) {
      QString msg = QApplication::tr("Failed to read '%1'").arg(filename.c_str());      
      QMessageBox::critical( 0, QApplication::tr("Load error" ), msg, QMessageBox::Abort,0 );
      qFatal(qPrintable(msg));
    }
    ifile.close();
  }

  if (data.isEmpty()) {
    QString msg = QApplication::tr("Failed to read '%1'").arg(filename.c_str());
    QMessageBox::critical( 0, QApplication::tr("Load error" ), msg, QMessageBox::Abort,0 );
    qFatal(qPrintable(msg));
  }


  QBuffer file(&data);
  QXmlInputSource xml_input(&file);
  if (reader.parse(xml_input)) {
    progress.setValue(90);
    app.processEvents();
    if (progress.wasCanceled()) {
      exit(0); // FIXME
    }

    AppWindow *gui = new AppWindow(config_parser);
    progress.setValue(100);
    app.processEvents();
    if (progress.wasCanceled()) {
      exit(0); // FIXME
    }

    gui->show();

#ifdef QTSINGLEAPPLICATION
    app.setActivationWindow(gui);
#else
    ratman::tcp_server server;
#endif

    //    QObject::connect(&app, SIGNAL(messageReceived(const QString&)),
    //    		     gui, SLOT(handling_msg(const QString&)));


    ratman::decorated_terrain_view *terrain_view=config_parser.decorated_terrain_view();
    terrain_view->async_update_start();

    int result = app.exec();
    terrain_view->async_update_stop();
    delete terrain_view;
    return result;
  } else {
    // parse error
    QString msg = QApplication::tr("Parse error: malformed input file '%1'").arg(filename.c_str());
    QMessageBox::critical( 0, QApplication::tr("Load error" ), msg, QMessageBox::Abort,0 );
    qFatal(qPrintable(msg));
    return 1;
  }  
}


