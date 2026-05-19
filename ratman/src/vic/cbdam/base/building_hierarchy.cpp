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
#include <vic/cbdam/base/building_hierarchy.hpp>
#include <sl/triangle_mesh_stripifier.hpp>
#include <queue>
#include <vic/cbdam/base/triangulate.hpp> // FIXME

namespace cbdam {

  void print_box(const aabox_t& x) {
    std::cerr << "min: " << x[0][0] << " " <<  x[0][1] << " " <<  x[0][2] << "\t"
              << "max: " << x[1][0] << " " <<  x[1][1] << " " <<  x[1][2] << "\t"
              << "diag: " << x.diagonal()[0] << " " <<  x.diagonal()[1] << " " <<  x.diagonal()[2] << std::endl;
  }
  
  building_hierarchy::building_hierarchy() {
    input_nodes_ = 0;
    building_hierarchy_ = 0;
    building_objects_ = 0;
    building_vertices_ = 0;
    opengl_vertices_ = 0;
    opengl_normals_ = 0;
    opengl_colors_ = 0;
    
    max_node_vertex_count_ = 3000;
    u_extent_ = 0.0;
    v_extent_ = 0.0;
    
    last_operation_succes_ = true;
  }

  building_hierarchy::~building_hierarchy() {
    close();
  }

  void building_hierarchy::close() {
    if (input_nodes_) {
      delete input_nodes_;
      input_nodes_ = 0;
    }
    if (building_hierarchy_) {
      delete building_hierarchy_;
      building_hierarchy_ = 0;
    }

    if (building_objects_) {
      delete building_objects_;
      building_objects_ = 0;
    }
    if (building_vertices_) {
      delete building_vertices_;
      building_vertices_ = 0;
    }
    if (opengl_vertices_) {
      delete opengl_vertices_;
      opengl_vertices_ = 0;
    }
    if (opengl_normals_) {
      delete opengl_normals_;        
      opengl_normals_ = 0;
    }
    if (opengl_colors_) {
      delete opengl_colors_;        
      opengl_colors_ = 0;
    }
  }
  
  void building_hierarchy::set_extent(double u_extent,
				      double v_extent) {
    u_extent_ = u_extent;
    v_extent_ = v_extent;
  }

  void building_hierarchy::scene_begin(const char* file_name) {
    base_name_ = (std::string)file_name;
    std::string input_nodes_name = base_name_ + ".acc";
    std::string hierarchy_name    = base_name_ + ".hrc";
    std::string objects_name      = base_name_ + ".obs";
    std::string vertices_name     = base_name_ + ".vts";
    std::string opengl_vert_name  = base_name_ + ".ovs";
    std::string opengl_norm_name  = base_name_ + ".ons";
    std::string opengl_color_name = base_name_ + ".ocs";

    input_nodes_         = new input_node_xarray_t(input_nodes_name, "t", 16*1024*1024);
    building_hierarchy_  = new building_hierarchy_node_xarray_t(hierarchy_name, "w", 1*1024*1024);
    building_objects_    = new building_object_xarray_t(objects_name, "w", 2*1024*1024);
    building_vertices_   = new building_vertex_xarray_t(vertices_name, "t", 8*1024*1024);
    opengl_vertices_     = new building_opengl_vertex_xarray_t(opengl_vert_name, "w", 64*1024*1024);
    opengl_normals_      = new building_opengl_normal_xarray_t(opengl_norm_name, "w", 32*1024*1024);
    opengl_colors_       = new building_opengl_color_xarray_t(opengl_color_name, "w", 16*1024*1024);

    if (!input_nodes_->is_open() ||
        !building_hierarchy_->is_open() ||
        !building_objects_->is_open() ||
        !building_vertices_->is_open()||
        !opengl_vertices_->is_open()||
        !opengl_normals_->is_open() ||
        !opengl_colors_->is_open()) {
      last_operation_succes_ = false;
      std::cerr << "failed to open scene files:\n\t"
                << input_nodes_name << "\n\t" << hierarchy_name << "\n\t"
                << objects_name << "\n\t" << vertices_name <<  "\n\t"
                << opengl_vert_name <<  "\n\t" << opengl_norm_name << "\n\t" << opengl_color_name << std::endl;
      return;
    }
  }

  void building_hierarchy::building_begin(double umin, double vmin, double zmin,
					  double umax, double vmax, double zmax,
					  uint32_t /*outlines_count*/) {
    const building_vertex_t abs_min = vertex_abs_coords(umin, vmin);
    const building_vertex_t abs_max = vertex_abs_coords(umax, vmax);
    const point3_t v0(abs_min[0], abs_min[1], (float)zmin);
    const point3_t v1(abs_max[0], abs_max[1], (float)zmax);
    const aabox_t box(v0, v1);
    input_nodes_->push_back(input_node_t(box, (uint32_t)building_vertices_->size(), (uint32_t)0));
    current_outline_ = 0;
  }

  void building_hierarchy::building_color(const color_rgba_t& roof_color,
                                          const color_rgba_t& facades_color) {
    input_node_t node = input_nodes_->back();
    node.roof_color() = roof_color;
    node.facades_color() = facades_color;
    input_nodes_->back() = node;
  }

  void building_hierarchy::building_outline_begin(uint32_t vertices_count) {
    // insert only first outline
    if (current_outline_ == 0) {
      input_node_t node = input_nodes_->back();
      node.vertex_count() = vertices_count;
      input_nodes_->back() = node;
    }
    ++current_outline_;
  }

  void building_hierarchy::building_outline_end() {

  }

  void building_hierarchy::building_vertex(double u, double v) {
    building_vertices_->push_back(vertex_abs_coords(u,v));
  }

  void building_hierarchy::building_end() {
    // is there anything to do ?
  }

  void building_hierarchy::scene_end() {
    if (input_nodes_ && input_nodes_->size() > 0) {
      build_bsp();
    }
  }

  void building_hierarchy::build_bsp() {
    // compute input bounding box
    uint32_t array_size = input_nodes_->size();
    aabox_t box;
    box.to_empty();
    uint32_t sum_vertex_count = 0;
    for(uint32_t i = 0; i < array_size; ++i) {
      const input_node_t& node = (*input_nodes_)[i];
      box.merge(node.bbox());
      sum_vertex_count += node.vertex_count();
    }

    std::cerr << "input merged box = ";
    print_box(box);
    std::cerr << "vertex_sum " << sum_vertex_count << std::endl;

    std::cerr << "build bsp from " << input_nodes_->size() << " input nodes...\n";
    // build bsp
    max_depth_ = 0;
    build_bsp(uint32_t(-1), uint32_t(-1), *input_nodes_, box, sum_vertex_count, 0);
    input_nodes_->close();
    std::cerr << "max depth reached " << max_depth_ << "\n";
    std::cerr << "built hierarchy with " << building_hierarchy_->size() << " nodes\n";
  }

  void building_hierarchy::build_bsp(uint32_t parent_id, uint32_t child_id,
                                     input_node_xarray_t& remaining_nodes,
                                     const aabox_t& bbox,
                                     uint32_t sum_vertex_count,
                                     uint32_t depth) {
    //    std::cerr << "build_bsp pid: " << parent_id << ", cid: " << child_id << ", rmns: " << remaining_nodes.size() << ", svc: " << sum_vertex_count << std::endl;
    if (remaining_nodes.size() == 0) {
      // it should'nt happen
      return;
    } else {
      uint32_t this_node_id = building_hierarchy_->size();

      // set entry in the xarray
      building_hierarchy_->push_back(building_hierarchy_node(bbox, parent_id));

      // set link from parent to this child
      if (parent_id != uint32_t(-1)) {
        // this is not the root
        assert(building_hierarchy_->size() > parent_id);

        // periphrasis to set a value on a field of a building_hierarchy_node
        building_hierarchy_node node = (*building_hierarchy_)[parent_id];
        node.child(child_id) = this_node_id;
        (*building_hierarchy_)[parent_id] = node;
      }

      if (sum_vertex_count < max_node_vertex_count_ || remaining_nodes.size() == 1) {
        // leaf
        if (depth > max_depth_) {
          max_depth_ = depth;
          std::cerr << "max depth reached " << max_depth_ << "\r";
        }

        // set object pointer
        building_hierarchy_node node = building_hierarchy_->back();
        node.index() = building_objects_->size();
        node.object_count() = remaining_nodes.size();
        building_hierarchy_->back() = node;
        
        // create objects: for each input node create one object with vertices pointers
        std::vector<color_rgba_t> roof_colors;
        std::vector<color_rgba_t> facades_colors;
        while(!remaining_nodes.empty()) {
          input_node_t in_node = remaining_nodes.back();
          remaining_nodes.pop_back();

          const aabox_t& box = in_node.bbox();
          building_objects_->push_back(building_object(box[0][2],
                                                       box[1][2],
                                                       in_node.first_point_index(),
                                                       in_node.vertex_count()));
          // store colors to be passed to build_opengl_data.
          // Don't store these values in building_object, because they will be inserted in opengl_colors_
          roof_colors.push_back(in_node.roof_color());
          facades_colors.push_back(in_node.facades_color());
        }

        build_opengl_data(this_node_id, roof_colors, facades_colors);
        remaining_nodes.close();
      } else {
        // split array
        aabox_t left_box, right_box;
        input_node_xarray_t left_samples(tmp_xarray_name());
        input_node_xarray_t right_samples(tmp_xarray_name());
        uint32_t split_index;
        uint32_t left_sum_vertex_count;
        split_xarray(remaining_nodes, bbox, left_samples, left_box, right_samples, right_box, split_index, left_sum_vertex_count);
        
        // store split index
        building_hierarchy_node node = building_hierarchy_->back();
        node.index() = split_index;
        building_hierarchy_->back() = node;

        remaining_nodes.close();
        
        build_bsp(this_node_id, 0, left_samples, left_box, left_sum_vertex_count, depth + 1);
        build_bsp(this_node_id, 1, right_samples, right_box, sum_vertex_count - left_sum_vertex_count, depth + 1);
      }
    }
  }

  void building_hierarchy::split_xarray(input_node_xarray_t& samples, const aabox_t& box,
                                        input_node_xarray_t& left_samples, aabox_t& left_box,
                                        input_node_xarray_t& right_samples, aabox_t& right_box,
                                        uint32_t& split_index, uint32_t& left_sum_vertex_count) const {
    // 2D BSP on X and Y
    split_index = box.diagonal()[0] > box.diagonal()[1] ? 0 : 1;
    uint32_t split_trial = 0;
    bool found = false;
    left_sum_vertex_count = 0;
    do {
      float center = box.center()[split_index];
      left_box.to_empty();
      right_box.to_empty();
      while (!samples.empty()) {
        input_node_t x = samples.back();
        samples.pop_back();
        if (x.bbox().center()[split_index] < center) {
          left_samples.push_back(x);
          left_box.merge(x.bbox());
          left_sum_vertex_count += x.vertex_count();
        } else {
          right_samples.push_back(x);
          right_box.merge(x.bbox());
        }
      }

      if (left_samples.size() == 0) {
        samples.swap(right_samples);
        ++split_trial;
        split_index = (split_index ? 0 : 1);
        left_sum_vertex_count = 0;
      } else if (right_samples.size() == 0) {
        samples.swap(left_samples);
        ++split_trial;
        split_index = (split_index ? 0 : 1);
        left_sum_vertex_count = 0;
      } else {
        found = true;
      }
    } while (!found && split_trial < 2);
    
    if (!found ) {
      // a big box probably contains other small boxes,
      // keep it in the right_samples and push other in left_samples
      float max_diag = 0;
      uint32_t max_idx = 0;
      for(uint32_t i = 0; i < samples.size(); ++i) {
        input_node_t in = samples[i];
        if (max_diag < in.bbox().diagonal().two_norm_squared()) {
          max_diag = in.bbox().diagonal().two_norm_squared();
          max_idx = i;
        }
      }

      for(uint32_t i = 0; i < samples.size(); ++i) {
        input_node_t in = samples[i];
        if (i != max_idx) {
          left_samples.push_back(in);
          left_sum_vertex_count += in.vertex_count();
        } else {
          right_samples.push_back(in);
        }
      }
      samples.clear();

      // keep it as an indication that a not valid split has been possible
      split_index = 3;
    }
  }
  
  void triangulate(const sl::point2f* points, 
		   uint32_t point_size, 
		   std::vector<uint32_t>& mesh_indices) {
    std::vector<sl::point2f> mesh_points;
    for (uint32_t i=0; i<point_size; ++i) {
      mesh_points.push_back(points[i]);
    }
    sl::simple_polygon_triangulator<float>::process(mesh_points, mesh_indices);
  }

  void stripify(const std::vector<uint32_t>& mesh_indices, std::vector<uint32_t>& stitched_strip) {
    sl::triangle_mesh_greedy_stripifier tms;
    tms.set_cache_size(16);
    tms.begin_input();
    for(uint32_t i = 0; i < mesh_indices.size(); i+=3) {
      tms.insert_input_triangle(mesh_indices[i], mesh_indices[i+1], mesh_indices[i+2]);
    }
    tms.end_input();

    // stripify and stitch strips
    tms.begin_output();
    while (tms.has_output_strip()) {
      tms.begin_output_strip();
      std::size_t v0 = tms.get_output_strip_vertex();
      if (stitched_strip.size()>0) {
        // close old strip
        stitched_strip.push_back(stitched_strip[stitched_strip.size()-1]);

        stitched_strip.push_back(v0);
        if (stitched_strip.size()%2==1) {
          stitched_strip.push_back(v0);
        }
      }
      assert(stitched_strip.size() % 2 == 0);
      stitched_strip.push_back(v0);

      while(tms.has_output_vertex()) {
        stitched_strip.push_back(tms.get_output_strip_vertex());
      }
      tms.end_output_strip();
    }
    tms.end_output();
  }

  building_hierarchy::normal_i16_t get_normal(const point2_t* vertices, uint32_t i, uint32_t vertex_count) {
    const point2_t& va  = vertices[i];
    uint32_t ip1 = i < vertex_count-1 ? i+1 : 0;
    const point2_t& vb = vertices[ip1];

    float dx = vb[0] - va[0];
    float dy = vb[1] - va[1];
    float norm = sqrt(dx*dx+dy*dy);
    if (norm) {
      dx /= norm;
      dy /= norm;
    }
    return building_hierarchy::normal_i16_t(int16_t(-dy*32767.0f), int16_t(dx*32767.0f), 0);
  }
  
  void building_hierarchy::close_strip(const point3_t& v, const point3_t& old_v,
                                       const color_rgba_t& c, const color_rgba_t& old_c,
                                       uint32_t& strip_size) {    
    // close old strip
    normal_i16_t n(0,0,32767);
    opengl_normals_->push_back(n);
    opengl_colors_->push_back(old_c);
    opengl_vertices_->push_back(old_v);
    opengl_normals_->push_back(n);
    opengl_colors_->push_back(c);
    opengl_vertices_->push_back(v);
    strip_size += 2;
    if (strip_size % 2 == 1) {
      opengl_normals_->push_back(n);
      opengl_colors_->push_back(c);
      opengl_vertices_->push_back(v);
      ++strip_size;
    }
  }

  void building_hierarchy::build_opengl_data(uint32_t node_idx,
                                             const std::vector<color_rgba_t>& roof_colors,
                                             const std::vector<color_rgba_t>& facades_color) {
    //    std::cerr << "node_idx " << node_idx << std::endl;
    building_hierarchy_node node = (*building_hierarchy_)[node_idx];
    uint32_t first_object = node.index();
    uint32_t object_count = node.object_count();
    node.first_opengl_vertex() = opengl_vertices_->size();
    point3_t old_v;
    color_rgba_t old_c;
    normal_i16_t n;
    uint32_t strip_size = 0;
    uint32_t triangle_count = 0;
    for(uint32_t j = 0; j < object_count; ++j) {
      const building_object& obj = object(first_object + j);
      float bottom = obj.bottom();
      float top = obj.top();
      uint32_t vertex_count = obj.vertex_count();
      const sl::point2f* verts = vertices(obj.first_point_index(), vertex_count);

      if (strip_size>0) {
        close_strip(point3_t(verts[0][0], verts[0][1], bottom), old_v,
                    facades_color[j], old_c, strip_size);
      }

      // set walls strip, last vertex is the first one repeated.
      n = get_normal(verts, 0, vertex_count);;
      for(uint32_t i = 0; i < vertex_count; ++i) {
        const point2_t& v  = verts[i];
        opengl_normals_->push_back(n);
        opengl_colors_->push_back(facades_color[j]);
        opengl_vertices_->push_back(point3_t(v[0], v[1], bottom));

        opengl_normals_->push_back(n);
        opengl_colors_->push_back(facades_color[j]);
        opengl_vertices_->push_back(point3_t(v[0], v[1], top));
        n = get_normal(verts, i, vertex_count);
        strip_size += 2;
      }

      old_v = point3_t(verts[vertex_count-1][0], verts[vertex_count-1][1], top);
      old_c = facades_color[j];
      
      // stripify roof
      std::vector<uint32_t> mesh_indices;

      // pass vertex_count-1 because last vertex is the first one repeated.
      triangulate(verts, vertex_count - 1, mesh_indices);
      if (mesh_indices.empty()) {
	SL_TRACE_OUT(1) << "  Triangulation error for contour with " << vertex_count-1 << " vertices." << std::endl;
      } else { 
	std::vector<uint32_t>  strip_indices;
	stripify(mesh_indices, strip_indices);

	// close previous strip
	close_strip(point3_t(verts[strip_indices[0]][0],
			     verts[strip_indices[0]][1],
			     top), old_v,
                    old_c, roof_colors[j], strip_size);

	// set roof vertices
	for(uint32_t i = 0; i < strip_indices.size(); ++i) {
	  const point2_t& v  = verts[strip_indices[i]];
	  opengl_normals_->push_back(normal_i16_t(0, 0, 32767));
	  opengl_colors_->push_back(roof_colors[j]);
	  opengl_vertices_->push_back(point3_t(v[0], v[1], top));
	  ++strip_size;
	}

	// save last v for stitching ths strips
	const point2_t& last_p = verts[strip_indices.back()];
	old_v = point3_t(last_p[0], last_p[1], top);
        old_c = roof_colors[j];
      }
      
      // update triangle count : 2 triangles x edge + roof triangles
      triangle_count += vertex_count * 2 + mesh_indices.size() / 3;
    }
    node.vertex_count() = opengl_vertices_->size() - node.first_opengl_vertex();
    node.triangle_count() = triangle_count;
    (*building_hierarchy_)[node_idx] = node;
   }

  void building_hierarchy::print_bsp() const {
    for(uint32_t i = 0; i < building_hierarchy_->size(); ++i) {
      const building_hierarchy_node& bhn = (*building_hierarchy_)[i];
      if (bhn.child(0) != (uint32_t(-1))) {
        std::cerr << "node: " << i
                  << ", c0: " << bhn.child(0)
                  << ", c1: " << bhn.child(1) << std::endl;
      } else {
        const building_object& bo = (*building_objects_)[bhn.index()];
        std::cerr << "node: " << i
                  << ", 1st-v: " << bo.first_point_index() << ", nv: " << bo.vertex_count() << std::endl;
        //        print_box(bhn.bbox());
      }
    }
  }
  
  void building_hierarchy::open_scene(const char* file_name) {
    base_name_ = (std::string)file_name;
    std::string hierarchy_name    = base_name_ + ".hrc";
    std::string objects_name      = base_name_ + ".obs";
    //    std::string vertices_name     = base_name_ + ".vts";
    std::string opengl_vert_name  = base_name_ + ".ovs";
    std::string opengl_norm_name  = base_name_ + ".ons";
    std::string opengl_color_name = base_name_ + ".ocs";

    building_hierarchy_  = new building_hierarchy_node_xarray_t(hierarchy_name, "r", 1*1024*1024);
    building_objects_    = new building_object_xarray_t(objects_name, "r", 2*1024*1024);
    //    building_vertices_   = new building_vertex_xarray_t(vertices_name, "r", 8*1024*1024);
    opengl_vertices_     = new building_opengl_vertex_xarray_t(opengl_vert_name, "r", 64*1024*1024);
    opengl_normals_      = new building_opengl_normal_xarray_t(opengl_norm_name, "r", 32*1024*1024);
    opengl_colors_       = new building_opengl_color_xarray_t(opengl_color_name, "r", 16*1024*1024);

    if (!building_hierarchy_->is_open() ||
        !building_objects_->is_open() ||
	//        !building_vertices_->is_open() ||
        !opengl_vertices_->is_open() ||
        !opengl_normals_->is_open() ||
        !opengl_colors_->is_open()) {
      last_operation_succes_ = false;
      return;
    } else {
      std::cerr << "buildings scene: nodes " << building_hierarchy_->size() << ", objects "
                << building_objects_->size() << std::endl;
    }

  }

  bool building_hierarchy::last_operation_success() const {
    return last_operation_succes_;
  }
  
  std::string building_hierarchy::tmp_xarray_name() const {
    static std::size_t id = 0;
    ++id;

    std::string name = base_name_ + "_tmp" + sl::to_string(id) + ".xarray";
    return name;
  }

  bool building_hierarchy::height_from_uv(double u, double v, double& top, double& bottom) const {
    // FIXME CHECK!!!!
    top = 0.0;
    bottom = 0.0;

#if 0
    const building_hierarchy_node& n = node(0);
    const aabox_t& box = n.bbox();
    float target_x = u * (box[1][0] - box[0][0]);
    float target_y = v * (box[1][1] - box[0][1]);
    return height_from_xy(target_x, target_y, top, bottom);
#else
    building_vertex_t p = vertex_abs_coords(u, v);
    return height_from_xy(p[0], p[1], top, bottom);
#endif
  }

  bool building_hierarchy::is_point_inside_current_input_building(double u, double v) const {
    input_node_t in = input_nodes_->back();
    building_vertex_t p = vertex_abs_coords(u, v);
    return is_point_inside_polygon(in.first_point_index(), in.vertex_count(), p[0], p[1]);
  }

  bool building_hierarchy::height_from_xy(double x, double y, double& top, double& bottom) const {
    std::stack<uint32_t> node_stack;
    node_stack.push(0);
    bool found = false;
    top = 0.0;
    bottom = 0.0;

    while(!node_stack.empty() && !found) {
      uint32_t node_idx = node_stack.top();
      node_stack.pop();

      const building_hierarchy_node& n = node(node_idx);
      const aabox_t& box = n.bbox();
      const point3_t& p0 = n.bbox()[0];
      const point3_t& p1 = n.bbox()[1];

      // check point to be inside box         
      if (p0[0] <= x && x <= p1[0] &&
          p0[1] <= y && y <= p1[1]) {
        if (!is_leaf(node_idx)) {
          uint32_t first_idx = 1;
          uint32_t second_idx = 0;
          uint32_t split_index = n.index();  // for inner nodes index represent the split_index
          // split_index = 0,1 = x,y.
          // split_index = 2 means box1 contains box0
          if (split_index < 2) {
            double coord = split_index == 0 ? x : y;
            if (coord < box.center()[split_index]) {
              first_idx = 0;
              second_idx = 1;
            } // else { first_idx = 1; second_idx = 0;} } else {first_idx = 1; second_idx = 0;}
          }
          node_stack.push(n.child(second_idx));
          node_stack.push(n.child(first_idx));
        } else {
          found = is_point_inside_buildings(node_idx, x,y, top, bottom);
        }
      }
    }

    return found;
  }

  bool building_hierarchy::is_point_inside_buildings(uint32_t node_idx, double x, double y, double& top, double& bottom) const {
    const building_hierarchy_node& n = node(node_idx);
    const point3_t& p0 = n.bbox()[0];
    const point3_t& p1 = n.bbox()[1];
    if (p0[0] <= x && x <= p1[0] &&
        p0[1] <= y && y <= p1[1]) {
      // inside bbox
      // iterate over node objects
      uint32_t first_object = n.index();
      uint32_t object_count = n.object_count();
      bool found = false;
      for(uint32_t j = 0; j < object_count && !found; ++j) {
	const building_object& obj = object(first_object+j);
        if (is_point_inside_polygon(obj.first_point_index(), obj.vertex_count(), x, y)) {
          found = true;
	  top = obj.top();
	  bottom = obj.bottom();
        }
      }
      return found;
    } else {
      return false;
    }
  }

  bool building_hierarchy::is_point_inside_polygon(uint32_t first_vertex, uint32_t vertex_count, double x, double y) const {
    /////////
    // from Real-Time-Rendering 1, PointInPolygon 2D algor. page 309.
    /////////
    
    bool inside = false;
    const sl::point2f* v = vertices(first_vertex, vertex_count);
    sl::point2f e0 = v[vertex_count-1];
    sl::point2f e1 = v[0];
    bool y0_bigger_than_y = e0[1] >= y;
    for(uint32_t i = 1; i <= vertex_count; ++i) {
      bool y1_bigger_than_y = e1[1] >= y;
      if (y0_bigger_than_y != y1_bigger_than_y) {
        // if (tan(e1-p)>=tan(e0-e1)) == y1_bigger_than_y
        if (((e1[1] - y)*(e0[0] - e1[0]) >= (e1[0] - x)*(e0[1] - e1[1])) == y1_bigger_than_y) {
          inside = !inside;
        }
      }
      if (i < vertex_count) {
        y0_bigger_than_y = y1_bigger_than_y;
        e0 = e1;
        e1 = v[i];
      }
    }

#if 0
    // DEBUG check if point is at least inside the bbox
    if (!inside) {
      double x_min = 10000;
      double x_max = -10000;
      double y_min = 10000;
      double y_max = -10000;
      for(uint32_t i = 0; i < vertex_count; ++i) {
	if (v[i][0] < x_min) x_min = v[i][0];
	if (v[i][0] > x_max) x_max = v[i][0];
	if (v[i][1] < y_min) y_min = v[i][1];
	if (v[i][1] > y_max) y_max = v[i][1];
      }
      if (x < x_min || x > x_max ||
	  y < y_min || y > y_max) {
	std::cerr << x << ", " << y << "\toutside of [" << x_min << ", " << x_max << "] , [ " << y_min << ", " << y_max << "]\n";
      } else {
	std::cerr << x << ", " << y << "\tINSIDE     [" << x_min << ", " << x_max << "] , [ " << y_min << ", " << y_max << "]\n";
      }
    }
#endif
    return inside;
  }
  
}
