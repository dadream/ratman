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
#include <qgl_window_base.hpp>


qgl_window_base::qgl_window_base( QWidget* parent, const char* name )
: QGLWidget( parent, name ){

}  

qgl_window_base::~qgl_window_base(){
  makeCurrent();
}

void qgl_window_base::initializeGL(){
  qglClearColor( Qt::black );
  glShadeModel( GL_FLAT );
}

/// Paint a demo quad in front of the user
void qgl_window_base::paintGL(){
  GLfloat init_z(5);
  glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
  glLoadIdentity();
  glTranslatef( 0.0f, 0.0f,-init_z );
  glBegin( GL_POLYGON );
  glNormal3f( 0.0f, 0.0f, 1.0f );  
  glVertex3f(-1.0f, 1.0f, 0.0f );
  glVertex3f(-1.0f,-1.0f, 0.0f );
  glVertex3f( 1.0f,-1.0f, 0.0f );
  glVertex3f( 1.0f, 1.0f, 0.0f );
  glEnd();
}

/// resize the window setting up a pre-defined view-frustum
void qgl_window_base::resizeGL( int w, int h ){
  float aspect_ratio, dim, p_near, p_far;
  aspect_ratio = ( h==0 ) ? 1 :  ((float)w / (float)h);
  p_near = 0.2f;
  dim = p_near;
  p_far = 500;
  glViewport( 0, 0, (GLint)w, (GLint)h );
  glMatrixMode( GL_PROJECTION ); 
  glLoadIdentity();
  gluPerspective(30,  aspect_ratio, p_near, p_far );
  glMatrixMode( GL_MODELVIEW );
}
