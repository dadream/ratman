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
#include <vic/ratman/item3d_manager.hpp>
#include <sl/utility.hpp>

#define ITEMS3D_PRIORITY -1000

namespace ratman {

  item3d_manager::item3d_manager() :
    active_renderable(NULL, "") {
    this->set_active(false);
  }

  void item3d_manager::init_manager(decorated_terrain_view* scene, const std::string& name, std::istream& config) {
    scene_ = scene;
    name_ = name;
    items3d_ = new active_renderable(scene_,"Items 3D");
    items3d_->set_active(true);
    this->set_active(true);
    items3d_->set_priority(ITEMS3D_PRIORITY);
    scene_->insert_decoration_layer(items3d_);
    read_stream(config);
  }

  item3d_manager* item3d_manager::instance() {
    static item3d_manager* instance_ = new item3d_manager();
    return instance_;
  }

  void item3d_manager::read_node(vic::xml::node_iterator node) {
    assert(node.is_element_node());
    if ( node.tag() == "item3d_formats" ) {
	for ( vic::xml::node_iterator child = node.down(); !child.is_null() ; child = child.next() ) {
	  if ( child.tag() == "files" ) {
	    std::string file_extension, factory_name;
	    if (child.has_attribute("extension")) {
	      file_extension = child.attribute("extension");
	      if (child.has_attribute("factory")) {
		factory_name = child.attribute("factory");
		factory_name_map_[file_extension] = factory_name;
		SL_TRACE_OUT(-1) << "Associated file extension'" << file_extension << "' with factory '" << factory_name << "'" << std::endl;
	      }
	    } else {
	      std::cerr << "ERROR > item3d_manager::read_node : configuration incomplete, remember tags 'file_extension' and 'factory'." << std::endl;
	    }
	  }
	}
    }
  }

  void item3d_manager::render_managed() {
    if ( items3d_->is_active() ) {
      register_factory_t::iterator it;
      for (it = factory_map_.begin(); it != factory_map_.end(); it++) {
	((*it).second)->render_managed();
      }
    }
  }

  void item3d_manager::register_factory(const std::string name, item3d_factory* factory) {
    if ( factory_map_.find(name) == factory_map_.end() ) {
      factory_map_[name] = factory;
      SL_TRACE_OUT(-1) << "Registered factory with name '" << name << "'" << std::endl;
    }
    else {
	std::cerr << "WARNING > ratman::item3d_manager::register_factory : Factory with name " << name << " previously registered" << std::endl;
    }
  }

  item3d_factory* item3d_manager::find_factory(std::string name) {
    if ( factory_map_.find(name) != factory_map_.end() )
      return factory_map_[name];
    else
      return NULL;
  }

  void item3d_manager::create_item3d(const std::string& name, sl::point3d location, sl::point3d rotation,
				     sl::point3d scale,const std::string& file) {

      // Do extract file extension TO DO
      std::string file_extension = sl::pathname_extension(file);
      // Look for the factory associated for the file extension of the model file
      item3d_factory* factory = find_factory(factory_name_map_[file_extension]);
      if (factory != NULL) {
        if (scene_ != NULL) {
	  terrain_item3d* item3d = factory->create_item3d(scene_, name, location, rotation, scale, file);
          // If the item3d was created correctly add to the interface as an active object
          if (item3d != NULL) {
            item3d->set_priority(ITEMS3D_PRIORITY);
	    item3d->set_active(true);
            items3d_->insert_child(item3d);
          }
        } else {
          std::cerr << "Terrain scene is null!. Remember that is needed to create the manager before create an item3D" << std::endl;
        }
      }
  }

  void item3d_manager::set_atmosphere(atmosphere* atmosphere) {
    atmosphere_ = atmosphere;
  }

  bool item3d_manager::is_atmosphere() {
    return atmosphere_->is_active();
  }

  bool item3d_manager::on_select_self(const projective_map3d_t& /*P*/,
				      const rigid_body_map3d_t& /*V*/,
				      const point2d_t& /*xy*/) {
    bool result;
    //   mutex_.lock();
    {
      result = false;
    }
    // mutex_.unlock();
    return result;
  }
    
  void item3d_manager::render_self(qgl_scene_view& /*qgl*/,
				   occupancy_map_t& /*occupancy_map*/,
				   const projective_map3d_t& /*P*/,
				   const rigid_body_map3d_t& /*V*/,
				   const point3d_t& /*C*/) {
      // Render all the items 3d
      instance()->render_managed();
  }
            
  void item3d_manager::async_update_self(const projective_map3d_t& /*P*/,
					     const rigid_body_map3d_t& /*V*/,
					     const point3d_t& /*G*/) {
    // Nothing to do
  }

}
