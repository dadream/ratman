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
#include <QImage>
#include <QGLWidget>
#include <iostream>
#include <vic/img/gl_quadtree_image_processor.hpp>

int main(int argc,
         const char* argv[]) {
#if 1
  if (argc != 3) {
    qFatal("Usage: %s <imgsrc> <imgout>", argv[0]);
  }

  const char* imgname_src = argv[1];
  const char* imgname_out = argv[2];
  
  QImage qimg_src(imgname_src);
  if (qimg_src.isNull()) {
    qFatal("Unable to read %s", imgname_src);
  }

  qimg_src = qimg_src.scaled(256,256,Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
  //  qimg_src = qimg_src.convertToFormat(QImage::Format_ARGB32);

  QImage qimg_src_gl = QGLWidget::convertToGLFormat(qimg_src);

  vic::img::gl_image<> gl_img_src;
  gl_img_src.assign(qimg_src.bits(),
		    4, // RGBA
		    256,
		    256);

  vic::img::gl_image<> gl_img_dst;

  gl_img_dst.reshape(4, // RGBA
		     256,
		     256);

  vic::img::gl_quadtree_image_processor gl_quadproc;

  gl_quadproc.magnify_in(gl_img_dst,
			 2, 0, 0,
			 gl_img_src,
			 0, 0, 0);
  QImage qimg_dst = QImage(gl_img_dst.to_pointer(),
			   256,
			   256,
			   QImage::Format_ARGB32); // FIXME

  qimg_dst.save(imgname_out);
		    
  return 0;
#else

  if (argc != 4) {
    qFatal("Usage: %s <imgsrc> <imgout>", argv[0]);
  }

  const char* imgname_src0 = argv[1];
  const char* imgname_src1 = argv[2];
  const char* imgname_out = argv[3];
  
  QImage qimg_src0(imgname_src0);
  QImage qimg_src1(imgname_src1);
  if (qimg_src0.isNull()) {
    qFatal("Unable to read %s", imgname_src0);
  }
  if (qimg_src1.isNull()) {
    qFatal("Unable to read %s", imgname_src1);
  }
  qimg_src0 = qimg_src0.convertToFormat(QImage::Format_ARGB32);

  QImage qimg_src_gl0 = QGLWidget::convertToGLFormat(qimg_src0.scaled(256,256,Qt::IgnoreAspectRatio, Qt::SmoothTransformation));
  QImage qimg_src_gl1 = QGLWidget::convertToGLFormat(qimg_src1.scaled(256,256,Qt::IgnoreAspectRatio, Qt::SmoothTransformation));

  vic::img::gl_image<> gl_img_src0;
  gl_img_src0.assign(qimg_src0.bits(),
		     4, // RGBA
		     256,
		     256);
  vic::img::gl_image<> gl_img_src1;
  gl_img_src1.assign(qimg_src1.bits(),
		     4, // RGBA
		     256,
		     256);
  gl_img_src1.set_alpha_from_black();

  vic::img::gl_image<> gl_img_dst;

  gl_img_dst.reshape(4, // RGBA
		     256,
		     256);

  vic::img::gl_quadtree_image_processor gl_quadproc;

  gl_quadproc.blend_in(gl_img_dst,
		       0, 0, 0,
		       gl_img_src0,
		       0, 0, 0);

  gl_quadproc.blend_in(gl_img_dst,
		       2, 0, 3,
		       gl_img_src1,
		       0, 0, 0);

  QImage qimg_dst = QImage(gl_img_dst.to_pointer(),
			   256,
			   256,
			   QImage::Format_ARGB32); // FIXME

  qimg_dst.save(imgname_out);
		    
  return 0;
#endif
}
