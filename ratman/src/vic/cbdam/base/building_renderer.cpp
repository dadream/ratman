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
#include <vic/cbdam/base/building_renderer.hpp>
#include <GL/glew.h>

namespace cbdam {

  std::ostream& operator<<(std::ostream& os, const building_hierarchy_node& n) {
    os << "box " << n.bbox()[0][0] << ", " << n.bbox()[0][1] << ", " << n.bbox()[0][2] << ", " 
       << n.bbox()[1][0] << ", " << n.bbox()[1][1] << ", " << n.bbox()[1][2] << ", "
       << "parent_id " << n.parent_id() << ", index " << n.index() << ", object_count " << n.object_count()
       << ", first_gl_vertex " << n.first_opengl_vertex() << ", vertex_count " << n.vertex_count()
       << ", triangle_count " << n.triangle_count() << std::endl;
    return os;
  }

  building_renderer::building_renderer() {
    scene_pointer_ = 0;
    visibility_threshold_ = 4;
    occlusion_culling_enabled_ = false;
    draw_bounding_box_enabled_ = false;
    light_direction_ = vector4_t(0.25f, 0.25f, 1.0f, 0.0f).ok_normalized();

    stat_rendered_objects_ = 0;
    stat_issued_occlusion_queries_ = 0;
    stat_rendered_triangles_ = 0;
    screen_tolerance_ = 0.005f;
 }

  building_renderer::~building_renderer() {
    scene_pointer_ = 0;
  }

  void building_renderer::set_scene_pointer(const building_hierarchy* bh) {
    scene_pointer_ = bh;
    if (scene_pointer_) {
      render_init();
      std::cerr << "initialized\n";
    }
  }

  const building_hierarchy* building_renderer::scene_pointer() const {
    return scene_pointer_;
  }
 
  void building_renderer::render_init() {
    frame_counter_ = 0;
    occlusion_info_.clear();
    occlusion_info_.resize(scene_pointer()->size());
  }

  void building_renderer::render(const double* Pptr,
                                 const double* Vptr) {
    matrix44d_t P = (*reinterpret_cast<const matrix44d_t*>(Pptr));
    matrix44d_t V = (*reinterpret_cast<const matrix44d_t*>(Vptr));
    current_PV_ = P*V;
    matrix44d_t inv_V = ~V;
    vector4d_t eye_h  = inv_V * vector4d_t(0.0, 0.0, 0.0, 1.0);
    current_eye_ = point3_t(eye_h[0]/eye_h[3], eye_h[1]/eye_h[3], eye_h[2]/eye_h[3]);
    sl::projective_map3d p = P;
    current_tan_half_fovy_ = 0.5f * std::tan(p.fov_from_std_3d_perspective());

    stat_rendered_objects_ = 0;
    stat_issued_occlusion_queries_ = 0;
    stat_rendered_triangles_ = 0;
    
    glPushAttrib(GL_ALL_ATTRIB_BITS);

    glEnable(GL_DEPTH_TEST);
#if 0
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
#else
    glDisable(GL_CULL_FACE);
#endif
    glShadeModel(GL_FLAT);
    glDisable(GL_TEXTURE_2D);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    // disable all not used vertex arrays
    glDisableClientState(GL_EDGE_FLAG_ARRAY);
    glDisableClientState(GL_INDEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);

    // enable required vertex arrays
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY);
    glEnable(GL_COLOR_MATERIAL);

    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadMatrixd(P.to_pointer());
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    
    // light fixed to the viewer
    glLoadIdentity();
    glLightfv(GL_LIGHT0, GL_POSITION, light_direction_.to_pointer());

    glLoadMatrixd(V.to_pointer());
   
    breadth_first_bsp_render();

    glDisableClientState(GL_COLOR_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
  }
  
  void building_renderer::breadth_first_bsp_render() {
#if 0
    uint32_t size = scene_pointer()->size();
    uint32_t leafs = 0;
    for(uint32_t i = 0; i < size; ++i) {
      if (scene_pointer()->is_leaf(i)) {
        ++leafs;
	bsp_render_data(i);
      }
    }
    std::cerr << "Rendered " << leafs << "/" << size << " nodes." << std::endl;
    return;
#endif

    std::priority_queue<std::pair<float, node_id_t> >   traversal_queue;
    std::queue<node_id_t>                               query_queue;

    ++frame_counter_;
    traversal_queue.push(std::make_pair(-min_distance_to_eye(0), 0));
    // Traversal
    while (!traversal_queue.empty() ||
           !query_queue.empty()) {

      // Check available occlusion queries
      while (!query_queue.empty() && 
	     (traversal_queue.empty() || is_occlusion_query_result_available(query_queue.front()))) {
      
        node_id_t node_idx = query_queue.front();
        query_queue.pop();

        uint32_t pixel_count = occlusion_query_result(node_idx);
	if (pixel_count > visibility_threshold()) {
	  bsp_pull_up_visibility(node_idx);
	  breadth_first_bsp_render_or_refine(node_idx,
                                             traversal_queue,
                                             query_queue,
                                             false);
	}
      }

      // traverse other nodes at this level
      if (!traversal_queue.empty()) {
        float d_eye_box;
        node_id_t node_idx;
        sl::tie(d_eye_box, node_idx) = traversal_queue.top();
        d_eye_box = -d_eye_box;
        traversal_queue.pop();
        
        node_occlusion_info& oi = occlusion_info_[node_idx];
        bool was_visible = (oi.visible() &&
                            oi.last_visited() == frame_counter_-1);
        oi.last_visited() = frame_counter_;
        oi.visible()      = false;
        oi.rendered()     = false;
        oi.queried()      = false;
        
	if (is_view_frustum_visible(node_idx)) {
          breadth_first_bsp_render_or_refine(node_idx,
                                             traversal_queue,
                                             query_queue,
                                             was_visible);
        } 
      }
    }
  }
    
  void building_renderer::breadth_first_bsp_render_or_refine(node_id_t node_idx,
                                                             std::priority_queue<std::pair<float, node_id_t> >& traversal_queue,
                                                             std::queue<node_id_t>& query_queue,
                                                             bool node_was_visible) {
    node_occlusion_info& oi = occlusion_info_[node_idx];
    if (oi.rendered()) {
      // Already rendered, this was a leaf rendered without waiting for
      // occlusion query results
    } else {
      const building_hierarchy_node& node = scene_pointer()->node(node_idx);
      const aabox_t& box = node.bbox();
      float min_dist_to_eye = box.distance_to(current_eye_);
      float length = box.diagonal().two_norm(); // box[1][2] - box[0][2];
      if ((length < 2.0f * min_dist_to_eye * current_tan_half_fovy_ * screen_tolerance_)) {
        // node project to less then visibility_threshold_: skip subtree
#if 0
        // splat node
        glBegin(GL_POINTS);
        glVertex3fv(box.center().to_pointer());
        glEnd();
#endif
        return;
      } else {
        // node projects to > visibility_threshold_
        if (scene_pointer()->is_leaf(node_idx)) {
          // leaf
          render_data_and_issue_occlusion_query(node_idx,
                                                query_queue,
                                                node_was_visible);
        } else {
          // inner node
	  bool is_inside_box = min_dist_to_eye < 1e-6f; 
          if (is_occlusion_culling_enabled() && !oi.queried() && !node_was_visible && !is_inside_box) {
            issue_occlusion_query(node_idx, query_queue);
            oi.queried() = true;          
          } else {
            traversal_queue.push(std::make_pair(-min_distance_to_eye(node.child(0)), node.child(0)));
            traversal_queue.push(std::make_pair(-min_distance_to_eye(node.child(1)), node.child(1)));
          }
        }
      }
    }
  }
  
  void building_renderer::bsp_render_data(node_id_t node_idx) {
    const building_hierarchy_node& node = scene_pointer()->node(node_idx);
    ++stat_rendered_objects_;
    stat_rendered_triangles_ += node.triangle_count();
    
    uint32_t first_gl_vertex = node.first_opengl_vertex();
    uint32_t vertex_count = node.vertex_count();
    const point3_t* v = scene_pointer()->opengl_vertices(first_gl_vertex, vertex_count);
    const building_hierarchy::normal_i16_t* n = scene_pointer()->opengl_normals(first_gl_vertex, vertex_count);
    const color_rgba_t* c = scene_pointer()->opengl_colors(first_gl_vertex, vertex_count);
    glVertexPointer(3, GL_FLOAT, 0, v);
    glNormalPointer(GL_SHORT, 0, n); 
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, c);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
  }
  
  void building_renderer::bsp_pull_up_visibility(node_id_t n_idx) {
    node_id_t node_idx = n_idx;
    while (!occlusion_info_[node_idx].visible()) {
      if (occlusion_info_[node_idx].last_visited() != frame_counter_) {
	SL_TRACE_OUT(-1) << "Pull up on unvisited node!" << std::endl;
      }
      occlusion_info_[node_idx].visible() = true;
      if (node_idx != 0) {
        node_idx = scene_pointer()->node(node_idx).parent_id();
      }
    }
  }
  
  void building_renderer::draw_bounding_box(node_id_t node_idx) {
    const aabox_t& node_box = scene_pointer()->node(node_idx).bbox();
    const point3_t& p000 = node_box[0];
    const point3_t& p111 = node_box[1];

    glBegin(GL_QUADS);
    glColor3f(0,0,0.9);
    glNormal3f( 0,0,-1 );
    glVertex3f(p111[0], p000[1], p000[2]);
    glVertex3f(p000[0], p000[1], p000[2]);
    glVertex3f(p000[0], p111[1], p000[2]);
    glVertex3f(p111[0], p111[1], p000[2]);
   
    glColor3f(0.3,1,0.3);
    glNormal3f( 0,1,0 );
    glVertex3f(p111[0], p111[1], p000[2]);
    glVertex3f(p000[0], p111[1], p000[2]);
    glVertex3f(p000[0], p111[1], p111[2]);
    glVertex3f(p111[0], p111[1], p111[2]);
 
    glColor3f(0.3,0.3,1);
    glNormal3f( 0,0,1 );
    glVertex3f(p111[0], p111[1], p111[2]);
    glVertex3f(p000[0], p111[1], p111[2]);
    glVertex3f(p000[0], p000[1], p111[2]);
    glVertex3f(p111[0], p000[1], p111[2]);
 
    glColor3f(0,0.9,0);
    glNormal3f( 0,-1,0 );
    glVertex3f(p111[0], p000[1], p111[2]);
    glVertex3f(p000[0], p000[1], p111[2]);
    glVertex3f(p000[0], p000[1], p000[2]);
    glVertex3f(p111[0], p000[1], p000[2]);

    glColor3f(0.9, 0, 0);
    glNormal3f( -1,0,0 );
    glVertex3f(p000[0], p000[1], p000[2]);
    glVertex3f(p000[0], p000[1], p111[2]);
    glVertex3f(p000[0], p111[1], p111[2]);
    glVertex3f(p000[0], p111[1], p000[2]);

    glColor3f(1, 0.3, 0.3);
    glNormal3f( 1,0,0 );
    glVertex3f(p111[0], p000[1], p000[2]);
    glVertex3f(p111[0], p111[1], p000[2]);
    glVertex3f(p111[0], p111[1], p111[2]);
    glVertex3f(p111[0], p000[1], p111[2]);
    glEnd();
  }
  
  void building_renderer::render_data_and_issue_occlusion_query(node_id_t node_idx,
                                                                std::queue<node_id_t>& query_queue,
                                                                bool node_was_visible) {
    const float occlusion_interval = 5.0f;
    node_occlusion_info& oi = occlusion_info_[node_idx];
    if (is_occlusion_culling_enabled() && !oi.queried()) {
      if (node_was_visible && random_pick(1.0f-1.0f/occlusion_interval)) {
        // Skip checking previously visible node, assume visible
        bsp_pull_up_visibility(node_idx);
        bsp_render_data(node_idx);
        oi.queried() = true;
        oi.rendered() = true;
      } else {
        glBeginQueryARB(GL_SAMPLES_PASSED_ARB, node_idx+1);
        bsp_render_data(node_idx);
        glEndQueryARB(GL_SAMPLES_PASSED_ARB);

        ++stat_issued_occlusion_queries_;
        query_queue.push(node_idx);
        oi.queried() = true;
        oi.rendered() = true;
      }
    } else {
      bsp_render_data(node_idx);
      oi.rendered() = true;
    }
  }
  
  void building_renderer::issue_occlusion_query(node_id_t node_idx,
                                                std::queue<node_id_t>& query_queue) {
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glBeginQueryARB(GL_SAMPLES_PASSED_ARB, node_idx+1);
    draw_bounding_box(node_idx);
    glEndQueryARB(GL_SAMPLES_PASSED_ARB);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    
    ++stat_issued_occlusion_queries_;
    query_queue.push(node_idx);
  }

  bool building_renderer::is_view_frustum_visible(node_id_t node_idx) const {
    aabox_t node_box = scene_pointer()->node(node_idx).bbox();
    bool visible = true;
    for (std::size_t i=0; i<6 && visible; ++i) {
      const double* p_pl = current_PV_.clip_plane(i).to_pointer();
      aabox_t::plane_t::hdual_vector_t hdv(p_pl[0], p_pl[1], p_pl[2], p_pl[3]);
      aabox_t::plane_t pl(hdv);
      if (node_box.is_fully_below(pl)) visible = false;
    }    
    return visible;
  }

  bool building_renderer::is_occlusion_query_result_available(node_id_t node_idx) const {
    GLuint available;
    glGetQueryObjectuivARB(node_idx+1, GL_QUERY_RESULT_AVAILABLE_ARB, &available);   
    return (bool)available;
  }

  uint32_t building_renderer::occlusion_query_result(node_id_t node_idx) const {
    GLuint pixel_count = 0;
    glGetQueryObjectuivARB(node_idx+1, GL_QUERY_RESULT_ARB, &pixel_count);
    return pixel_count;
  }

  void building_renderer::release_graphic_resources() {
    // we do not use any cache by now: simply clear stats
    stat_issued_occlusion_queries_ = 0;
    stat_rendered_triangles_ = 0;
    stat_rendered_objects_ = 0;
  }

  void building_renderer::set_occlusion_culling_enabled(bool x) {
    occlusion_culling_enabled_ = x;

    reset_occlusion_info();
  }

  void building_renderer::reset_occlusion_info() {
    for(std::vector<node_occlusion_info>::iterator it = occlusion_info_.begin();
	it != occlusion_info_.end();
	++it) {
      node_occlusion_info& oi = *it;
      oi.last_visited() = frame_counter_;
      oi.visible()      = true;
      oi.rendered()     = false;
      oi.queried()      = false;
    }
  }

  bool building_renderer::is_occlusion_culling_enabled() const {
    return occlusion_culling_enabled_;
  }
}
  
