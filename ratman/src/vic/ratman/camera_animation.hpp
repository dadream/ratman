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
#ifndef RATMAN_CAMERA_ANIMATION_HPP
#define RATMAN_CAMERA_ANIMATION_HPP

#include <vic/ratman/oriented_position.hpp>
#include <sl/interpolation.hpp>
#include <sl/clock.hpp>
#include <QObject>

namespace ratman {

  class camera_animation {
  protected:
    typedef sl::interpolation_track<double, oriented_position> interpolation_track_t;
  protected:
    interpolation_track_t* interpolation_track_;
    sl::real_time_clock	   clock_;
    bool		   is_active_;	
	
  public:
    camera_animation();

    ~camera_animation();

    void compute_track_from_to(const oriented_position& from, const oriented_position& to, double to_millisec);
    void compute_fly_from_to(const oriented_position& from, const oriented_position& to, double to_millisec);

    void begin_track();

    void add_track_point(double t_millisec, const oriented_position& p);

    void end_track();

    void start();

    void stop();

    bool is_active();

    double end_time() const;

    oriented_position current_position_xyz() const;

    double current_time() const;
  };

}
#endif // RATMAN_CAMERA_ANIMATION_HPP

#ifndef RATMAN_CAMERA_ANIMATION_IPP
#define RATMAN_CAMERA_ANIMATION_IPP

namespace ratman {
  inline void camera_animation::start() {
    clock_.restart();
    is_active_ = true;
  }

  inline void camera_animation::stop() {
    is_active_ = false;
  }

}
#endif // RATMAN_CAMERA_ANIMATION_IPP
