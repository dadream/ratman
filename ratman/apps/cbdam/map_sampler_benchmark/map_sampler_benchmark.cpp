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
#include <vic/cbdam/base/coordinate_transform.hpp>
#include <vic/cbdam/geo/map_raster_sampler.hpp>
#include <vic/cbdam/geo/map_mosaic_sampler.hpp>
#include <vic/cbdam/base/diamond_graph_builder.hpp>

#include <sl/clock.hpp>

#include <GL/gl.h>
#include <GL/glut.h>

typedef cbdam::diamond_graph_builder                       diamond_graph_builder_t;
typedef cbdam::diamond_graph_builder::diamond_graph_t      diamond_graph_t;
typedef cbdam::diamond_graph_builder::diamond_t            diamond_t;
typedef cbdam::diamond_graph_builder::diamond_state_t      diamond_state_t;
typedef cbdam::diamond_graph_builder::diamond_id_t         diamond_id_t;
typedef cbdam::diamond_graph_builder::diamond_id_t         grid_point_t;
typedef cbdam::coordinate_transform                        geo_xform_t;              

typedef vic::geo::map_rgb_sampler<8>                                  geo_map_sampler_t;

// ======================================================================

static diamond_graph_t* dgraph;

static void glut_reshape(int w, int h) {
  glViewport(0, 0, w, h);
}

static void glut_draw() {

  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClear(GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho( 0.0, 1.0,
	   0.0, 1.0,
	  -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_LIGHTING);

  glColor3f(1.0, 1.0, 1.0);

  if (dgraph) {
    glBegin(GL_LINE_STRIP);

    const std::size_t level =  dgraph->level_count()-1;
    const std::size_t diamond_count = dgraph->level_diamond_count(level);

    for (diamond_graph_t::grid_diamond_map_const_iterator_t cdiamond_it = dgraph->level_begin(level);
	 cdiamond_it != dgraph->level_end(level);
	 ++cdiamond_it) {
      const diamond_t         d    = cdiamond_it->first;
      const diamond_id_t      d_id = d.id();
      
      float x = float(d_id[0]-cbdam::min_grid_coord()) / float(cbdam::max_grid_coord()-cbdam::min_grid_coord());
      float y = float(d_id[1]-cbdam::min_grid_coord()) / float(cbdam::max_grid_coord()-cbdam::min_grid_coord());
      
      glVertex2f(x,y);
      std::cerr << x << " " << y << std::endl;
    }
    glEnd();
  }

  glutSwapBuffers();
}

static void glut_init(int argc, char * argv[], int width, int height) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
  glutInitWindowSize (width, height);
  //  glutCreateWindow("THE ASTONISHING GLUT VIC VBDAM HOLOVIEWER");
  glutCreateWindow("Graph");
  glutDisplayFunc(glut_draw);
  glutReshapeFunc(glut_reshape);
}

static void build_graph(const geo_map_sampler_t* input_sampler,
			const geo_xform_t*       geo_xform,
			std::size_t              N=128,
			double min_duv=0.0) {
  std::cerr << "---------------------------------------------------------------------------" << std::endl;
  std::cerr << "Building diamond graph" << std::endl;
  std::cerr << "---------------------------------------------------------------------------" << std::endl;
  
  sl::real_time_clock ck;
  ck.restart();

  diamond_graph_builder_t gb;
  dgraph =  gb.new_diamond_graph("/tmp/dgraph", // FIXME!!!!
				 input_sampler,
				 geo_xform,
				 N, 
				 min_duv,
				 false,
				 false);
  std::cerr << "---------------------------------------------------------------------------" << std::endl;
  std::cerr << "TIME: " << sl::human_readable_duration(ck.elapsed()) << std::endl;
  std::cerr << "---------------------------------------------------------------------------" << std::endl;
  std::cerr << std::endl;
}

// ======================================================================
static void benchmark(const geo_map_sampler_t* input_sampler,
		      const geo_xform_t*       geo_xform,
		      std::size_t              N=128,
		      double min_duv=0.0) {

  std::cerr << "---------------------------------------------------------------------------" << std::endl;
  std::cerr << "Sampling leafs of diamond graph" << std::endl;
  std::cerr << "---------------------------------------------------------------------------" << std::endl;

  sl::real_time_clock ck;
  ck.restart();
  std::size_t  sampled_diamond_count = 0;
  sl::uint64_t sampled_pixel_count = 0;
  sl::uint64_t last_report_sampled_pixel_count = 0;
  const std::size_t level =  dgraph->level_count()-1;
  const std::size_t diamond_count = dgraph->level_diamond_count(level);

  for (diamond_graph_t::grid_diamond_map_const_iterator_t cdiamond_it = dgraph->level_begin(level);
       cdiamond_it != dgraph->level_end(level);
       ++cdiamond_it) {
    const diamond_t         d    = cdiamond_it->first;
    const diamond_id_t      d_id = d.id();

    const bool is_valid_fragment[2] = {
      d.is_valid_fragment(0),
      d.is_valid_fragment(1)
    };
    
    // sample input diamond
    for (int patch_id=0; patch_id<2; ++patch_id) {
      // get patch data if valid
      if (is_valid_fragment[patch_id]) {
        // planar dataset: interpolate over corners
	const grid_point_t gp0 = d.corner((1+2*patch_id)%4);
	const grid_point_t gp1 = d.corner((2+2*patch_id)%4);
	const grid_point_t gp2 = d.corner((0+2*patch_id)%4);
	
        const cbdam::point3d_t  dgp0     = cbdam::point3d_t(gp0[0], gp0[1], gp0[2]);
        const cbdam::vector3d_t dgp0dgp1 = cbdam::point3d_t(gp1[0], gp1[1], gp1[2]) - dgp0;
        const cbdam::vector3d_t dgp0dgp2 = cbdam::point3d_t(gp2[0], gp2[1], gp2[2]) - dgp0;
        
        const double inv_patch_dim = 1.0/float(N);
        
        // get control points
        for(int y = 0; y <= int(N); ++y) {
          for(int x = 0; x <= int(N) - y; ++x) {
	    // P
	    cbdam::point3d_t dgp_p = (dgp0 + dgp0dgp1 * inv_patch_dim * (x     ) + dgp0dgp2 * inv_patch_dim * (y     ));
	    grid_point_t gp_p = grid_point_t(int32_t(dgp_p[0]), int32_t(dgp_p[1]), int32_t(dgp_p[2]));
	    cbdam::point2d_t uv = geo_xform->uv_from_grid(gp_p);
	    (void)input_sampler->value_at(uv[0], uv[1]);
	    ++sampled_pixel_count;
	    // Q
	    if ((y<int(N)) && (x<int(N)-y)) {
	      cbdam::point3d_t dgp_q = (dgp0 + dgp0dgp1 * inv_patch_dim * (x+0.5f) + dgp0dgp2 * inv_patch_dim * (y+0.5f));
	      grid_point_t gp_q = grid_point_t(int32_t(dgp_q[0]), int32_t(dgp_q[1]), int32_t(dgp_q[2]));
	      cbdam::point2d_t uv = geo_xform->uv_from_grid(gp_q);
	      (void)input_sampler->value_at(uv[0], uv[1]);
	      ++sampled_pixel_count;
	    }
          }
        } 
      }
    }

    ++sampled_diamond_count;

    if ((sampled_pixel_count-last_report_sampled_pixel_count)>50000) {
      sl::time_duration t_current_rt = ck.elapsed();
      sl::time_duration t_eta_rt     = sl::time_duration(sl::int64_t(double(diamond_count)*t_current_rt.as_microseconds()/double(sampled_diamond_count)));
      //std::cerr << std::endl << "REQ = " << input_sampler->stat_requested_sample_count() << " LOAD=" << input_sampler->stat_loaded_sample_count() << std::endl;
      std::cerr << 
	"[" << sl::human_readable_percent(100.0*double(sampled_diamond_count)/double(diamond_count)) << "]"
	" T: " << sl::human_readable_duration(t_current_rt) << 
	" - ETA: " << sl::human_readable_duration(t_eta_rt) << 
	" - Speed: " << sl::human_readable_quantity(double(sampled_pixel_count)/(0.001*double(ck.elapsed().as_milliseconds()))) << "samples/s" <<
	" - IO Efficiency: " << sl::human_readable_percent(100.0*
							      double(input_sampler->stat_requested_sample_count())/
							      double(input_sampler->stat_loaded_sample_count())) <<
	"           \r";
      last_report_sampled_pixel_count = sampled_pixel_count;
    }
  } // for each diamond
  std::cerr << std::endl << "DONE!" << std::endl; 
  std::cerr << "REQUESTED PIXELS=" << input_sampler->stat_requested_sample_count() << std::endl;
  std::cerr << "LOADED    PIXELS=" << input_sampler->stat_loaded_sample_count() << std::endl;
}

// ======================================================================
int main(int argc, char** argv) {
  if (argc != 3) {
    std::cerr << "usage " << argv[0] << " dirname pattern" << std::endl;
    return 0;
  }

  const std::string dirname = argv[1];
  const std::string pattern = argv[2];
 
  vic::geo::map_mosaic_sampler mosaic(dirname, pattern);
   
  vic::geo::map_rgb_sampler<8> rgb_sampler(&mosaic);
  cbdam::planar_coordinate_transform geo_xform;
  geo_xform.set_bounding_rectangle(rgb_sampler.bounding_rectangle());

  build_graph(&rgb_sampler,
	      &geo_xform);

  glut_init(argc, argv, 512, 512);
  glutMainLoop();

  benchmark(&rgb_sampler,
	    &geo_xform);
             
  return 0;
}
