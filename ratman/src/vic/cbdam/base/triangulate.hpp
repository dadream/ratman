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
#ifndef SL_TRIANGULATE_HPP
#define SL_TRIANGULATE_HPP

#include <sl/fixed_size_point.hpp>
#include <sl/cstdint.hpp>
#include <vector>
#include <cassert>

namespace sl {

  /**
   * Simple polygon triangulation.
   * Based on flipcode article:
   * http://www.flipcode.org/cgi-bin/fcarticles.cgi?show=63943
   * COTD Entry submitted by John W. Ratcliff [jratcliff@verant.com]
   */
  template <class T>
  class simple_polygon_triangulator {
  public:
    typedef T value_t;
    typedef sl::fixed_size_point<2,value_t> point2_t;
  public:

    /// compute area of a contour/polygon
    static value_t area(const std::vector<point2_t> &contour) {
      const int n = (int)contour.size();
      value_t A=0;
      for(int p=n-1,q=0; q<n; p=q++) {
	A+= contour[p][0]*contour[q][1] - contour[q][0]*contour[p][1];
      }
      return A*0.5f;
    }

    // decide if point Px/Py is inside triangle defined by
    // (Ax,Ay) (Bx,By) (Cx,Cy)
    static bool is_inside_triangle(value_t Ax, value_t Ay,
				   value_t Bx, value_t By,
				   value_t Cx, value_t Cy,
				   value_t Px, value_t Py) {
      value_t ax, ay, bx, by, cx, cy, apx, apy, bpx, bpy, cpx, cpy;
      value_t cCROSSap, bCROSScp, aCROSSbp;

      ax = Cx - Bx;  ay = Cy - By;
      bx = Ax - Cx;  by = Ay - Cy;
      cx = Bx - Ax;  cy = By - Ay;
      apx= Px - Ax;  apy= Py - Ay;
      bpx= Px - Bx;  bpy= Py - By;
      cpx= Px - Cx;  cpy= Py - Cy;

      aCROSSbp = ax*bpy - ay*bpx;
      cCROSSap = cx*apy - cy*apx;
      bCROSScp = bx*cpy - by*cpx;

      return ((aCROSSbp >= 0) && (bCROSScp >= 0) && (cCROSSap >= 0));
    }


    /**
     * triangulate a contour/polygon, places results in STL vector
     * as series of triangles. Returns empty result in case of error.
     */
    static void process(const std::vector<point2_t>& contour,
			std::vector<uint32_t>& result) {
      result.clear();
      
      int n = (int)contour.size();
      if ( n < 3 ) return; // ERROR

      int *V = new int[n];

      /* we want a counter-clockwise polygon in V */

      if ( 0.0f < area(contour) )
	for (int v=0; v<n; v++) V[v] = v;
      else
	for(int v=0; v<n; v++) V[v] = (n-1)-v;
      
      int nv = n;

      /*  remove nv-2 Vertices, creating 1 triangle every time */
      int count = 2*nv;   /* error detection */

      for(int m=0, v=nv-1; nv>2; ) {
	/* if we loop, it is probably a non-simple polygon */
	if (0 >= (count--)) {
	  //** Triangulate: ERROR - probable bad polygon!
	  result.clear(); 
	  return;
	}

	/* three consecutive vertices in current polygon, <u,v,w> */
	int u = v  ; if (nv <= u) u = 0;     /* previous */
	v = u+1; if (nv <= v) v = 0;     /* new v    */
	int w = v+1; if (nv <= w) w = 0;     /* next     */
	
	if (snip(contour,u,v,w,nv,V)) {
	  int a,b,c,s,t;

	  /* true names of the vertices */
	  a = V[u]; b = V[v]; c = V[w];
	  
	  /* output Triangle */
	  result.push_back(uint32_t(a));
	  result.push_back(uint32_t(b));
	  result.push_back(uint32_t(c));

	  m++;
	  
	  /* remove v from remaining polygon */
	  for(s=v,t=v+1;t<nv;s++,t++) V[s] = V[t];
	  nv--;

	  /* resest error detection counter */
	  count = 2*nv;
	}
      }
      delete [] V;
    }

  protected:
    
    static bool snip(const std::vector<point2_t> &contour,
		     int u,int v,int w,
		     int n,
		     int *V) {
      static const float EPSILON=0.0000000001f;

      int p;
      float Ax, Ay, Bx, By, Cx, Cy, Px, Py;

      Ax = contour[V[u]][0];
      Ay = contour[V[u]][1];
      
      Bx = contour[V[v]][0];
      By = contour[V[v]][1];
      
      Cx = contour[V[w]][0];
      Cy = contour[V[w]][1];
      
      if ( EPSILON > (((Bx-Ax)*(Cy-Ay)) - ((By-Ay)*(Cx-Ax))) ) return false;
      
      for (p=0;p<n;p++) {
	if( (p == u) || (p == v) || (p == w) ) continue;
	Px = contour[V[p]][0];
	Py = contour[V[p]][1];
	if (is_inside_triangle(Ax,Ay,Bx,By,Cx,Cy,Px,Py)) return false;
      }
      
      return true;
    }
    
  };

} // namespace sl

#endif
