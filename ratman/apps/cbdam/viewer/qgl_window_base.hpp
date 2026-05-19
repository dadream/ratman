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
#ifndef VIEWER_QGL_WINDOW_BASE_H
#define VIEWER_QGL_WINDOW_BASE_H

#include <GL/glew.h>
#include <QGLWidget>

/** 
 * Base class for windows which use OpenGl.
 * This class simply set up an OpenGL window
 * and displays a white square.
 */
class qgl_window_base : public QGLWidget
{

  Q_OBJECT

public:
  /// Default ctor: recalls base class QWidget ctor 
  qgl_window_base(QWidget* parent);

  /// Destructor recalls makeCurrent()
  virtual ~qgl_window_base();

public slots:
 

protected:

  /// Initialize settings with FLAT shade and enabling one light
  virtual void initializeGL();

  /// Paints a demo quad in front of the user
  virtual void paintGL();

  /// set view frustum to perspective in interval [-1,1],[-1,1],[1,100]
  virtual void resizeGL( int w, int h );

  // mouse events
  // virtual void mouseMoveEvent ( QMouseEvent * e );
  // virtual void mousePressEvent ( QMouseEvent * e );

private:
 

};

#endif // VIEWER_QGL_WINDOW_BASE_H
