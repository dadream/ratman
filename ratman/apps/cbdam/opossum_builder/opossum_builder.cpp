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
#include <iostream>
//#include <vic/opossum/scene_compiler.hpp>
#include <opossum/scene_compiler.hpp> // to compile with old opossum applications
#include <qimage.h>
#include <shapefil.h>
#include <cassert>

#define USE_CRS4_VERSION 1

static int elevation_field_sampler(void* context,
				   double u,
				   double v) {
  assert(context);
  assert(0.0 <= u && u <= 1.0);
  assert(0.0 <= v && v <= 1.0);
  
  QImage* qi = static_cast<QImage*>(context);
  assert(qi->depth() == 8);
  
  int x = (int)(u * (qi->width() - 1));
  int y = (int)((1.0 - v) * (qi->height() - 1));

  return qi->scanLine(y)[x];
}

static opossum::rgb8_color_t color_field_sampler(void* context,
                                                 double u,
                                                 double v) {
  assert(context);
  assert(0.0 <= u && u <= 1.0);
  assert(0.0 <= v && v <= 1.0);
  
  QImage* qi = static_cast<QImage*>(context);
  int x = (int)(u * (qi->width() - 1));
  int y = (int)((1.0 - v) * (qi->height() - 1));
  QRgb p = qi->pixel(x, y);
  opossum::rgb8_color_t c;
  c.r = qRed(p);
  c.g = qGreen(p);
  c.b = qBlue(p);
  return c;
}

#if USE_CRS4_VERSION
static void build_buildings(const char* input_buildings_file_name,
                            opossum::scene_compiler& scene_compiler,
			    double u_extent,
			    double v_extent) {
  SHPHandle sfile = SHPOpen(input_buildings_file_name, "rb");
  if (sfile) {
    scene_compiler.buildings_begin();
    
    std::cerr << "Open file of type " << sfile->nShapeType << " with " << sfile->nRecords << " objs" << std::endl;
    std::cerr << "     " <<
      " BOX = " << "[ " <<
      sfile->adBoundsMin[0] << " " << sfile->adBoundsMin[1] << " " << sfile->adBoundsMin[2] << " " << sfile->adBoundsMin[3] << "; " <<
      sfile->adBoundsMax[0] << " " << sfile->adBoundsMax[1] << " " << sfile->adBoundsMax[2] << " " << sfile->adBoundsMax[3] << "]" << std::endl;
    
    int count = 0;

    // translate to min_x, min_y.
    // paris values
    float xmin = 447000;	//  sfile->adBoundsMin[0] 
    float ymin = 5409000;	//  sfile->adBoundsMin[1]
    std::cerr << "According to Paris dataset, remove its origin P(" << xmin << ", " << ymin << ") from buildings coordinates\n";
    for (int i = 0; i < sfile->nRecords; ++i) {
      SHPObject* obj = SHPReadObject(sfile, i);
      float zmin = 0; // std::max(obj->dfZMin, 0.0);
      //      if (zmin != obj->dfZMax) {
      {
	// if obj->nParts is 0: a single outline is present else nParts
	int outlines_count = obj->nParts != 0 ? obj->nParts : 1;

	// begin building, u, v in parametric coords, z in absolute coords
        scene_compiler.building_begin((obj->dfXMin - xmin)/u_extent, (obj->dfYMin - ymin)/v_extent, zmin,
				      (obj->dfXMax - xmin)/u_extent, (obj->dfYMax - ymin)/v_extent, obj->dfZMax,
				      outlines_count);
	
	// insert all outlines
	int first_vert = 0;
	int last_vert = obj->nVertices;
	
	for(int outlines = 0; outlines < outlines_count; ++outlines) {
	  if (obj->nParts > 1) {
	    // panPartStart[i] is the start vertex of part i
	    first_vert = obj->panPartStart[outlines];
	    if (outlines == outlines_count - 1) {
	      last_vert = obj->nVertices;
	    } else {
	      last_vert = obj->panPartStart[outlines+1];
	    }
	  }
	  
	  scene_compiler.building_outline_begin(last_vert-first_vert);
	    
	  // add building vertices: u, v in parametric coords, z in absolute coords
	  for(int j = first_vert; j < last_vert; ++j) {
	    double u = (obj->padfX[j] - xmin)/u_extent;
	    double v = (obj->padfY[j] - ymin)/v_extent;
	    scene_compiler.building_vertex(u, v);
	  }
	  scene_compiler.building_outline_end();
	}

	// end building
	scene_compiler.building_end();
	++count;
      }
      
      if (i%1000 == 0) {
        std::cerr << "processed " << i << " objects\r";
      }
    }

    std::cerr << "inserted     " << count << " objects\n";
    std::cerr << "not inserted " << sfile->nRecords - count << " flat objects\n";
    std::cerr << "build objects bsp..\n";

    // end buildings compilation
    scene_compiler.buildings_end();
    std::cerr << "buildings done\n";
    
    SHPClose(sfile);
  } else {
    std::cerr << "Error opening buildings " << input_buildings_file_name << std::endl;
  }
}
#else
static void build_buildings(const char* input_buildings_file_name, opossum::scene_compiler& scene_compiler,
                            double u_extent, double v_extent)
{
	SHPHandle hShp = SHPT_NULL;
	DBFHandle hDbf = 0;

	const std::string shpFileName = std::string(input_buildings_file_name) + ".shp";
	const int maxEntities = 0;

	hShp = SHPOpen(input_buildings_file_name, "rb");
	if (hShp == SHPT_NULL)
	{
		printf("unable to open file \"%s\", exit.\n", shpFileName);
		return;
	}

	int entities = 0;
	int shapetype = 0;
	double bMin[4];
	double bMax[4];

	SHPGetInfo(hShp, &entities, &shapetype, bMin, bMax);
	if ((shapetype != SHPT_POLYGON) && (shapetype != SHPT_POLYGONZ))
	{
		SHPClose(hShp);
		printf("polygon format is not handled, exit.\n");
		return;
	}

	entities = (maxEntities == 0) ? (entities) : ((entities < maxEntities) ? (entities) : (maxEntities));

	float heights[2];

	printf("started buildings processing of %d entities...\n", entities);

	scene_compiler.buildings_begin();

	for (int i=0; i<entities; ++i)
	{
		printf("\rreading input: %d of %d   ", i+1, entities);

		SHPObject * obj = SHPReadObject(hShp, i);
		if (obj == 0)
		{
			SHPDestroyObject(obj);
			continue;
		}
		SHPComputeExtents(obj);
		if (obj->dfZMax <= 0.0f)
		{
			SHPDestroyObject(obj);
			continue;
		}

		heights[0] = 0.0f;
		heights[1] = obj->dfZMax;

        scene_compiler.building_begin(0.0, 0.0, heights[0], 0.0, 0.0, heights[1], 0);

		bool firstVert = true;
		float lastX = 0.0f;
		float lastY = 0.0f;
		for (int j=0; j<(int)(obj->nParts); ++j)
		{
			scene_compiler.building_outline_begin(0);
			const int nMax = (j == (obj->nParts - 1)) ? (obj->nVertices-1) : (obj->panPartStart[j+1]-1);
			int hk = 0;
			int jk = nMax;
			for (int k=obj->panPartStart[j]; k<=nMax; ++k, ++hk, --jk)
			{
				const float bx = (((float)(obj->padfX[jk] - bMin[0])) / u_extent);
				const float by = (((float)(obj->padfY[jk] - bMin[1])) / v_extent);
				if (firstVert)
				{
					firstVert = true;
					scene_compiler.building_vertex(bx, by);
				}
				else
				{
					if ((lastX != bx) || (lastY != by))
					{
						scene_compiler.building_vertex(bx, by);
					}
					lastX = bx;
					lastY = by;
				}
			}
			scene_compiler.building_outline_end();
		}

		SHPDestroyObject(obj);

		scene_compiler.building_end();
	}

	SHPClose(hShp);

	printf("\rinput read.                                \n");

	scene_compiler.buildings_end();

	printf("buildings processing done.\n");
}
#endif
                             
static void build_data(const char* input_height_file_name,
                       const char* input_color_file_name,
                       const char* input_buildings_file_name,
                       const char* output_file_name,
                       double u_extent,
                       double v_extent,
                       double height_ds,
                       double color_ds,
                       unsigned int patch_dim,
                       unsigned int subsampling_level,
                       double height_scale_factor) {
  QImage height_image;
  QImage color_image;
  opossum::scene_compiler scene_compiler;

  // load heights and set height sampler in the scene compiler
  if (input_height_file_name != 0) {
    if (height_image.load(input_height_file_name)) {
      // convert to monochrome if rgb image
      if (height_image.depth() > 8) {
        height_image = height_image.convertDepth(8, Qt::MonoOnly);
      }
      std::cerr << "set scene height sampler\n";
      scene_compiler.set_elevation_field_sampler(&height_image, elevation_field_sampler, height_scale_factor);
    } else {
      std::cerr << "unable to load heights from " << input_height_file_name << std::endl;
      return;
    }
  }

  // load colors and set color sampler in the scene compiler
  if (input_color_file_name != 0) {
    if (color_image.load(input_color_file_name)) {
      // color sampler is used to build colors and to set the roof color of the buildings
      scene_compiler.set_color_field_sampler(&color_image, color_field_sampler);
    } else {
      std::cerr << "unable to load colors from " << input_height_file_name << std::endl;
      return;
    }
  }

  // set other scene compiler parameters
  bool create_buildings = input_buildings_file_name != 0;
  bool create_colors = input_color_file_name != 0;

  scene_compiler.set_extent(u_extent, v_extent);
  scene_compiler.set_target_elevation_field_sampling_rate(height_ds);
  scene_compiler.set_target_color_field_sampling_rate(color_ds);
  scene_compiler.set_patch_dimension(patch_dim);
  scene_compiler.set_subsampling_level(subsampling_level);
  scene_compiler.set_color_construction_enabled(create_colors);
  scene_compiler.set_building_construction_enabled(create_buildings);

  std::cerr << "Begin processing terrain heights and colors\n";
  scene_compiler.scene_begin(output_file_name);

  if (scene_compiler.is_open()) {
    // build buildings
    if (create_buildings) {
      build_buildings(input_buildings_file_name, scene_compiler, u_extent, v_extent);
    }
    
    // close compilation
    scene_compiler.scene_end();
    std::cerr << "done\n";
  }
}

void print_error_and_exit(const std::string& str) {
  std::cerr << str << std::endl;
  std::cerr << "usage: opossum_builder [-height height-file-name] [-color color-file-name] [-buildings buildings-file-name] [-output output-file-name]\n";
  exit(0);
}

int main(int argc, const char** argv) {
  if (argc == 1) {
    print_error_and_exit("expected input output parameters");
  }
  const char* input_height_file_name = 0;
  const char* input_color_file_name = 0;
  const char* input_buildings_file_name = 0;
  const char* output_file_name = 0;

#if 1
  // paris
  double u_extent = 9000.0;
  double v_extent = 9000.0;
  double height_ds = 1;
  double color_ds =  1; 	// the proper value for the paris texture 36kx36k is 0.25 (its sampling rate in m)
  unsigned int patch_dim = 64;
  unsigned int subsampling_level = 4; //each level halves the number of points of the multires structure
  double height_scale_factor = 0.5;
#else
  // puget sound
  double u_extent = 1024.0*160;
  double v_extent = 1024.0*160;
  double height_ds = 160.0;
  double color_ds =  160.0;
  unsigned int patch_dim = 64;
  unsigned int subsampling_level = 0; //each level halves the number of points of the multires structure
  double height_scale_factor = 25.6;
#endif
  int count = 1;
  // parse file name parameters
  while(count < argc) {
    if (argv[count][0] == '-') {
      const char* param = &argv[count][1];
      ++count;
      if (count < argc) {
        if (strcmp(param, "height") == 0) {
          input_height_file_name = argv[count];
        } else if (strcmp(param, "color") == 0) {
          input_color_file_name = argv[count];
        } else if (strcmp(param, "buildings") == 0) {
          input_buildings_file_name = argv[count];
        } else if (strcmp(param, "output") == 0) {
          output_file_name = argv[count];
        }
        
        ++count;
      } else {
        print_error_and_exit((std::string)"missing parameter after " + argv[count-1]);
      }
    } else {
      print_error_and_exit((std::string)"missing type of parameter " + argv[count-1]);
    }
  }

  if (output_file_name == 0) {
    print_error_and_exit("missing output file name");
  }
  if (input_height_file_name)           std::cerr << "input height file name    " << input_height_file_name << std::endl;
  if (input_color_file_name)            std::cerr << "input color file name     " << input_color_file_name << std::endl;
  if (input_buildings_file_name)        std::cerr << "input buildings file name " << input_buildings_file_name << std::endl;
  if (output_file_name)                 std::cerr << "output_file_name          " << output_file_name << std::endl;

  build_data(input_height_file_name,
             input_color_file_name,
             input_buildings_file_name,
             output_file_name,
             u_extent,
             v_extent,
             height_ds,
             color_ds,
             patch_dim,
             subsampling_level,
             height_scale_factor);
  
  return 0;
}
