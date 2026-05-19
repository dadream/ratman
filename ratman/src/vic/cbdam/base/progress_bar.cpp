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
#include <vic/cbdam/base/progress_bar.hpp>

#ifdef _WIN32
#include <windows.h>
#endif
#include <vic/gl/gl.hpp>

namespace cbdam {

#if 0
  progress_bar::progress_bar(float x, float y, float l, bool is_vertical, bool look3d) :
    x_(x), y_(y), length_(l), is_vertical_(is_vertical), look3d_(look3d) {

  }
#endif

  progress_bar::progress_bar(bool is_vertical, bool look3d) :
    is_vertical_(is_vertical), look3d_(look3d) {

  }

  progress_bar::~progress_bar() {

  }

  void progress_bar::draw(float value) {
    GLint xywh[4];
    glGetIntegerv(GL_VIEWPORT, xywh);
#if 0
    float x_min = (xywh[2] * x_);
    float y_min = (xywh[3] * y_);
    float l = is_vertical_ ? (length_ * xywh[3]) : (length_ * xywh[2]);
    std::cerr << "progress_bar l " << l << std::endl;
#else
    // FIXME ignore x y lenght 
    float l = 128;
    float x_offset = is_vertical_ ? 24 : 128+16;
    float y_offset = is_vertical_ ? 128+16 : 24;
    float x_min = xywh[2] - x_offset;
    float y_min = xywh[3] - y_offset;
#endif
    float x_max = x_min;
    float y_max = y_min;
    const float bar_ratio = 8.0f;
    //    std::cerr << "progress bar " << x_min << " " << y_min << " " << l << std::endl;
    if (is_vertical_) {
      x_max += l/bar_ratio;
      y_max += l;
    } else {
      x_max += l;
      y_max += l/bar_ratio;     
    }
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, xywh[2], 0, xywh[3]);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw border
    glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
    glLineWidth(2);
    glBegin(GL_LINE_STRIP);
    glVertex2f(x_min, y_min);    
    glVertex2f(x_min, y_max);
    glVertex2f(x_max, y_max);    
    glVertex2f(x_max, y_min);    
    glVertex2f(x_min, y_min);
    glEnd();
    glLineWidth(1);

    // draw progress quad
    const float border = 3;
    x_min += border;
    y_min += border;
    if (is_vertical_) {
      y_max = y_min + ((l - 2*border)* value);
      x_max -= border;
    } else {
      y_max -= border;
      x_max = x_min + ((l - 2*border) * value);
    }

    //    std::cerr << "progress_bar::draw x_min " << x_min  << ", x_max " << x_max << ", v " << value << "  \r";
    if (look3d_) {
      const uint8_t v0 = 130;
      const uint8_t v1 = 180;
      const uint8_t v2 = 200;
      const uint8_t v3 = 255;
      const uint8_t data[] = {v1, v1, v1, 255,
			      v2, v2, v2, 255,
			      v3, v3, v3, 255,
			      v3, v3, v3, 255,
			      v2, v2, v2, 255,
			      v1, v1, v1, 255,
			      v0, v0, v0, 255,
			      v0, v0, v0, 255};
			    
      glEnable(GL_TEXTURE_1D);
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
      glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S,GL_CLAMP);
      glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T,GL_CLAMP);
      glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 
		   8, 0, GL_RGBA, GL_UNSIGNED_BYTE,
		   data);

      glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
      if (is_vertical_) {
	glBegin(GL_QUADS);
	glTexCoord1f(1.0f);    glVertex2f(x_min, y_min);    
	glTexCoord1f(1.0f);    glVertex2f(x_min, y_max);
	glTexCoord1f(0.0f);    glVertex2f(x_max, y_max);    
	glTexCoord1f(0.0f);    glVertex2f(x_max, y_min);    
	glEnd();
      } else {
	glBegin(GL_QUADS);
	glTexCoord1f(1.0f);    glVertex2f(x_min, y_min);    
	glTexCoord1f(0.0f);    glVertex2f(x_min, y_max);
	glTexCoord1f(0.0f);    glVertex2f(x_max, y_max);    
	glTexCoord1f(1.0f);    glVertex2f(x_max, y_min);    
	glEnd();
      }
    } else {
      glColor4f(1.0f, 1.0f, 1.0f, 0.7f);
      glBegin(GL_QUADS);
      glVertex2f(x_min, y_min);    
      glVertex2f(x_min, y_max);
      glVertex2f(x_max, y_max);    
      glVertex2f(x_max, y_min);    
      glEnd();
    }
    glPopAttrib();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
  }

} // namespace cbdam
