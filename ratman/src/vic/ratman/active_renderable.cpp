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
#include <vic/ratman/active_renderable.hpp>
#include <vic/ratman/decorated_terrain_view.hpp>
#include <vic/ratman/terrain_renderable.hpp>
#include <vic/cbdam/base/terrain_model.hpp>

#include <algorithm> // for std::sort
#include <QEvent>

namespace ratman {

  struct prioritized_active_renderable_pointer_cmp {
    bool operator()(const active_renderable* x,
                    const active_renderable* y) const {
      int32_t x_priority = x->priority();
      int32_t y_priority = y->priority();
      if (x_priority < y_priority) {
        return true;
      } else if (y_priority < x_priority) {
        return false;
      } else {
        // Equal priority, sort by pointer
        return ((const void*)x) < ((const void*)y);
      }
    }
  };


  //----- static
  
  vic::gl::font* active_renderable::shared_3dfont_;

  
  //----- creation and destruction
  
  active_renderable::active_renderable(decorated_terrain_view* scene, const std::string& name, QImage* icon)
      :
      mutex_(QMutex::Recursive),
      priority_(0),
      name_(name),
      scene_(scene),
      parent_(0),
      children_(),
      is_initialized_(false),
      is_active_(false),
      is_selectable_(true)
  {
    if (!shared_3dfont_) {
      shared_3dfont_ = new vic::gl::font();
      shared_3dfont_->set_scaling(1.0f,1.0f);
    }
    icon_=icon;
  }

  active_renderable::~active_renderable() {
    finalize(); 
  }

  const terrain_renderable* active_renderable::terrain_layer() const {
    assert(scene_);
    return scene_->terrain_layer();
  }

  terrain_renderable* active_renderable::terrain_layer() {
    assert(scene_);
    return scene_->terrain_layer();
  }

  double active_renderable::altitude(const point3d_t& xyz) const {
    assert(scene_);
    return terrain_layer()->model()->uvh_xyz_transform()->altitude_from_xyz(xyz);
  }

  // ----- icon
 const QImage* active_renderable::icon() const {
    return icon_;
  }

  QImage* active_renderable::icon() {
    return icon_;
  }


  // ----- name and priority
  
  const std::string& active_renderable:: name() const {
    // Assume constant and never changed, don't use mutex
    return name_;
  }
    
  int32_t active_renderable::priority() const {
    int32_t result = priority_;
    return result;
  }

  void active_renderable::set_priority(int32_t x) {
    if (parent_) {
      parent_->set_child_priority(this, x);
    } else {
      mutex_.lock();
      priority_ = x;
      mutex_.unlock();
    }
  }

  void active_renderable::set_child_priority(this_t* c, int32_t x) {
    mutex_.lock();
    {
      c->priority_ = x; // FIXME
      std::sort(children_.begin(), children_.end(), prioritized_active_renderable_pointer_cmp());
    }
    mutex_.unlock();
  }
  
  // ----- hierarchy

  std::size_t active_renderable::child_count() const {
    return children_.size();
  }
  
  active_renderable::this_t* active_renderable::child(std::size_t i) {
    return children_[i];
  }

  const active_renderable::this_t* active_renderable::child(std::size_t i) const {
    return children_[i];
  }

  active_renderable::this_t* active_renderable::child(const std::string& id) {
    this_t* result = 0;
    for (std::size_t i=0; i<children_.size() && !result; ++i) {
      if (children_[i]->name() == id) {
        result = children_[i];
      }
    }
    return result;
  }

 const active_renderable::this_t* active_renderable::child(const std::string& id) const {
    this_t* result = 0;
    for (std::size_t i=0; i<children_.size() && !result; ++i) {
      if (children_[i]->name() == id) {
        result = children_[i];
      }
    }
    return result;
  }

  active_renderable::this_t* active_renderable::descendant(const std::string& id) {
    this_t* result = child(id);
    for (std::size_t i=0; i<children_.size() && !result; ++i) {
      result = children_[i]->descendant(id);
    }
    return result;
  }    

  const active_renderable::this_t* active_renderable::descendant(const std::string& id) const {
    const this_t* result = child(id);
    for (std::size_t i=0; i<children_.size() && !result; ++i) {
      result = children_[i]->descendant(id);
    }
    return result;
  }    

  /// The parent of this object, or null if root
  active_renderable::this_t* active_renderable::parent() {
    return parent_;
  }

  /// The parent of this object, or null if root
  const active_renderable::this_t* active_renderable::parent() const {
    return parent_;
  }

  /// Insert given child
  void active_renderable::insert_child(this_t* child) {
    mutex_.lock();
    {
      children_.push_back(child);
      child->parent_ = this; // ??? 
      std::sort(children_.begin(), children_.end(), prioritized_active_renderable_pointer_cmp());
    }
    mutex_.unlock();
  }
  
  /// Erase given child
  void active_renderable::erase_child(this_t* child) {
    mutex_.lock();
    {
      std::vector<this_t*>::iterator it_to_erase = children_.end();
      for (std::vector<this_t*>::iterator it=children_.begin();
           (it!= children_.end()) && (it_to_erase!=children_.end());
           ++it) {
        if ((*it) == child) it_to_erase = it;
      }
      if (it_to_erase != children_.end()) {
        children_.erase(it_to_erase);
        child->parent_ = 0; // ??? 
      }
    }
    mutex_.unlock();
  }
  
  /// Erase all children (do not delete them)
  void active_renderable::erase_children() {
    mutex_.lock();
    {
      while (!children_.empty()) {
        this_t* child = children_.back();
        children_.pop_back();
        child->parent_ = 0;
      }
    }
    mutex_.unlock();
  }

  bool active_renderable::is_initialized() const {
    bool result = is_initialized_;
    return result;
  }

  void active_renderable::initialize() {
    initialize_self();
    mutex_.lock();
    {
      for (std::vector<this_t*>::iterator it=children_.begin();
           it!= children_.end();
           ++it) {
        (*it)->initialize();
      }
    }
    mutex_.unlock();
  }

  void active_renderable::initialize_self() {
    mutex_.lock();
    {
      /* NOTHING TO DO BY DEFAULT */
      is_initialized_ = true; 
    }
    mutex_.unlock();
  }

  void active_renderable::finalize() {
    mutex_.lock();
    {
      for (std::vector<this_t*>::iterator it=children_.begin();
           it!= children_.end();
           ++it) {
        (*it)->finalize();
      }
    }
    mutex_.unlock();
    
    finalize_self();
  }
  
  void active_renderable::finalize_self() {
    mutex_.lock();
    {
      /* NOTHING TO DO BY DEFAULT */
      is_initialized_ = false; 
    }
    mutex_.unlock();
  }

  // ----- Activation/deactivation

  bool active_renderable::is_active() const {
    return is_active_;
  }

  void active_renderable::set_active(bool x) {
    mutex_.lock();
    {
      is_active_ = x; 
    }
    mutex_.unlock();
  }

  // ----- Activation/deactivation

  bool active_renderable::is_selectable() const {
    return is_selectable_;
  }

  void active_renderable::set_selectable(bool x) {
    mutex_.lock();
    {
      is_selectable_ = x; 
    }
    mutex_.unlock();
  }


  // ----- Events

  bool active_renderable::on_event(qgl_scene_view& qgl,
				    QEvent* e,
				    const projective_map3d_t& P,
                                    const rigid_body_map3d_t& V) {
    bool result = false;
    if (is_active()) {
      // Reverse as painting order
      // FRONT TO BACK...
      for (std::vector<this_t*>::reverse_iterator it=children_.rbegin();
           (it!= children_.rend()) && (!result);
           ++it) {
        if (!result) result = (*it)->on_event(qgl,e,P,V);
      }
      if (!result) result = on_event_self(qgl,e,P,V);
    }
    e->setAccepted(result);
    return result;
  }

  bool active_renderable::on_event_self(qgl_scene_view& /*qgl*/,
					 QEvent* /*e*/,
					 const projective_map3d_t& /*P*/,
                                         const rigid_body_map3d_t& /*V*/) {
    /* NOTHING TO DO BY DEFAULT */
    return false;
  }

  // ----- Rendering

  void active_renderable::render(qgl_scene_view& qgl,
                                 occupancy_map_t& occupancy_map,
				 const projective_map3d_t& P,
                                 const rigid_body_map3d_t& V,
				 const point3d_t& C) {
    if (is_active()) {
      // BACK TO FRONT... 
      render_self(qgl, occupancy_map, P,V,C);
      {
        for (std::vector<this_t*>::iterator it=children_.begin();
             it!= children_.end();
             ++it) {
          (*it)->render(qgl, occupancy_map, P,V,C);
        }
      }
    }
  }

  void active_renderable::render_self(qgl_scene_view& /*qgl*/,
                                      occupancy_map_t& /*occupancy_map*/,
				      const projective_map3d_t& /*P*/,
                                      const rigid_body_map3d_t& /*V*/,
				      const point3d_t& /*C*/) {
    mutex_.lock();
    {
      /* NOTHING TO DO BY DEFAULT */
    }
    mutex_.unlock();
  }

  void active_renderable::async_update(const projective_map3d_t& P,
                                       const rigid_body_map3d_t& V,
				       const point3d_t& G) {
    if (is_active()) {
      async_update_self(P,V,G);
      
      // FIXME -- Assume hierarchy modifs made with render thread stopped
      // FIXME -- mutex_.lock(); 
      {
        for (std::vector<this_t*>::iterator it=children_.begin();
             it!= children_.end();
             ++it) {
          (*it)->async_update(P,V,G);
        }
      }
    }
    // FIXME -- mutex_.unlock();
  }

  /// Asynchronously update the object. Called in a separate thread.
  void active_renderable::async_update_self(const projective_map3d_t& /*P*/,
                                            const rigid_body_map3d_t& /*V*/,
					    const point3d_t& /*G*/) {
    mutex_.lock();
    {
      /* NOTHING TO DO BY DEFAULT */
    }
    mutex_.unlock();
  }    

}
