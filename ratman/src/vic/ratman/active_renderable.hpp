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
#ifndef RATMAN_ACTIVE_RENDERABLE_HPP
#define RATMAN_ACTIVE_RENDERABLE_HPP

#include <GL/glew.h>
#include <vic/ratman/ratman.hpp>
#include <sl/dense_array.hpp>
#include <vic/gl/font.hpp>
#include <string>
#include <vector>
#include <qmutex.h> // change to qreadwritelock
#include <QImage>
#include <QEvent>

namespace ratman {

  class qgl_scene_view;
  class decorated_terrain_view;
  class terrain_renderable;

  /**
   * Named objects that are asynchronously updated and
   * are able to render their current representation.
   * Objects are maintained in a hierarchy. The implementation
   * assumes two threads: one for updating and one for all the
   * rest. The updating thread has read-only access to
   * the object state with the only exception of the representation
   * used for rendering.
   */
  class active_renderable {
  public:
    typedef active_renderable this_t;
    typedef sl::dense_array<bool, 2, void> occupancy_map_t; 
  protected:
    mutable QMutex mutex_;
    
    int32_t priority_;
    std::string name_;
    decorated_terrain_view* scene_;
    this_t* parent_;
    std::vector<this_t*> children_;

    bool is_initialized_;
    bool is_active_;
    bool is_selectable_;

    QImage* icon_;

  protected: // Shared font object. 

    // FIXME: Actually, we should make the font an instance variable of
    // qgl_scene_view, as we need one font per opengl context.
    // We should probably make active_renderables children of
    // qgl_scene_view. For now the following hack works because we have
    // only one context.
    
    static vic::gl::font* shared_3dfont_;

    inline vic::gl::font& shared_3dfont() const {
      return *shared_3dfont_;
    }
    
  public:
    
    active_renderable(decorated_terrain_view* scene, 
		      const std::string& name = "Anonymous", 
		      QImage* icon=0);

    virtual ~active_renderable();
    
  public:

    const decorated_terrain_view* scene() const {
      return scene_;
    }

    decorated_terrain_view* scene() {
      return scene_;
    }

    const terrain_renderable* terrain_layer() const;

    terrain_renderable* terrain_layer();
    
    double altitude(const point3d_t& xyz) const;

  public: // id

    /// The objects' icon
    const QImage* icon() const;
    QImage* icon();
    
    /// The objects' name
    const std::string& name() const;

    /// The objects' priority (for sorting)
    int32_t priority() const;

    /// Set  objects' priority (for sorting)
    void set_priority(int32_t x);

    /// Set  objects' priority (for sorting)
    void set_child_priority(this_t* child, int32_t x);
    
  public: // hierarchy

    /// The number of children
    std::size_t child_count() const;
    
    /// The i-th child of this object (in current order)
    this_t* child(std::size_t i);

    /// The i-th child of this object (in current order)
    const this_t* child(std::size_t i) const;

    /// The first child of this object named id, or null if not found
    this_t* child(const std::string& id);

    /// The first child of this object named id, or null if not found
    const this_t* child(const std::string& id) const;

    /// The first child of this object named id, or null if not found
    this_t* descendant(const std::string& id);

    /// The first child of this object named id, or null if not found
    const this_t* descendant(const std::string& id) const;

    /// The parent of this object, or null if root
    this_t* parent();

    /// The parent of this object, or null if root
    const this_t* parent() const;

    /// Insert given child
    void insert_child(this_t* child);

    /// Erase given child
    void erase_child(this_t* child);

    /// Erase all children (do not delete them)
    void erase_children();
    
  public:
    
    /// Has the object been initialized?
    virtual bool is_initialized() const;

    /// Initialize the hierarchy of objects rooted at this 
    virtual void initialize();

    /// Initialize this 
    virtual void initialize_self();

    /// Finalize the hierarchy of objects rooted at this, releasing all resources
    virtual void finalize();      

    /// Finalize this, releasing all resources
    virtual void finalize_self();      

  public: // Activation/deactivation
    
    /// Is the object active? If true, the object is updated and rendered.
    virtual bool is_active() const;

    /// Activate/deactivate the object
    virtual void set_active(bool x);
    
  public: // Selectable
    
    /// Is the object active? If true, the object is updated and rendered.
    virtual bool is_selectable() const;

    /// Activate/deactivate the object
    virtual void set_selectable(bool x);
    
  public: // Events

    /// Handle picking and return true if handled
    virtual bool on_event(qgl_scene_view& qgl,
			  QEvent* e,
			  const projective_map3d_t& P,
			  const rigid_body_map3d_t& V);

    /// Handle picking and return true if handled
    virtual bool on_event_self(qgl_scene_view& qgl,
			       QEvent* e,
			       const projective_map3d_t& P,
			       const rigid_body_map3d_t& V);

  public: // Rendering

    /// Render the hierarchy object rooted at this
    virtual void render(qgl_scene_view& qgl,
                        occupancy_map_t& occupancy_map,
			const projective_map3d_t& P,
                        const rigid_body_map3d_t& V,
			const point3d_t& C); // FIXME I3D

    /// Render the hierarchy object rooted at this
    virtual void render_self(qgl_scene_view& qgl,
                             occupancy_map_t& occupancy_map,
			     const projective_map3d_t& P,
                             const rigid_body_map3d_t& V,
			     const point3d_t& C);
    
  public: // Async update

    /// Asynchronously update the hierarchy of objects rooted at this. Called in a separate thread.
    virtual void async_update(const projective_map3d_t& P,
                              const rigid_body_map3d_t& V,
			      const point3d_t& G);

    /// Asynchronously update the object. Called in a separate thread.
    virtual void async_update_self(const projective_map3d_t& P,
                                   const rigid_body_map3d_t& V,
				   const point3d_t& G);

  };

}

#endif
