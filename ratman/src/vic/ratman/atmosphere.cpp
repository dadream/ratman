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
#include <vic/ratman/atmosphere.hpp>
#include <vic/ratman/qgl_scene_view.hpp>
#include <qfile.h>
#include <QStatusBar>
#include <qtextstream.h>
#include <qstringlist.h>
#include <iostream>
#include <string>
#include <cmath>

#include <sl/rigid_body_map.hpp>

#include "sundisk.hpp"

#ifndef M_PI
# define M_PI		3.14159265358979323846
#endif

#define DTOR (M_PI/180.0f)
#define SQR(x) ((x)*(x))
 

namespace ratman {
  static const int MAP_WIDTH=256;
  static const int MAP_HEIGHT=32;

  static const int MAX_THETA_STEPS=120;
  static const float MAX_SUN_THETA=85;

  static const int DOME_AZIMUTH_STEP = 60; // 60;
  static const int DOME_ZENIT_STEP = 30; // 30;

  static const float L_th = 0.90f;
  static const float x_th = 0.50f;
  static const float y_th = 0.35f;
  
  //Coefficents for Perez distribution function
  static const float AY[] = {0.1787,-1.4630};
  static const float BY[] = {-0.3554,0.4275};
  static const float CY[] = {-0.0227,5.3251};
  static const float DY[] = {0.1206,-2.5771};
  static const float EY[] = {-0.0670,0.3703};

  static const float Ax[] = {-0.0193,-0.2592};
  static const float Bx[] = {-0.0665,0.0008};
  static const float Cx[] = {-0.0004,0.2125};
  static const float Dx[] = {-0.0641,-0.8989};
  static const float Ex[] = {-0.0033,0.0452};

  static const float Ay[] = {-0.0167,-0.2608};
  static const float By[] = {-0.0950,0.0092};
  static const float Cy[] = {-0.0079,0.2102};
  static const float Dy[] = {-0.0441,-1.6537};
  static const float Ey[] = {-0.0109,0.0529};

 
  static float solar_declination(int julian){
    //in radians
    return 0.4093*sin( (2*M_PI*float(julian-81))/368.0 );
  }

  static float solar_time(float time,float minutes,const float& j_date, float lon){
    //decimal time in hours
    //input angle must be in radian s
    float abs_lon = 15*int((lon+7.5)/15.0) - lon; // FIXME int was trunc
    return ( (time+minutes) + 0.170*sin(4*M_PI*(j_date-80.0)/373.0)-0.129*sin(2*M_PI*(j_date-8.0)/355.0)+(12.0*abs_lon/M_PI) );
  }
  
  static sl::vector2f sun_position(float solar_time,const float& sun_dcl,float lat)
  {
    sl::vector2f result;
    result[0] = (M_PI/2.0)-asinf( (sin(lat)*sin(sun_dcl)) - (cos(lat)*cos(sun_dcl)*cos(M_PI*solar_time/12.0)) );
    
    result[1] = atanf( -(cos(sun_dcl)*sin(M_PI*solar_time/12.0))/( (cos(lat)*sin(sun_dcl))-(sin(lat)*cos(sun_dcl)*cos(M_PI*solar_time/12.0)) ) );
    return result;
  }
  
  atmosphere::atmosphere(decorated_terrain_view* scene, const std::string& name) :
    active_renderable(scene,name) {

    current_sky_image_ = QImage(MAP_WIDTH,MAP_HEIGHT, QImage::Format_ARGB32);
    
    sun_visible_ = true;

    first_frame_=true;
    real_time_mode_=false;

    sun_azimuth_ = 90.0;
    sun_theta_ = 5.0;

    fov_y_ = 30.0;

    dome_radius_ = 1.0;
    sky_ambient_ = sl::vector3f(0.0,0.0,0.0);
    sun_diffuse_ = sl::vector3f(255.0,255.0,255.0);
    sun_direction_ = sl::vector3f(0.0,0.0,1.0);
    horizont_diffuse_front_ = sl::vector3f(255.0,255.0,255.0);
    horizont_diffuse_back_ = sl::vector3f(255.0,255.0,255.0);

    current_longitude_ = 9.0;
    current_latitude_ = 40.0;
   
    current_decl_ = solar_declination(j_date_);
  
    PF_sun_.clear();
    PF_sun_.resize(MAX_THETA_STEPS);

    gl_texid_[0] = 0;
    gl_texid_[1] = 0;
  }
  
  atmosphere::~atmosphere() {
    for (int i=0; i<2; i++){
      glDeleteTextures(1, &gl_texid_[i]);
    }
    destroy_dome();
  }

  void atmosphere::init_gl_tex(){
    for (unsigned int i=0; i<2; i++){
      glGenTextures(1, &gl_texid_[i]);
      glBindTexture(GL_TEXTURE_2D, gl_texid_[i]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
      glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
      glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE); 
    }
  }

  void atmosphere::init_gl_draw(){
    build_quads_dome(dome_radius_, DOME_AZIMUTH_STEP, DOME_ZENIT_STEP);
    init_gl_tex();

    if (sun_visible_){
      set_sun();
    }
    compute_sky();
  }

  void atmosphere::set_active(bool x) {
    active_renderable::set_active(x);
  }

  bool atmosphere::on_select_self(const projective_map3d_t& /*P*/,
				  const rigid_body_map3d_t& /*V*/,
				  const point2d_t& /*xy*/) {
    return false;
  }

 
  bool atmosphere::on_event_self(qgl_scene_view& qgl,
				 QEvent* e,
				 const projective_map3d_t& /*P*/,
				 const rigid_body_map3d_t& /*V*/) { 
    if (e->type() == QEvent::KeyPress) {
      QKeyEvent *ke = dynamic_cast<QKeyEvent *>(e);  
      if (ke->key() == Qt::Key_B){
	std::cerr << "*******geo position  "<<qgl.current_WGS84_lonlat_position()<< std::endl;
	return true;
      }
    }
    return false;
  }

  static sl::rigid_body_map3d rotations_local_to_global(qgl_scene_view& qgl_sv,const point3d_t& xyz) {
    point3d_t O = qgl_sv.uvh_xyz_transform()->xyz_on_ground(xyz);
    vector3d_t X = qgl_sv.uvh_xyz_transform()->east_from_xyz(O);
    vector3d_t Y = qgl_sv.uvh_xyz_transform()->north_from_xyz(O);
    vector3d_t Z = qgl_sv.uvh_xyz_transform()->up_from_xyz(O);
    
    return rigid_body_map3d_t(sl::matrix4d(X[0], Y[0], Z[0], 0.0,
					   X[1], Y[1], Z[1], 0.0,
					   X[2], Y[2], Z[2], 0.0,
					    0.0,  0.0,  0.0, 1.0)); 
  } 
  
  void atmosphere::render_self(qgl_scene_view& qgl,
			       occupancy_map_t& /*occupancy_map*/,
			       const projective_map3d_t& P,
			       const rigid_body_map3d_t& V,
			       const point3d_t& /*C*/) {
    
    if (first_frame_) {
      init_gl_draw();
      set_skycolor(current_sky_image_);
      first_frame_=false;
    }

    current_frame_ = rotations_local_to_global(qgl,
					       qgl.camera_controller().camera_position_xyz());

    if (real_time_mode_){ 
      if (qgl.camera_controller().get_oriented_position() != last_oriented_position_ && clock_.elapsed().as_milliseconds() >= 1000) {
	point3d_t lon_lat_elv = qgl.current_WGS84_lonlat_position();
	current_longitude_ = lon_lat_elv[0];
	current_latitude_  = lon_lat_elv[1];
	set_real_sun(current_longitude_,current_latitude_);
	clock_.restart();
	last_oriented_position_ = qgl.camera_controller().get_oriented_position();
      } 
    }
        
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    {
      glMatrixMode(GL_TEXTURE);
      glLoadIdentity();
      glMatrixMode(GL_MODELVIEW);
      glDisable(GL_LIGHTING);
      glDisable(GL_TEXTURE_GEN_S);
      glDisable(GL_TEXTURE_GEN_T);
      glEnable(GL_ALPHA_TEST);
      glDisable(GL_DEPTH_TEST);
      glDepthMask(GL_FALSE);
      glDisable(GL_CULL_FACE);
      
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      
      glEnable(GL_TEXTURE_2D);
      // Extract far distance
      vector4d_t p_zfar = (~(P*V)).as_matrix()*vector4d_t(0.0f, 0.0f, 1.0f, 1.0f);
      double     atmosphere_radius = std::abs(p_zfar[2]/p_zfar[3]);
      glMatrixMode(GL_MODELVIEW);
      glPushMatrix();
      {
	// Zero out translation and scale to max size within frustum
	matrix4x4d_t V_prime = V.as_matrix();
	V_prime(0,3) = 0.0f; V_prime(1,3) = 0.0f; V_prime(2,3) = 0.0f;
	glLoadMatrixd(V_prime.to_pointer());
	glScalef(0.4*atmosphere_radius,0.4*atmosphere_radius,0.4*atmosphere_radius);
	
	glPushMatrix(); 
	{
	  float atm_density = sl::median(0.0f, 1.0f, float(24000.0f/qgl.camera_controller().camera_altitude()));
	  sky_ambient_ = sl::vector3f(255.0*(1.0-0.8*atm_density),255.0*(1.0-0.8*atm_density),255.0*(1.0-0.8*atm_density));
	  glColor4f(1.0,1.0,1.0,atm_density);
	  //frame reference on the sphere
	  glMultMatrixd(current_frame_.to_pointer());
	  render_quads_skydome();
	}
	glPopMatrix();
	
	if (sun_visible_){
	  glColor4f(1.0,1.0,1.0,1.0);
	  //frame reference on the sphere -- done -- inside light position
	  render_sun_disk(V_prime);
	}
	
      } 
      glPopMatrix();
      
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_ALPHA_TEST);
      glDepthMask(GL_TRUE);
      glEnable(GL_CULL_FACE);
      glDisable(GL_BLEND);
    }
    glPopAttrib();
  }
  
  
  void atmosphere::build_quads_dome(float radius, int dtheta, int dphi){
    destroy_dome();
    
    for (int phi= 0; phi <= 180-dphi; phi+=dphi) {
      for (int theta=0; theta<=360-dtheta; theta+=dtheta) {
	
	for (int vtx = 0; vtx<4; ++vtx) {
	  static const int phi_offset[4]   = { 0, 0, 1, 1 };
	  static const int theta_offset[4] = { 0, 1, 1, 0 };
	  
	  int phi_prime = phi + phi_offset[vtx]*dphi;
	  int theta_prime = theta +theta_offset[vtx]*dtheta;
	  
	  sl::point3f xyz(float(radius * sinf((phi_prime)*DTOR) * cosf((theta_prime)*DTOR)),
			  float(radius * sinf((phi_prime)*DTOR) * sinf((theta_prime)*DTOR)),
			  float(radius * cosf((phi_prime)*DTOR)));
	  
	  sl::point2f uv = sl::point2f( (float(theta_prime)/360.0f),
					1.0 - (float(phi_prime)/90.0f) );
	  if (uv[1]< 0.0f) { uv[1]=0.0f; }
	  
	  sky_vertices_xyz_.push_back(xyz);
	  sky_vertices_uv_.push_back(uv);
	}
      }
    }
    
    SL_TRACE_OUT(1) << "Created dome with " << sky_vertices_uv_.size() << std::endl;
  }
  
  void atmosphere::destroy_dome(){
    sky_vertices_xyz_.clear();
    sky_vertices_uv_.clear();
  } 
  
  void atmosphere::render_quads_skydome(){
    glPushMatrix();
    {
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      
      glBindTexture(GL_TEXTURE_2D, gl_texid_[0]);
      glBegin(GL_QUADS);
      {
	for (std::size_t i=0; i < sky_vertices_uv_.size(); ++i) {
	  glTexCoord2fv( sky_vertices_uv_[i].to_pointer() );
	  glVertex3fv( sky_vertices_xyz_[i].to_pointer() );
	}
      }
      glEnd();
      glBindTexture(GL_TEXTURE_2D, 0);
    }
    glPopMatrix();
  }

  void atmosphere::set_skycolor(const QImage &img){
    //QImage crop_img = img.copy(0,0,img.width(),img.height()-2);
    if (gl_texid_[0] != 0) {
      QImage skymap = QGLWidget::convertToGLFormat(img);
      glBindTexture(GL_TEXTURE_2D, gl_texid_[0]);
      glTexImage2D(GL_TEXTURE_2D, 
		   0, 
		   (skymap.hasAlphaChannel() ? 4 : 3), 
		   skymap.width(), 
		   skymap.height(), 0,
		   GL_RGBA, 
		   GL_UNSIGNED_BYTE, 
		   skymap.bits()
		   );
    }
  }
  
  void atmosphere::set_sun(){
    QImage sunmap = QGLWidget::convertToGLFormat(QImage(sundisk.pixel_data,sundisk.width,sundisk.height, QImage::Format_ARGB32));
    //QImage sunmap = QGLWidget::convertToGLFormat(QImage("sun.png"));
    glBindTexture(GL_TEXTURE_2D, gl_texid_[1]);
    glTexImage2D(GL_TEXTURE_2D, 
		 0, 
		 (sunmap.hasAlphaChannel() ? 4 : 3), 
		 sunmap.width(), 
		 sunmap.height(), 0,
		 GL_RGBA, 
		 GL_UNSIGNED_BYTE, 
		 sunmap.bits());
  }
  
  
  static float gamma_sun_dome( float theta, float phi, float theta_s, float phi_s ) {
    sl::vector3f s ( sin( theta_s * DTOR ) * cos ( phi_s * DTOR ),
		     sin( theta_s * DTOR ) * sin ( phi_s * DTOR ),
		     cos( theta_s * DTOR ) );
    sl::vector3f v ( sin( theta * DTOR ) * cos ( phi * DTOR ),
		     sin( theta * DTOR ) * sin ( phi * DTOR ),
		     cos( theta * DTOR ) );
    return  atan2 ( s.cross(v).two_norm(), s.dot(v) ) / DTOR;
  }
  
  static sl::vector3f Lxy_to_rgb(const sl::vector3f &Lxy){
    sl::vector3f XYZ(Lxy[0]*(Lxy[1]/Lxy[2]),
		     Lxy[0],
		     Lxy[0]*((1-Lxy[1]-Lxy[2])/Lxy[2]) );
    
    sl::matrix3f weights(3.240479, -1.53715, -0.49853,
			 -0.969256, +1.875991, +0.041556,
			 0.055648,  -0.204043, +1.057311);
    sl::vector3f rgb;
    rgb[0] = sl::median(0.0f, 1.0f, (weights * XYZ)[0])*255.0;
    rgb[1] = sl::median(0.0f, 1.0f, (weights * XYZ)[1])*255.0;
    rgb[2] = sl::median(0.0f, 1.0f, (weights * XYZ)[2])*255.0;
  
    return rgb;
  }

  
  template<class vector3_t>
  vector3_t product_by_component( const vector3_t& a, const vector3_t& b ) {
    return vector3_t( a[0] * b[0] , a[1] * b[1], a[2] * b[2] );
  }
  
  template<class vector3_t>
  vector3_t division_by_component( const vector3_t& a, const vector3_t& b ) {
    return vector3_t( a[0] / b[0] , a[1] / b[1], a[2] / b[2] );
  }
  
  template <class value_t>
  void  logarithmic_mean(std::vector<value_t>& values,
			 const double& threshold,
			 double& mean_value)
  {
    double log_product = 1.0;
    double log_mean_value = 0.0;
    
    const std::size_t total_voxel_count = values.size();
    // Log mean computation
    for ( std::size_t i = 0; i < total_voxel_count; ++i ) {
      log_product *= ( 1.0 + values[i] );
      if (log_product >= threshold ) {
	log_mean_value += log ( log_product );
	log_product = 1.0;
      }
    }
    // Logarithmic scale intensity 
    log_mean_value += log ( log_product );
    mean_value =  exp ( log_mean_value  / static_cast<double>( total_voxel_count ) ) -  1.0;
  }
  
  
  static float estimate_luminance(const float& current,const float& max,const float& min){
    float result = min;
    if ( (current/max) > min ) {
      result = (current/max);
    }
    if ( current <= 0.0 ) {
      result = 0.0;
    }
    return result;
  }

  static float estimate_crominance(const float& current,const float& th){
    float value = th;
    if ( (current) < th ) {
      value = current;
    }
    return value;
  }

  void atmosphere::compute_sky(){
    //GEO PARAM inizialize sun and atmospheric parameters
    zenit_values_ = Yxy_zenit();
    sl::vector3f PF_sun = PF_sun_[sl::min(int(sun_theta_), MAX_THETA_STEPS-1)];
    //std::cerr << "zenit_values_  "<<zenit_values_<< std::endl;
    float Y_max = -1.0e6;
    
    std::vector<float> Y(MAP_WIDTH * MAP_HEIGHT );
    std::vector<float> x(MAP_WIDTH * MAP_HEIGHT );
    std::vector<float> y(MAP_WIDTH * MAP_HEIGHT );
    
    float theta_step = 90.0 / float(MAP_HEIGHT);
    float phi_step = 360.0 / float(MAP_WIDTH);
    
    for (std::size_t i = 0; i < std::size_t(MAP_WIDTH); ++i ) {
      for (std::size_t j = 0; j < std::size_t(MAP_HEIGHT); ++j ) {
	// Sample at center of texels
	float point_theta = (0.5+j)*theta_step;
	float point_phi   = (0.5+i)*phi_step;
	float point_gamma =  gamma_sun_dome( point_theta, point_phi, sun_theta_, sun_azimuth_ );
	
	sl::vector3f cur_Yxy = product_by_component( zenit_values_, division_by_component( PF_distribution(point_theta,point_gamma), PF_sun) );
	
	Y[i + j*MAP_WIDTH] = cur_Yxy[0];
	x[i + j*MAP_WIDTH] = cur_Yxy[1];
	y[i + j*MAP_WIDTH] = cur_Yxy[2];
	
	if (  cur_Yxy[0] > Y_max  )  {
	  Y_max  =  cur_Yxy[0];
	}
	
      }
    }
    
    for (std::size_t i = 0; i < std::size_t(MAP_WIDTH); ++i ) {
      for (std::size_t j = 0; j < std::size_t(MAP_HEIGHT); ++j ) {
	sl::vector3f rgb_color = Lxy_to_rgb( sl::vector3f(estimate_luminance( Y[i + j*MAP_WIDTH],Y_max,L_th),
							  estimate_crominance(x[i + j*MAP_WIDTH],x_th),
							  estimate_crominance(y[i + j*MAP_WIDTH],y_th)
							  ));
	int air_alpha = 255;
	QRgb color = qRgba ( int(rgb_color[0]), int(rgb_color[1]),int(rgb_color[2]), air_alpha );
	current_sky_image_.setPixel(i,j, color);
      }
    }
    
    //Update lighting and haze*****************************************************
    // Sample at center of texels
    float sun_sample_theta = (sun_theta_+theta_step/2.0f);
    float sun_sample_phi   = (sun_azimuth_+phi_step/2.0f);
    
    point3d_t l_xyz = sl::point3d(sin( sun_sample_theta *DTOR )*cos( sun_sample_phi *DTOR ),
				  sin( sun_sample_theta *DTOR )*sin( sun_sample_phi *DTOR ),
				  cos( sun_sample_theta *DTOR )
				  );
    point3d_t l_xyz_local = transformation(current_frame_, l_xyz);
    sun_direction_ = sl::vector3f( l_xyz_local[0], l_xyz_local[1], l_xyz_local[2] );
    //std::cerr << "*****sun_direction_     "<<sun_direction_<< std::endl;

    
    //TURN OFF THE LIGHT BELOW THE HORIZONT -- MOONLIGHT DIFFUSE
    if ( sun_theta_ > MAX_SUN_THETA)
      {
	sun_visible_ = false;
	sun_diffuse_ = sl::vector3f( 255.0f,255.0f,255.0f );
	sun_direction_ = sl::vector3f( -0.992086,-0.0767509,0.0993729);

	if ( !current_sky_image_.isNull() ){
	  set_skycolor(current_sky_image_);
	  horizont_diffuse_front_ = sun_diffuse_; 
	}
      }
    else
      {
	sun_visible_ = true;
	sl::vector3f sun_Yxy = product_by_component( zenit_values_, division_by_component( PF_distribution(sun_sample_theta,0.0f), PF_sun) );
	sl::vector3f low_light_ = Lxy_to_rgb(sl::vector3f( estimate_luminance( sun_Yxy[0],Y_max,0.87f),
							   estimate_crominance(sun_Yxy[1],0.50f),
							   estimate_crominance(sun_Yxy[2],0.40f) 
							   ) );
	QColor sun_light = QColor( int(low_light_[0]), int(low_light_[1]), int(low_light_[2]), 255 ).light( int(160-(sun_theta_/2.0f)) );
	sun_diffuse_ = sl::vector3f( sun_light.red(),sun_light.green(),sun_light.blue() );
	
	if ( !current_sky_image_.isNull() ){
	  set_skycolor(current_sky_image_);
	  if (sun_sample_phi>360.0f) sun_sample_phi -= 360.0f; 
	  horizont_diffuse_front_ = sl::vector3f( qRed( current_sky_image_.pixel( int((MAP_WIDTH-1)*(sun_sample_phi/360.0f)),MAP_HEIGHT-1 )),
						  qGreen( current_sky_image_.pixel( int((MAP_WIDTH-1)*(sun_sample_phi/360.0f)),MAP_HEIGHT-1 )),
						  qBlue( current_sky_image_.pixel( int((MAP_WIDTH-1)*(sun_sample_phi/360.0f)),MAP_HEIGHT-1 ))
						  );
	}
	
      }
    
#if 0
    QString file_name = QString("skydome%1az%2th%3t")
      .arg(sun_azimuth_)
      .arg(sun_theta_)
      .arg(turbidity_);
    file_name.append(".png");
    current_sky_image_.save(file_name,"png",-1);
#endif
    
  }

  static float x_chromaticity(float sun_theta,float turbidity){
    sl::vector4f theta_factor(powf(sun_theta*DTOR,3.0),powf(sun_theta*DTOR,2.0),sun_theta*DTOR,1.0);
    
    sl::matrix4f Mx(0.00166, -0.00375, 0.00209, 0.0,
		    -0.02903, 0.06377, -0.03202, 0.00394,
		    0.11693, -0.21196, 0.06052, 0.25886,
		    0.0, 0.0, 0.0, 1.0);

    sl::vector4f x_coef = Mx * theta_factor;
    return ( x_coef[0]*powf(turbidity,2.0)+ x_coef[1]*turbidity+ x_coef[2] );
  }

  static float y_chromaticity(float sun_theta,float turbidity){
    sl::vector4f theta_factor(powf(sun_theta*DTOR,3.0),powf(sun_theta*DTOR,2.0),sun_theta*DTOR,1.0);

    sl::matrix4f My(0.00275, -0.00610, 0.00317, 0.0,
		    -0.04214, 0.08970, -0.04153, 0.00516,
		    0.15346, -0.26756, 0.06670, 0.26688,
		    0.0, 0.0, 0.0, 1.0);

    sl::vector4f y_coef = My * theta_factor;
    return ( y_coef[0]*powf(turbidity,2.0)+ y_coef[1]*turbidity+ y_coef[2] );
  }
  
  sl::vector3f atmosphere::Yxy_zenit(){
    sl::vector3f zenit_values;
    //GP  Yz formula
    float Yz = (4.0453 * turbidity_ - 4.9710)*tanf( ((4.0/9.0)-(turbidity_/120)) * (180.0-2.0*sun_theta_)*DTOR )-0.2155*turbidity_+10.775;
    if (sun_theta_ > MAX_SUN_THETA){
      zenit_values[0]=0.0;
    } else{
      zenit_values[0] = Yz;
    }
    zenit_values[1] = x_chromaticity(sun_theta_,turbidity_);
    zenit_values[2] = y_chromaticity(sun_theta_,turbidity_);
    return zenit_values;
  }
   
  sl::vector3f atmosphere::PF_distribution(const float& theta,const float& gamma){
    sl::vector3f result( 1.0f, 1.0f, 1.0f);

    float c2gamma = cos( gamma * DTOR ) * cos( gamma * DTOR );
    float ctheta = sl::median(0.01f, 1.0f, cosf( theta * DTOR ) ); 

    result[0] = 
      (( 1+ (AY[0]*turbidity_+AY[1])*exp((BY[0]*turbidity_+BY[1])/ctheta) )*
       ( 1+ (CY[0]*turbidity_+CY[1])*exp((DY[0]*turbidity_+DY[1])*gamma*DTOR)+(EY[0]*turbidity_+EY[1])* c2gamma ));
    
    result[1] = 
      (( 1+ (Ax[0]*turbidity_+Ax[1])*exp((Bx[0]*turbidity_+Bx[1])/ctheta) )*
       ( 1+ (Cx[0]*turbidity_+Cx[1])*exp((Dx[0]*turbidity_+Dx[1])*gamma*DTOR)+(Ex[0]*turbidity_+Ex[1])* c2gamma ));
      
    result[2] = 
      (( 1+ (Ay[0]*turbidity_+Ay[1])*exp((By[0]*turbidity_+By[1])/ctheta) )*
       ( 1+ (Cy[0]*turbidity_+Cy[1])*exp((Dy[0]*turbidity_+Dy[1])*gamma*DTOR)+(Ey[0]*turbidity_+Ey[1])* c2gamma ));

    return result;
  }
  
  sl::vector3f atmosphere::light_direction(){
    return sun_direction_;
  }
  
  sl::color4f atmosphere::sky_ambient(){
    return sl::color4f(sky_ambient_[0]/255.0,sky_ambient_[1]/255.0,sky_ambient_[2]/255.0,1.0);
  }
  
  sl::color4f atmosphere::sun_diffuse(){
    //std::cerr << "*****sun_diffuse  "<<sun_diffuse_<< std::endl;
    return sl::color4f(sun_diffuse_[0]/255.0,sun_diffuse_[1]/255.0,sun_diffuse_[2]/255.0,1.0);
  }
  
  sl::color4f atmosphere::fog_color(float yaw){
    //FIXME convert to NE coordinates
    float yaw_ne = yaw+90.0f;
    sl::vector3f result;
    float angle_vs = fabsf(yaw_ne-sun_azimuth_);
    float a = cos(DTOR*angle_vs*3.0)*sin(DTOR*sun_theta_);
    if ( (angle_vs > 89.0) || (a <= 0.0f) ) a=0.0f;

    if ( !current_sky_image_.isNull() ){
      if (yaw_ne>360.0f) yaw_ne -= 360.0f; 
      horizont_diffuse_back_ = sl::vector3f( qRed( current_sky_image_.pixel( int((MAP_WIDTH-1)*(yaw_ne/360.0f)),MAP_HEIGHT-1 )),
					     qGreen( current_sky_image_.pixel( int((MAP_WIDTH-1)*(yaw_ne/360.0f)),MAP_HEIGHT-1 )),
					     qBlue( current_sky_image_.pixel( int((MAP_WIDTH-1)*(yaw_ne/360.0f)),MAP_HEIGHT-1 ))
					     );
    }
    
    result = sl::vector3f( (a*horizont_diffuse_front_[0]+(1.0-a)*horizont_diffuse_back_[0]),
			   (a*horizont_diffuse_front_[1]+(1.0-a)*horizont_diffuse_back_[1]),
			   (a*horizont_diffuse_front_[2]+(1.0-a)*horizont_diffuse_back_[2]));
    
    return sl::color4f(result[0]/255.0f,result[1]/255.0f,result[2]/255.0f,
		       qAlpha( current_sky_image_.pixel( int((MAP_WIDTH-1)*(yaw_ne/360.0f)),MAP_HEIGHT-1 ))/255.0f);
  }

  void atmosphere::set_turbidity(int a){
    turbidity_ = float(a);
    for (int i=0; i<MAX_THETA_STEPS; i++){
      PF_sun_[i] = PF_distribution(0.0,float(i)); 
    }
  }
  
  void atmosphere::set_daytime(int a)
  {
    daytime_ = float(a);
    //HACK time tuning
    //if (daytime_ != 0 && daytime_ != 24) daytime_ -= 2;
    set_real_sun(current_longitude_, current_latitude_);
  }

  void atmosphere::set_minutes(int a){
    minutes_ = float(a)/60.0;
    set_real_sun(current_longitude_, current_latitude_);
  }
  
  void atmosphere::set_date(int a)
  {
    j_date_ = a;
    set_real_sun(current_longitude_, current_latitude_);
  }

  void atmosphere::set_real_sun(const float& current_lon, const float& current_lat){
    sl::vector2f sun_tp = sun_position( solar_time( daytime_, minutes_, j_date_, current_lon*DTOR ), solar_declination(j_date_), current_lat*DTOR );
    sun_theta_ = sun_tp[0]/DTOR;
    //convert to NE coordinates
    sun_azimuth_ = 270.0-sun_tp[1]/DTOR;
    //std::cerr << "*****sun position     "<<sun_theta_<<" "<<sun_azimuth_<< std::endl;
    compute_sky();
  }
  
  void atmosphere::set_real_time_mode(bool b){
    real_time_mode_=b;
  }

 void atmosphere::render_sun_disk(const matrix4x4d_t& V){
   matrix4x4d_t C = V.inverse();
   vector3d_t   camera_x   = vector3d_t(C(0,0), C(0,1), C(0,2)).ok_normalized();
   vector3d_t   camera_y   = vector3d_t(C(1,0), C(1,1), C(1,2)).ok_normalized();
   vector3d_t   b_up    = camera_y;
   vector3d_t   b_right = camera_x;
   vector3d_t   b_look  = b_right.cross(b_up);
   matrix4x4d_t sunplane_matrix;
   sunplane_matrix(0,0) = b_right[0]; sunplane_matrix(0,1) = b_right[1]; sunplane_matrix(0,2) = b_right[2]; sunplane_matrix(0,3) = 0.0;
   sunplane_matrix(1,0) = b_up[0];    sunplane_matrix(1,1) = b_up[1];    sunplane_matrix(1,2) = b_up[2];    sunplane_matrix(1,3) = 0.0;
   sunplane_matrix(2,0) = b_look[0];  sunplane_matrix(2,1) = b_look[1];  sunplane_matrix(2,2) = b_look[2];  sunplane_matrix(2,3) = 0.0;
   sunplane_matrix(3,0) = 0.0;        sunplane_matrix(3,1) = 0.0;        sunplane_matrix(3,2) = 0.0;        sunplane_matrix(3,3) = 1.0;
    
   glPushMatrix();
   {
         
      glTranslated(light_direction()[0], light_direction()[1], light_direction()[2]);
      glMultMatrixd(sunplane_matrix.to_pointer()); // Undo view rotation
      glBindTexture(GL_TEXTURE_2D, gl_texid_[1]);
      glColor3f(sun_diffuse()[0], sun_diffuse()[1], sun_diffuse()[2]);
      glBegin(GL_QUADS);
      {
	glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.2,-0.2, 0.0);
	glTexCoord2f(1.0f, 0.0f); glVertex3f(0.2,-0.2, 0.0);
	glTexCoord2f(1.0f, 1.0f); glVertex3f(0.2, 0.2, 0.0);
	glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.2, 0.2, 0.0);
      } 
      glEnd();
      glBindTexture(GL_TEXTURE_2D, 0);	  
    }
    glPopMatrix();
  } 
  
  
}
