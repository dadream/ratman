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
#ifndef CBDAM_GRID_DIAMOND_STATE_HPP
#define CBDAM_GRID_DIAMOND_STATE_HPP

#include <vic/cbdam/base/grid_diamond.hpp>
#include <vic/cbdam/base/diamond_vertices.hpp>
#include <sl/serializer.hpp>
#include <sl/oriented_box.hpp>

namespace cbdam {

  /// State of a diamond in a graph
  class grid_diamond_state {
  protected:
    uint8_t flags_;
  protected:

    inline bool is_bit_set(int bit) const {
      const uint8_t mask = uint8_t(1<<bit);
      return (flags_ & mask) ? true : false;
    }
    
    inline void set_bit(int bit, bool x) {
      const uint8_t mask = uint8_t(1<<bit);
      flags_ &= (mask^0xff);
      assert(is_bit_set(bit) == false);
      flags_ |= (x?mask:0x00);
      assert(is_bit_set(bit) == x);
    }
  public: // Serialization
    
    void store_to(sl::output_serializer& s) const {
      s.write_simple(flags_);
    }

    void retrieve_from(sl::input_serializer& s) {
      s.read_simple(flags_);
    }

  public:

    inline grid_diamond_state() : flags_(0) {
    }

    inline virtual ~grid_diamond_state() {
    }

    inline grid_diamond_state(bool x_is_leaf, bool x_has_fragment0, bool x_has_fragment1) : flags_(0) {
      set_is_leaf(x_is_leaf);
      set_has_fragment(0, x_has_fragment0);
      set_has_fragment(1, x_has_fragment1);
      assert(is_leaf() == x_is_leaf);
      assert(has_fragment(0) == x_has_fragment0);
      assert(has_fragment(1) == x_has_fragment1);
    }
    
    inline bool is_leaf() const {
      return is_bit_set(0);
    }
    inline void set_is_leaf(bool x) {
      set_bit(0,x);
      assert(is_leaf() == x);
    }

    inline bool has_fragment(int i) const {
      assert(i>=0 &&i<2);
      return is_bit_set(1+i);
    }
    
    inline void set_has_fragment(int i, bool x) {
      assert(i>=0 &&i<2);
      set_bit(1+i,x);
      assert(has_fragment(i) == x);
    }
    
    inline bool operator<(const grid_diamond_state& other) const {
      return flags_ < other.flags_;
    }
  };

  /// State of a diamond in a render graph
  class grid_diamond_render_state : public grid_diamond_state {
  public:
    typedef sl::oriented_box<3, double>         bounding_volume_t;

  protected:
    bounding_volume_t		bounding_volume_;
    double			triangle_edge_length_;
    bool			is_procedural_;
    const diamond_vertices*	reference_counted_patch_data_[2];

  public:
    inline grid_diamond_render_state() : grid_diamond_state() {
      is_procedural_ = false;
      reference_counted_patch_data_[0] = 0;
      reference_counted_patch_data_[1] = 0;
    }

    inline virtual ~grid_diamond_render_state() {
      if (reference_counted_patch_data_[0]) {
	reference_counted_patch_data_[0]->deref();
	//	std::cerr << "dereffed reference_counted_patch_data_0, use count = " << reference_counted_patch_data_[0]->use_count() << std::endl;
      }
      if (reference_counted_patch_data_[1]) {
	reference_counted_patch_data_[1]->deref();
	//	std::cerr << "dereffed reference_counted_patch_data_1, use count = " << reference_counted_patch_data_[1]->use_count() << std::endl;
      }
    }

    inline grid_diamond_render_state(const grid_diamond_render_state& x) :
      grid_diamond_state(x),
      bounding_volume_(x.bounding_volume()), triangle_edge_length_(x.triangle_edge_length()), is_procedural_(x.is_procedural()) {
      for(std::size_t i = 0; i < 2; ++i) {
	reference_counted_patch_data_[i] = x.reference_counted_patch_data(i);
	if (reference_counted_patch_data_[i]) reference_counted_patch_data_[i]->ref();
      }
    }

    inline const grid_diamond_render_state& operator=(const grid_diamond_render_state& x) {
      (grid_diamond_state&)(*this) = x;
      bounding_volume_ = x.bounding_volume();
      triangle_edge_length_ = x.triangle_edge_length();
      is_procedural_ = x.is_procedural();
      for(std::size_t i = 0; i < 2; ++i) {
	reference_counted_patch_data_[i] = x.reference_counted_patch_data(i);
	if (reference_counted_patch_data_[i]) reference_counted_patch_data_[i]->ref();
      }

      return *this;
    }

    inline grid_diamond_render_state(bool x_is_leaf, bool x_has_fragment0, bool x_has_fragment1,
                                     double triangle_edge_length, bool is_procedural,
				     const diamond_vertices* v0 = 0, const diamond_vertices* v1 = 0) : 
      grid_diamond_state(x_is_leaf, x_has_fragment0, x_has_fragment1),
      triangle_edge_length_(triangle_edge_length),
      is_procedural_(is_procedural) {
      reference_counted_patch_data_[0] = v0;
      reference_counted_patch_data_[1] = v1;
      if (reference_counted_patch_data_[0]) reference_counted_patch_data_[0]->ref();
      if (reference_counted_patch_data_[1]) reference_counted_patch_data_[1]->ref();      
    }

    void set_reference_counted_patch_data(int patch_id, const diamond_vertices* dv) {
      if (reference_counted_patch_data_[patch_id]) reference_counted_patch_data_[patch_id]->deref();
      reference_counted_patch_data_[patch_id] = dv;
      reference_counted_patch_data_[patch_id]->ref();
    }

    const diamond_vertices* reference_counted_patch_data(int patch_id) const {
      assert(0 <= patch_id && patch_id <2);
      return reference_counted_patch_data_[patch_id];
    }

    const bounding_volume_t& bounding_volume() const {
      return bounding_volume_;
    }

    void set_bounding_volume(const bounding_volume_t& bs) {
      bounding_volume_ = bs;
    }

    double triangle_edge_length() const {
      return triangle_edge_length_;
    }

    void set_triangle_edge_length(double el) {
      triangle_edge_length_ = el;
    }

    void set_procedural(bool x) {
      is_procedural_ = x;
    }

    bool is_procedural() const {
      return is_procedural_;
    }
  };

}

#endif // CBDAM_GRID_DIAMOND_STATE_HPP
