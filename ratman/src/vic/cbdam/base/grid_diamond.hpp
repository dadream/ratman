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
#ifndef VIC_CBDAM_GRID_DIAMOND_HPP
#define VIC_CBDAM_GRID_DIAMOND_HPP

#include <vic/cbdam/base/grid_point.hpp>
#include <cassert>
#include <sl/serializer.hpp>
#include <iostream>

namespace cbdam {

  /**
   * A diamond represented by the four points on
   * a 3D grid.
   */
  class grid_diamond {
  public:
    typedef grid_diamond this_t;
  protected:
    grid_point_t corners_[4];
  public:

    // O and 2 are diagonal
    inline explicit grid_diamond(const grid_point_t& x0 = invalid_grid_point(),
                                 const grid_point_t& x1 = invalid_grid_point(),
                                 const grid_point_t& x2 = invalid_grid_point(),
                                 const grid_point_t& x3 = invalid_grid_point()) {
      corners_[0] = x0;
      corners_[1] = x1;
      corners_[2] = x2;
      corners_[3] = x3;
    }

    inline ~grid_diamond() {
    }

  public: // Serialization
    
    inline void store_to(sl::output_serializer& s) const {
      corners_[0].store_to(s);
      corners_[1].store_to(s);
      corners_[2].store_to(s);
      corners_[3].store_to(s);
    }
    
    inline void retrieve_from(sl::input_serializer& s) {
      corners_[0].retrieve_from(s);
      corners_[1].retrieve_from(s);
      corners_[2].retrieve_from(s);
      corners_[3].retrieve_from(s);
    }

  public: // corners

    inline bool is_valid() const {
      return
        (corner(0) != invalid_grid_point()) ||
        (corner(2) != invalid_grid_point());
    }

    inline grid_point_t corner(int i) const {
      assert(i<4);
      return corners_[i];
    }

    inline void set_corner(int i, const grid_point_t& x) {
      assert(i<4);
      corners_[i] = x;
    }
    
  public: // Ids and subdivision
      
    inline grid_point_t id() const {
      return midpoint(corners_[0],corners_[2]);
    }

    inline this_t key() const {
      grid_point_t this_id = id();
      return this_t(this_id, this_id, this_id, this_id);
    }
    
    inline grid_point_t parent_id(int i) const {
      assert(i >= 0 && i < 2);
      return corners_[1+2*i];
    }

    inline grid_point_t child_id(int i, int j) const {
      /// get midpoint of the edge going from corner idx to corner (idx+1)%4
      int idx = 2*i+j;
      const grid_point_t ip = invalid_grid_point();
      if (corners_[idx] == ip || corners_[(idx+1)&0x3] == ip) {
        // invalid point if one of the corner is not valid
        return ip;
      } else {
        return midpoint(corners_[idx], corners_[(idx+1)%4]);
      }
    }
    
    inline bool subdividable() const {
      const grid_point_t my_id = id();
      return 
	my_id != corners_[0] &&
	my_id != corners_[2];
    }

  public: // Subdivision

    static inline this_t canonical_root(int i) {
      assert(i >= 0 && i < 6);
      /**
       * Each column is a face. Face order is not important, except the first one.
       *  (which is used also from planar).
       * Diamond main diagonal start at first corner, ends at third.
       * With right hand rule, thumb points to the inner of the cube
       * First 3 corners define fragment 0. Last 2 + first define fragment 1
       * Corners MUST be arranged such that corners of a child have the same order
       * both if generated from each of the two parents.
       * Child 0 is made of corners 1,id,0,out (and has fragment 0 inside, fragment 1 outside)
       * Child 1 is made of corners 1,out,2,id (and has fragment 1 inside, fragment 0 outside)
       * Child 2 is made of corners 3,id,2,out (and has fragment 0 inside, fragment 1 outside)
       * Child 3 is made of corners 3,out,0,id (and has fragment 1 inside, fragment 0 outside)
       * This situation must be coherent among all the adjacent faces.
       * Thanks God for its precious help!
       */
      //                            Fz,Fx,Fy,Bz,Bx,By
      static const uint32_t i0[] = { 0, 2, 2, 7, 5, 7};
      static const uint32_t i1[] = { 1, 6, 1, 6, 1, 4};
      static const uint32_t i2[] = { 2, 7, 5, 5, 0, 0};
      static const uint32_t i3[] = { 3, 3, 6, 4, 4, 3};

      return this_t(grid_canonical_point(i0[i]),
                    grid_canonical_point(i1[i]),
                    grid_canonical_point(i2[i]),
                    grid_canonical_point(i3[i]));
    }

    bool child_ij_from_child_id(const grid_point_t& c_id, int& i, int& j) const {
      bool result = false;
      for(i = 0; i < 2 && !result; ++i) {
        for(j = 0; j < 2 && !result; ++j) {
          result = (c_id == child_id(i,j));
        }
      }
      --i; --j;
      return result;
    }

    inline this_t child_diamond(int i, int j, const grid_point_t& other_corner) const {
      // other_corner is the external one deriving from the other parent
      // new corners meaning:
      //   c0 is the id() of the grand-parent, which is father of the 2 parents.
      //   nexts are clockwise ordered.
      //   c1 is parent 0, c3 is parent 1.

      if (i == 0) {
        if (j == 0) {
          // 1, id, 0, oc
          return this_t(corners_[1], id(), corners_[0], other_corner);
        } else {
          // 1, oc, 2, id
          return this_t(corners_[1], other_corner, corners_[2], id());
        }
      } else {
        if (j == 0) {
          // 3, id, 2, oc
          return this_t(corners_[3], id(), corners_[2], other_corner);
        } else {
          // 3, oc, 0, id
          return this_t(corners_[3], other_corner, corners_[0], id());
        }        
      }
    }


    inline this_t canonical_planar_child_diamond(int i, int j) const {
      // other_corner is the external one deriving from the other parent
      const grid_point_t center = id();
      const grid_point_t child_center = child_id(i, j);
      const grid_point_t other_corner = child_center + (child_center - center);

      return child_diamond(i, j, other_corner);
    }

    inline this_t canonical_spherical_child_diamond(int i, int j) const {
      // FIXME NOT WORKING!!!!
      static grid_value_t gridmin = min_grid_coord();
      static grid_value_t gridmax = max_grid_coord();

      // Try with canonical planar extension, then wrap around to remain on cube
      this_t d = canonical_planar_child_diamond(i,j);

      // check if one of the 2 corners v1, v3 are outside bounds
      grid_point_t c_to_exchage[2] = {d.corner(1), d.corner(3)};
      bool something_has_changed = false;
      for(int i = 0; i < 2; ++i) {
        grid_point_t c = c_to_exchage[i];

        bool inside_bounds = true;
        int delta = 0;
        int i_out;
        for(i_out = 0; i_out < 3 && inside_bounds; ++i_out) {
          // if one coord is outside border, clamp it to bounds
          if (c[i_out] < gridmin) {
            delta = gridmin - c[i_out];
            c[i_out] = gridmin;
            inside_bounds = false;
          } else if (c[i_out] > gridmax) {
            delta = c[i_out] - gridmax;
            c[i_out] = gridmax;
            inside_bounds = false;
          }
        }

        if (!inside_bounds) {
          something_has_changed = true;

          // 1 step back to get exit value
          i_out -= 1;
#if 1
          // identify which of the 2 other coords have to be modified
          bool found = false;
          int i1 = (i_out + 1) % 3;
          while(i1 != i_out && !found) {
            // move toward center of delta the coords which lies on the boundaries
            if (c[i1] == gridmax) {
              c[i1] -= delta;
              found = true;
            } else if (c[i1] == gridmin) {
              c[i1] += delta;
              found = true;
            } else {
              i1 = (i1 + 1) % 3;
            }
          }
#else
	  bool found = true;
#endif
          c_to_exchage[i] = c;
          if (!found) {
            SL_TRACE_OUT(-1) << "not found second component to move " << d.corner(2*i+1) << std::endl;
          }
        }
      }
      if (something_has_changed) {
	//	std::cerr << d.corner(1) << " : " << d.corner(3) << " vs " <<  c_to_exchage[0] << " : " << c_to_exchage[1] << std::endl;
        return this_t(d.corner(0), c_to_exchage[0], d.corner(2), c_to_exchage[1]);
      } else {
        return d;
      }
    }
    

    inline this_t canonical_cylindrical_child_diamond(int i, int j) const {
      // for cylindrical check only on x,z, because there is no wrap around on top y.
      // for cylindrical max and min are half the original min max.

      static grid_value_t gridmin = min_grid_coord()/2;
      static grid_value_t gridmax = max_grid_coord()/2;

      // Try with canonical planar extension, then wrap around to remain on cube
      this_t d = canonical_planar_child_diamond(i,j);

      // check if one of the 2 corners v1, v3 are outside bounds
      grid_point_t c_to_exchage[2] = {d.corner(1), d.corner(3)};

      // indices to check are only 0, 2: remap i_out_idx 0,1 ->0,2
      bool something_has_changed = false;
      for(int i = 0; i < 2; ++i) {
        grid_point_t c = c_to_exchage[i];

        bool inside_bounds = true;
        int delta = 0;
        int i_out;
        for(i_out = 0; i_out < 3 && inside_bounds; i_out+=2) {
          // if one coord is outside border, clamp it to bounds
          if (c[i_out] < gridmin) {
            delta = gridmin - c[i_out];
            c[i_out] = gridmin;
            inside_bounds = false;
          } else if (c[i_out] > gridmax) {
            delta = c[i_out] - gridmax;
            c[i_out] = gridmax;
            inside_bounds = false;
          }
        }

        if (!inside_bounds) {
          // 1 step back to get exit value
          i_out -= 2;
          int other_idx = (i_out+2)%4; // 0 V 2
          if (c[other_idx] == gridmax) {
            c[other_idx] -= delta;
          } else if (c[other_idx] == gridmin) {
            c[other_idx] += delta;
          } else {
            SL_TRACE_OUT(-1) << "not found second component to move " << d.corner(2*i+1) << std::endl;
          }
          c_to_exchage[i] = c;
          something_has_changed = true;
        }
      }
      if (something_has_changed) {
        //      std::cerr << d.corner(1) << " : " << d.corner(3) << " vs " <<  c_to_exchage[0] << " : " << c_to_exchage[1] << std::endl;
        return this_t(d.corner(0), c_to_exchage[0], d.corner(2), c_to_exchage[1]);
      } else {
        return d;
      }
    }

  public: // Fragments

    inline bool is_valid_fragment(int i) const {
      assert(i >= 0 && i < 2);

      static grid_value_t gridmin = min_grid_coord();
      static grid_value_t gridmax = max_grid_coord();
      
      const grid_point_t& c = corners_[2*i + 1];
      return
        (c[0]>=gridmin) && (c[0]<=gridmax) &&
        (c[1]>=gridmin) && (c[1]<=gridmax) &&
        (c[2]>=gridmin) && (c[2]<=gridmax);
    }
    
    
    ///  which is the fragment idx of d, which derives from parent_id ? 0 : 1
    inline int fragment_id_deriving_from_parent(const grid_point_t& parent_id) const {
      if (parent_id == corners_[1]) {
        return 0;
      } else if (parent_id == corners_[3]) {
        return 1;
      } else {
        SL_TRACE_OUT(-1) << std::endl << 
	  "parent_id " << parent_id << ","  << " not a parent of " << id() << std::endl << 
	  "corners GOOD: " << corners_[1] << " " << corners_[3] << std::endl << 
	  "corners BAD : " << corners_[0] << " " << corners_[2] << std::endl;
        abort();
        return -1;
      }
    }

    // quadtree functions: diamond graph on even level: no rotated quads. 
  public:
    static inline this_t cylindrical_canonical_root(int i) {
#if 1
      static const uint32_t i0[] = { 2, 9, 9, 2,   2, 9, 9, 2};
      static const uint32_t i1[] = { 4, 3,11, 8,   3, 7, 8, 0};
      static const uint32_t i2[] = { 5, 5,10,10,   1, 1, 6, 6};
      static const uint32_t i3[] = { 3,11, 8, 4,   0, 3, 7, 8};
#else
      static const uint32_t i0[] = { 2, 9,10, 2,   2, 9, 6, 2};
      static const uint32_t i1[] = { 4, 3, 8, 8,   3, 7, 7, 0};
      static const uint32_t i2[] = { 5, 5, 9,10,   1, 1, 9, 6};
      static const uint32_t i3[] = { 3,11,11, 4,   0, 3, 8, 8};
#endif
      return this_t(grid_cylindrical_canonical_point(i0[i]),
                    grid_cylindrical_canonical_point(i1[i]),
                    grid_cylindrical_canonical_point(i2[i]),
                    grid_cylindrical_canonical_point(i3[i]));
    }
  
    inline this_t quad_child_diamond(int r, int c) const {
      // i = row 0 bottom -> top, j = column left ->right
      if (r == 0) {
         if (c == 0) {
          return this_t(id(),
                        midpoint(corners_[3], corners_[0]),
                        corners_[0],
                        midpoint(corners_[0], corners_[1]));                        
        } else {
          return this_t(id(),
                        midpoint(corners_[2], corners_[3]),
                        corners_[3],
                        midpoint(corners_[3], corners_[0]));
        }        
      } else {
       if (c == 0) {
          return this_t(id(),
                        midpoint(corners_[0], corners_[1]),
                        corners_[1],
                        midpoint(corners_[1], corners_[2]));
                        
        } else {
          return this_t(id(),
                        midpoint(corners_[1], corners_[2]),
                        corners_[2],
                        midpoint(corners_[2], corners_[3]));
        }
      }
    }

    inline grid_point_t quad_parent_id() const {
      return corners_[0];
    }

  public: // sorting

    inline bool operator < (const this_t& other) const {
      grid_point_morton_cmp is_less;
      return is_less(id(), other.id());
    }

    inline bool operator == (const this_t& other) const {
      return id() == other.id();
    }
    
    inline bool operator != (const this_t& other) const {
      return id() != other.id();
    }

    inline bool operator <= (const this_t& other) const {
      grid_point_t x_id = id();
      grid_point_t y_id = other.id();
      grid_point_morton_cmp is_less;
      return (x_id == y_id) || (is_less(x_id, y_id));
    }

    inline bool operator >= (const this_t& other) const {
      grid_point_t x_id = id();
      grid_point_t y_id = other.id();
      grid_point_morton_cmp is_less;
      return !is_less(x_id, y_id);
    }
    
    inline bool operator > (const this_t& other) const {
      grid_point_t x_id = id();
      grid_point_t y_id = other.id();
      grid_point_morton_cmp is_less;
      return (x_id != y_id) && (!is_less(x_id, y_id));
    }
    
  };


  inline std::ostream& operator<<(std::ostream& os, const grid_diamond& gd) {
    os << "id= " << gd.id()[0] << " " << gd.id()[1] << " " << gd.id()[2] << std::endl;
    for (std::size_t i=0; i<4; ++i) {
      os << "  corner[" << i << "] = " << gd.corner(i)[0] << " " << gd.corner(i)[1] << " " << gd.corner(i)[2] << std::endl;
    }
    return os;
  }

}

#endif
