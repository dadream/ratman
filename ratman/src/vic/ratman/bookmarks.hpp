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
#ifndef BOOKMARKS_HPP
#define BOOKMARKS_HPP

#include <vic/ratman/ratman.hpp>
#include<QString>
#include <QDomDocument>

namespace ratman {

  /**
   * Handles a vector of bookmarks. Each of them is made of a name,
   * a position and an orientation. Bookmarks are saved in 
   * GEORATMAN_DIR/bookmarks.kml, using xml syntax.
   */
  class bookmarks {

  public:

    typedef ratman::point3d_t point3d_t;

    bookmarks();
    ~bookmarks();


  public:

    std::size_t count(){
      return names_.size();
    }

    ratman::point3d_t location(int i) const{
      return locations_[i];
    }

    ratman::point3d_t orientation(int i) const{
      return orientations_[i];
    }

    QString name(int i) const{
      return names_[i];
    }

 
    void set_name(const QString& s,uint i);


    void insert(const QString& s,const point3d_t& p, const point3d_t& o){
      names_.push_back(s);
      locations_.push_back(p);
      orientations_.push_back(o);
    }

    void insert_xml_node(const QString& s,const point3d_t& p, const point3d_t& o);

    void delete_bookmark(uint i);
    void edit_kml_bookmark( const QString& s, bool to_delete, const QString& n = 0 );

    void clear_container(){
      names_.clear();
      locations_.clear();
      orientations_.clear();
    }

    void load_kml(QString filename);
    void save_kml(QString filename);

    QDomDocument* dom_document(){
      return &doc_;
    }

  protected:
    std::vector<QString> names_;
    std::vector<point3d_t> locations_;
    std::vector<point3d_t> orientations_;

    QDomDocument doc_;
   
  };

}
#endif

