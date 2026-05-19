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
#ifndef RATMAN_ITEM3D_MANAGER_HPP
#define RATMAN_ITEM3D_MANAGER_HPP

#include <vic/ratman/ratman.hpp>
#include <vic/ratman/atmosphere.hpp>
#include <vic/ratman/item3d_factory.hpp>
#include <vic/ratman/active_renderable.hpp>
#include <vic/ratman/decorated_terrain_view.hpp>
#include <vic/xml/document.hpp>

namespace ratman {

  /**
  *  The class which manages the association between file formats and item3d_factories.
  */
    class item3d_manager : public active_renderable, public vic::xml::document {
  public:
    typedef std::map<std::string, item3d_factory*> register_factory_t;
    typedef std::map<std::string, std::string> map_file_extension_factory_name_t;

  protected:
    register_factory_t factory_map_;
    map_file_extension_factory_name_t factory_name_map_;
    decorated_terrain_view* scene_;
    active_renderable* items3d_;
    atmosphere* atmosphere_;

  private:

    item3d_manager();

  public:
    virtual ~item3d_manager() { };

    static item3d_manager* instance();

    /// It is necessary to first call this method before operate with the manager. It inicializes the active renderable tree.
    void init_manager(decorated_terrain_view* scene, const std::string& name, std::istream& config);

    /// Parse the config for the 3d file formats supported by the item3d_manager
    void read_node(vic::xml::node_iterator node);

    /// Render items from factories who manage their own rendering process. i.e. OpenSceneGraph.
    virtual void render_managed();

    /// Register a new factory for items 3d with a name in the manager
    void register_factory(const std::string name, item3d_factory* factory);

    /// Find a factory by name in the manager.
    item3d_factory* find_factory(std::string name);

    /// Creates a terrain_item3d based on the file extension
    void create_item3d(const std::string& name,
		       sl::point3d location, sl::point3d rotation, sl::point3d scale,
		       const std::string& file);

    /// Sets the atmosphere
    virtual void set_atmosphere(atmosphere* atmosphere);

    /// Return is atmosphere flag is active or not
    virtual bool is_atmosphere();

    /// Handle picking and return true if handled
    virtual bool on_select_self(const projective_map3d_t& P,
                                const rigid_body_map3d_t& V,
                                const point2d_t& xy);

    /// Render the hierarchy object rooted at this
    virtual void render_self(qgl_scene_view& qgl,
                             occupancy_map_t& occupancy_map,
			     const projective_map3d_t& P,
                             const rigid_body_map3d_t& V,
			     const point3d_t& C);


    /// Asynchronously update the object. Called in a separate thread.
    virtual void async_update_self(const projective_map3d_t& P,
                                   const rigid_body_map3d_t& V,
				   const point3d_t& G);

  };

}

#endif
