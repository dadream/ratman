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
#include <vic/cbdam/base/img_operations.hpp>
#include <vic/cbdam/base/imgfilter.hpp>

#include <sl/utility.hpp>
#include <math.h>
#include <QColor>
#include <iostream>

namespace cbdam {

  QImage img_operations::zoom(const QImage& src, int dst_width, int dst_height, imgfilter* filter) {
    if (src.width()==0 || src.height()==0 || src.format()!=QImage::Format_ARGB32) {
      std::cerr << "Img rescaling: src image is malformed!" << std::endl;
      return QImage(0,0,src.format());
    }
    if (filter==0) {
      return src.scaled(dst_width, dst_height,
			Qt::IgnoreAspectRatio,
			Qt::FastTransformation);
    } else {
      QImage tmp(dst_width, src.height(), src.format());
      double xscale = double(dst_width)  / double(src.width());
      double yscale = double(dst_height) / double(src.height());
      std::vector< std::vector<contrib_item_t> > contrib;

      contrib.resize(dst_width);

      if (xscale < 1.0f) {
	double w = filter->aperture() / xscale;
	double fscale = 1.0 / xscale;
	for (int i=0; i < dst_width; ++i) {
	  //	  contrib[i].resize(int(w)*2+1);
	  double center = double(i)/xscale;
	  double left = ceil(center - w);
	  double right = floor(center + w);
	  for (int j=int(left); j<=int(right); ++j) {
	    double weight = center - double(j);
	    weight = filter->value(weight/fscale)/fscale;
	    int pixel=j;
	    if (j < 0) pixel=-j;
	    else if (j >= src.width()) pixel=(src.width()-j) + src.width()-1;
	    contrib_item_t ci;
	    ci.first=pixel;
	    ci.second=weight;
	    contrib[i].push_back(ci);
	  }
	}
      } else {
	for (int i=0; i < dst_width; ++i) {
	  double center = double(i)/xscale;
	  double left = ceil(center - filter->aperture());
	  double right = floor(center + filter->aperture());
	  for (int j=int(left); j<=int(right); ++j) {
	    double weight = center - double(j);
	    weight = filter->value(weight);
	    int pixel=j;
	    if (j < 0) pixel=-j;
	    else if (j >= src.width()) pixel=(src.width()-j) + src.width()-1;
	    contrib_item_t ci;
	    ci.first=pixel;
	    ci.second=weight;
	    contrib[i].push_back(ci);	     
	  }	  
	}	
      }
      // zoom horizontally
      for (int k=0; k<tmp.height(); ++k) {
	for (int i=0; i<tmp.width(); ++i) {
	  double wtot[3];
	  wtot[0]=0.0; wtot[1]=0.0; wtot[2]=0.0;
	  for (int j=0; j<int(contrib[i].size()); ++j) {
	    QRgb c = src.pixel(contrib[i][j].first,k);
	    wtot[0] += qRed(c)*contrib[i][j].second;
	    wtot[1] += qGreen(c)*contrib[i][j].second;
	    wtot[2] += qBlue(c)*contrib[i][j].second;
	  }
	  int r = int(sl::median(wtot[0], 0.0, 255.0));
	  int g = int(sl::median(wtot[1], 0.0, 255.0));
	  int b = int(sl::median(wtot[2], 0.0, 255.0));
	  QColor col(r,g,b,b);
	  tmp.setPixel(i,k,col.rgba());
	}
      }      

      for(int i=0; i<int(contrib.size()); ++i) {
	contrib[i].clear();
      }
      contrib.clear();

      contrib.resize(dst_height);
      if (yscale < 1.0f) {
	double w = filter->aperture() / yscale;
	double fscale = 1.0 / yscale;
	for (int i=0; i < dst_height; ++i) {
	  //	  contrib[i].resize(int(w)*2+1);
	  double center = double(i)/yscale;
	  double left = ceil(center - w);
	  double right = floor(center + w);
	  for (int j=int(left); j<=int(right); ++j) {
	    double weight = center - double(j);
	    weight = filter->value(weight/fscale)/fscale;
	    int pixel=j;
	    if (j < 0) pixel=-j;
	    else if (j >= tmp.height()) pixel=(tmp.height()-j) + tmp.height()-1;
	    contrib_item_t ci;
	    ci.first=pixel;
	    ci.second=weight;
	    contrib[i].push_back(ci);
	  }
	}
      } else {
	for (int i=0; i < dst_height; ++i) {
	  double center = double(i)/yscale;
	  double left = ceil(center - filter->aperture());
	  double right = floor(center + filter->aperture());
	  for (int j=int(left); j<=int(right); ++j) {
	    double weight = center - double(j);
	    weight = filter->value(weight);
	    int pixel=j;
	    if (j < 0) pixel=-j;
	    else if (j >= tmp.height()) pixel=(tmp.height()-j) + tmp.height()-1;
	    contrib_item_t ci;
	    ci.first=pixel;
	    ci.second=weight;
	    contrib[i].push_back(ci);	     
	  }	  
	}	
      }
      // zoom vertically
      QImage dst(dst_width, dst_height, src.format());
     
      for (int k=0; k<dst.width(); ++k) {
	for (int i=0; i<dst.height(); ++i) {
	  double wtot[3];
	  wtot[0]=0.0f; wtot[1]=0.0f; wtot[2]=0.0f;
	  for (int j=0; j<int(contrib[i].size()); ++j) {
	    QRgb c = tmp.pixel(k,contrib[i][j].first);
	    wtot[0] += qRed(c)*contrib[i][j].second;
	    wtot[1] += qGreen(c)*contrib[i][j].second;
	    wtot[2] += qBlue(c)*contrib[i][j].second;
	  }
	  int r = int(sl::median(wtot[0], 0.0, 255.0));
	  int g = int(sl::median(wtot[1], 0.0, 255.0));
	  int b = int(sl::median(wtot[2], 0.0, 255.0));
	  QColor col(r,g,b,b);
	  dst.setPixel(k,i,col.rgba());
	}
      }      

      for(int i=0; i<int(contrib.size()); ++i) {
	contrib[i].clear();
      }
      contrib.clear();
      
      return dst;
    }
  }

} // namespace cbdam
