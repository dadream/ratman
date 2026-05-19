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
#ifndef CBDAM_TERRAIN_MODEL_HPP
#define CBDAM_TERRAIN_MODEL_HPP

#include <vic/cbdam/base/config.hpp>
#include <vic/cbdam/base/geoimage_quad_fetcher.hpp>
#include <vic/cbdam/base/cbdam_diamond_fetcher.hpp>
#include <vic/cbdam/base/grid_diamond_graph_incore.hpp>
#include <vic/cbdam/base/grid_texture_quadtree.hpp>
#include <vic/cbdam/base/diamond_vertices.hpp>
#include <vic/cbdam/base/diamond_patch_accessor.hpp>
#include <vic/cbdam/base/texture_layer.hpp>
#include <vic/cbdam/base/geometry_layer.hpp>
#include <vic/geo/srs/spatial_reference.hpp>
#include <sl/rigid_body_map.hpp> 
#include <sl/projective_map.hpp>
#include <sl/oriented_box.hpp>
#include <QMutex>

namespace cbdam {

  class terrain_model_thread;
  
  /**
   *  The class manages all the information related to a terrain model.
   *  The class implements some useful functions to query parameters
   *  (such as planet radius, srs, etc...) 
   *  or to perform coordinate conversions.
   */
  class terrain_model {
  public:
    typedef grid_diamond                                                grid_diamond_t;
    typedef std::pair<grid_point_t, int>                                diamond_patch_id_t;
    typedef std::map<diamond_patch_id_t, diamond_patch_accessor*>       diamond_data_map_t;
    typedef sl::projective_map3d                                        projective_map_t;
    typedef sl::rigid_body_map3d                                        rigid_body_map_t;
    typedef sl::oriented_box<3, double>                                 bounding_volume_t;
    typedef diamond_vertices						diamond_vertices_t;

    // texture stuff
    typedef sl::fixed_size_point<4, uint8_t>					color4_t;
    typedef grid_diamond_graph_incore::refinement_heap_t                        refinement_heap_t;
    typedef texture_tile_stack::affine_map_t                                    affine_map_t;
    typedef texture_tile_stack::compressed_rgba_image_t				compressed_rgba_image_t;
    typedef texture_tile_stack::uncompressed_rgba_image_t			uncompressed_rgba_image_t;
    typedef reference_counted_object<compressed_rgba_image_t >                  reference_counted_compressed_image_t;
    typedef reference_counted_object<uncompressed_rgba_image_t >                reference_counted_uncompressed_image_t;
    typedef reference_counted_cache_base<grid_point_t, reference_counted_uncompressed_image_t > texture_cache_t;
    typedef std::set<std::pair<int, int> >					layer_set_t;

    // Network stuff
    typedef geoimage_quad_fetcher texture_fetcher_t;
    typedef cbdam_diamond_fetcher geometry_fetcher_t;

    typedef vic::geo::srs::spatial_reference spatial_reference_t;
    typedef vic::geo::srs::spatial_reference_transformation spatial_reference_transformation_t;

  protected:
    mutable QMutex              representation_mutex_;
    mutable QMutex              layers_mutex_;
    mutable QMutex              parameters_mutex_;

    terrain_model_thread*       update_thread_;

    geometry_layer*             geometry_layer_;
    std::vector<texture_layer*> base_overlay_color_layers_[2];
    texture_cache_t             texture_out_cache_;
    diamond_data_map_t*         current_representation_;
    double                      data_missing_fraction_;
    std::vector<std::string>	current_color_copyrights_;

    sl::projective_map3d        camera_projection_;
    sl::rigid_body_map3d        camera_view_;
    double			previous_camera_height_;

    float                       threshold_;
    float                       focus_fraction_;
    int				incremental_updates_count_;
    uint32_t			frame_counter_;

    uint32_t                    texture_cache_capacity_;
    color4_t                    background_color_;

    spatial_reference_t         spatial_reference_;

  public:

    /**
     *  Create terreain model from a geometry fetcher. Check is_open after
     *  creation to verify connection. 
     */    
    terrain_model(geometry_fetcher_t* gf);

    
    /**
     *  Release all data structures.
     */    
    ~terrain_model();

  public:

    /// True iff connected to geometry and able to stream
    bool is_connected() const;
 
    /// The spatial reference system of the geometry
    std::string srs() const;

    /// The uv coordinate bbox of the geometry
    aabox2d_t uv_box() const;

    /// The radius of the planet
    double radius() const;

    // FIXME
    const coordinate_transform* uvh_xyz_transform() const;

    // FIXME
    uint32_t height_patch_dim() const;

    // FIXME
    uint32_t texture_quad_width() const;

    // FIXME
    double height_scale_factor() const;

    // True iff the terrain is planar
    bool is_planar() const;

    const std::string& current_elevation_copyright() const;

  public: // Coordinate conversions

    const spatial_reference_t& spatial_reference_system() const;

    /// Coordinate conversion
    point3d_t xyz_from_WGS84_lonlat(const point3d_t& uvh) const;
    /// Coordinate conversion
    point3d_t uvh_from_WGS84_lonlat(const point3d_t& uvh) const;
    /// Coordinate conversion
    point3d_t xyz_from_uvh(const point3d_t& uvh) const;

    /// Coordinate conversion
    point3d_t uvh_from_xyz(const point3d_t& xyz) const;
    /// Coordinate conversion
    point3d_t WGS84_lonlat_from_xyz(const point3d_t& xyz) const;
    /// Coordinate conversion
    point3d_t WGS84_lonlat_from_uvh(const point3d_t& uvh) const;

    /// Coordinate conversion
    vector3d_t xyz_up_from_xyz(const point3d_t& xyz) const;
    /// Coordinate conversion
    vector3d_t xyz_north_from_xyz(const point3d_t& xyz) const;
    /// Coordinate conversion
    vector3d_t xyz_east_from_xyz(const point3d_t& xyz) const;

    /// Coordinate conversion
    vector3d_t xyz_up_from_uvh(const point3d_t& uvh) const;
    /// Coordinate conversion
    vector3d_t xyz_north_from_uvh(const point3d_t& uvh) const;
    /// Coordinate conversion
    vector3d_t xyz_east_from_uvh(const point3d_t& uvh) const;

    /// Coordinate conversion
    vector3d_t xyz_up_from_WGS84_lonlat(const point3d_t& uvh) const;
    /// Coordinate conversion
    vector3d_t xyz_north_from_WGS84_lonlat(const point3d_t& uvh) const;
    /// Coordinate conversion
    vector3d_t xyz_east_from_WGS84_lonlat(const point3d_t& uvh) const;
    /// Coordinate conversion
    rigid_body_map_t local_to_global_from_WGS84_lonlat(const point3d_t& uvh) const;

  public: // ray tracing

    /// Coordinate conversion
    std::pair<bool, double> current_representation_ground_elevation_from_WGS84_lonlat(const point2d_t& uvh) const;
    /// Coordinate conversion
    std::pair<bool, double> current_representation_ground_elevation_from_uv(const point2d_t& uv) const;
    /// Coordinate conversion
    std::pair<bool, double> current_representation_ground_elevation_from_xyz(const point3d_t& xyz) const;
    /// Coordinate conversion
    std::pair<bool, double> current_representation_nearest_intersection_from_xyz(const point3d_t& origin, 
										 const point3d_t& extremity) const;
    std::pair<bool, double> patch_elevation(const point3d_t& origin_xyz, 
					    const point3d_t& extremity_xyz, 
					    const diamond_vertices_t* v) const;

  public: // Layer access 
    /// The elevation data of the terrain   
    const geometry_layer* elevation_layer() const;

    /// The i-th base color layer   
    const texture_layer* base_color_layer(std::size_t i) const;

    /// The i-th overlay color layer   
    const texture_layer* overlay_color_layer(std::size_t i) const;

    /// The number of base color layer   
    std::size_t base_color_layer_count() const;

    /// The number of overlay color layer   
    std::size_t overlay_color_layer_count() const;

    /// Insert a new texture layer and return its id, size_t(-1) if error
    std::size_t insert_base_color_layer(const std::string& id,
					texture_fetcher_t* ds, 
					std::size_t first_level = 0,
					std::size_t last_level = 64,
					double min_altitude = -10e30,
					double max_altitude = 10e30,
					bool is_active = false);
    
    /// Insert a new texture layer and return its id, size_t(-1) if error
    std::size_t insert_overlay_color_layer(const std::string& id,
					   texture_fetcher_t* ds, 
					   std::size_t first_level = 0,
					   std::size_t last_level = 64,
					   double min_altitude = -10e30,
					   double max_altitude = 10e30,
					   bool is_active = false);
    
    /// Select the base color layer to be displayed
    void set_base_color_layer_active(std::size_t l);

    /// Enable/Disable the i-th overlay color level
    void set_overlay_color_layer_active(std::size_t l, bool x);
    /// True iff the i-th overlay color level is active
    bool is_overlay_color_layer_active(std::size_t l) const;

    /// Clean the base color layer structure
    void clear_base_color_layers();

    /// Clean the overlay  color layer structure
    void clear_overlay_color_layers();

    /// -1 if no active layer
    int active_base_layer() const;

  public: // Renderer interface

    void set_refine_parameters(float threshold, const projective_map_t& P, const rigid_body_map_t& V, float focus_fraction);
    
    void lock_current_representation() const;

    // Valid only with lock_current_rep!
    const diamond_data_map_t& current_representation() const;

    // Valid only with lock_current_rep!
    const std::vector<std::string>& current_color_copyrights() const;

    // Valid only with lock_current_rep!
    double data_missing_fraction() const;

    void unlock_current_representation() const;

    bool fetchers_data_available() const;

    int incremental_updates_count() const;

    uint32_t frame_counter() const;

  public:
     
    std::pair<float,float> estimated_elevation_range() const;

    bool is_visible(const bounding_volume_t& bv, const projective_map_t& camera_pv) const;

  protected: // Connection - called at creation

    void connect(geometry_fetcher_t* gf);

  protected:

    void extract_texture_cut();

    void tile_covering_diamond_in(grid_point_t& tile_level_xy, grid_diamond_t& tile_d,
                                  std::size_t level, const grid_diamond_t& d, int patch_id) const;

    void build_texture_stack_in(texture_tile_stack& ts, const grid_point_t& level_xy, const grid_diamond_t& d, layer_set_t& used_texture_layers,
				double camera_h) const;

  public: // Update thread

    void update_refine(bool& cut_updated);

    void update_swap_cuts();

    void update_tick();

  public:
    
    void update_start();

    void update_stop();

    void update_stop_and_clear();    

  protected:
    void clear_data_cut(diamond_data_map_t& cut);

    void fill_data_cut(diamond_data_map_t& cut,
		       std::vector<std::string>& copyrights);

    void regenerate_out_cache();

    uint64_t current_time_stamp();
  };


} // namespace cbdam 

#endif // CBDAM_TERRAIN_MODEL_HPP

#ifndef CBDAM_TERRAIN_MODEL_IPP
#define CBDAM_TERRAIN_MODEL_IPP

namespace cbdam {

  inline const coordinate_transform* terrain_model::uvh_xyz_transform() const {
    return geometry_layer_->diamond_graph().uvh_xyz_transform();
  }

  inline uint32_t terrain_model::height_patch_dim() const {
    return geometry_layer_->diamond_graph().height_patch_dim();
  }

  inline uint32_t terrain_model::texture_quad_width() const {
    // FIXME consider all layer with same texture_quad_witdh
    uint32_t result = 0;
    for(int i = 0; i < 2 && result == 0; ++i) {
      if (base_overlay_color_layers_[i].size()>0) {
	result = base_overlay_color_layers_[i][0]->fetcher()->quad_width();
      }
    }

    return result;
  }

  inline std::pair<float,float> terrain_model::estimated_elevation_range() const {
    return geometry_layer_->diamond_graph().estimated_elevation_range();
  }

  inline bool terrain_model::is_visible(const bounding_volume_t& bv, const projective_map_t& camera_pv) const {
    return geometry_layer_->diamond_graph().is_visible(bv, camera_pv); // FIXME
  }

  inline bool terrain_model::is_planar() const {
    return geometry_layer_->diamond_graph().is_planar();
  }

  inline double terrain_model::data_missing_fraction() const {
    return data_missing_fraction_;
  }
      
  inline const std::vector<std::string>& terrain_model::current_color_copyrights() const {
    return current_color_copyrights_;
  }

  inline double terrain_model::height_scale_factor() const {
    return geometry_layer_->diamond_graph().height_scale_factor();
  }

  inline const geometry_layer* terrain_model::elevation_layer() const {
    return geometry_layer_;
  }

  inline const texture_layer* terrain_model::base_color_layer(std::size_t i) const {
    assert(i < base_color_layer_count());
    return base_overlay_color_layers_[0][i];
  }

  inline const texture_layer* terrain_model::overlay_color_layer(std::size_t i) const {
    assert(i < overlay_color_layer_count());
    return base_overlay_color_layers_[1][i];
  }

  inline std::size_t terrain_model::base_color_layer_count() const {
    return base_overlay_color_layers_[0].size();
  }

  inline std::size_t terrain_model::overlay_color_layer_count() const {
    return base_overlay_color_layers_[1].size();
  }

  inline int terrain_model::active_base_layer() const {
    int result = -1;
    std::size_t check_active_count = 0;
    for(std::size_t i = 0; i < base_overlay_color_layers_[0].size(); ++i) {
      if (base_overlay_color_layers_[0][i]->is_active()) {
	result = i;
	++check_active_count;
      }
    }
    assert(check_active_count < 2);

    return result;
  }

  inline double terrain_model::radius() const {
    double result = 0.0;
    const spherical_coordinate_transform* geo_xform = dynamic_cast<const spherical_coordinate_transform*>(uvh_xyz_transform());
    if (geo_xform) {
      result = geo_xform->radius();
    }
    return result;
  }

  inline int terrain_model::incremental_updates_count() const {
    return incremental_updates_count_;
  }
    
  inline uint32_t terrain_model::frame_counter() const {
    return frame_counter_;
  }

} // namespace cbdam 

#endif // CBDAM_TERRAIN_MODEL_IPP
