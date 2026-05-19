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
#ifndef CBDAM_BUILDING_RENDERER_HPP
#define CBDAM_BUILDING_RENDERER_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/building_hierarchy.hpp>
#include <sl/projective_map.hpp>
#include <sl/affine_map.hpp>
#include <sl/linear_map_factory.hpp>
#include <sl/random.hpp>
#include <stack>
#include <queue>
#include <set>

namespace cbdam {
  
  /// per node information to perform occlusion culling
  class node_occlusion_info {
  public:
    node_occlusion_info() :
      last_visited_(uint16_t(-1)), 
      visible_(false),
      rendered_(false),
      queried_(false) {
    }

    CBDAM_RW_ACCESSOR(uint16_t, last_visited);
    CBDAM_RW_ACCESSOR(bool,     visible);
    CBDAM_RW_ACCESSOR(bool,     rendered);
    CBDAM_RW_ACCESSOR(bool,     queried);

  protected:
    uint16_t last_visited_;
    bool visible_;
    bool rendered_;
    bool queried_;
  };

  /**
   *
   */
  class building_renderer {
  public:
    typedef uint32_t                      node_id_t;
    
  protected:
    const building_hierarchy*		scene_pointer_;
    std::vector<node_occlusion_info>    occlusion_info_;
    sl::projective_map3d                current_PV_;
    point3_t                            current_eye_;
    vector4_t			        light_direction_;
    uint16_t             frame_counter_;
    uint32_t             visibility_threshold_; // in pixels
    bool                 occlusion_culling_enabled_;
    bool                 draw_bounding_box_enabled_;
    float                current_tan_half_fovy_;
    float                screen_tolerance_;

    uint32_t             stat_rendered_objects_;
    uint32_t             stat_rendered_triangles_;
    uint32_t             stat_issued_occlusion_queries_;
    
  public:
    building_renderer();

    ~building_renderer();

    void set_scene_pointer(const building_hierarchy* bh);

    const building_hierarchy* scene_pointer() const;
    
    CBDAM_RW_ACCESSOR(uint32_t, visibility_threshold);
    CBDAM_RW_ACCESSOR(bool, draw_bounding_box_enabled);
    CBDAM_R_ACCESSOR(uint32_t, stat_rendered_objects);
    CBDAM_R_ACCESSOR(uint32_t, stat_rendered_triangles);
    CBDAM_R_ACCESSOR(uint32_t, stat_issued_occlusion_queries);
    CBDAM_W_ACCESSOR(vector4_t, light_direction);
    CBDAM_RW_ACCESSOR(float, screen_tolerance);

    void set_occlusion_culling_enabled(bool x);

    bool is_occlusion_culling_enabled() const;

    void render(const double* P,
		const double* V);

    void release_graphic_resources();
    
  protected:
    void render_init();
  
    void breadth_first_bsp_render();
    
    void breadth_first_bsp_render_or_refine(node_id_t node_idx,
                                            std::priority_queue<std::pair<float, node_id_t> >& traversal_queue,
                                            std::queue<node_id_t>& query_queue,
                                            bool node_was_visible = false);
    void bsp_render_data(node_id_t node_idx);
    void bsp_pull_up_visibility(node_id_t node_idx);

    void draw_bounding_box(node_id_t node_idx);
    void render_data_and_issue_occlusion_query(node_id_t node_idx,
                                               std::queue<node_id_t>& query_queue,
                                               bool node_was_visible);
    void issue_occlusion_query(node_id_t node_idx,
                               std::queue<node_id_t>& query_queue);

    bool is_occlusion_query_result_available(node_id_t node_idx) const;

    uint32_t occlusion_query_result(node_id_t node_idx) const;

    bool is_view_frustum_visible(node_id_t node_idx) const;

    float min_distance_to_eye(node_id_t node_idx) const;
    
    void reset_occlusion_info();

  protected:
    mutable sl::random::uniform_closed<double>  rng_;

    inline bool random_pick(double p) const {
      return rng_.value() < p;
    }
  };


} // namespace cbdam 

#endif // CBDAM_BUILDING_RENDERER_HPP

#ifndef CBDAM_BUILDING_RENDERER_IPP
#define CBDAM_BUILDING_RENDERER_IPP

namespace cbdam {
  
  inline float building_renderer::min_distance_to_eye(node_id_t node_idx) const {
    const building_hierarchy_node& node = scene_pointer()->node(node_idx);
    const aabox_t& box = node.bbox();
    return box.distance_to(current_eye_);
  }

} // namespace cbdam 

#endif // CBDAM_BUILDING_RENDERER_IPP
