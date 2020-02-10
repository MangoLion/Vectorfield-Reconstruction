#include <stdio.h>
#include <iostream>
#include<fstream>
// yes, I know stdio.h is not good C++, but I like the *printf()
#include <stdlib.h>
#include <ctype.h>
#include <sstream>
#include <cmath>

#define _USE_MATH_DEFINES
#include <math.h>

#ifdef WIN32
#include <windows.h>
#pragma warning(disable:4996)
#endif


// You need to adjust the location of these header files according to your configuration

#include <windows.h>
//#include <freeglut.h>
#include "GL\include\GL.H"
#include "GL\include\glu.h"
#include "GL\include\glut.h"
#include "GL\include\glui.h"

#include "Skeleton.h"
#include <time.h>
//#include "glui.h"

#include "structures.h"
#include "Reader.h"
#include "Graph.h"
#include "LIC.h"

#include "GUI.h"
//
//
//	This is a sample OpenGL / GLUT / GLUI program
//
//	The objective is to draw a 3d object and change the color of the axes
//		with radio buttons
//
//	The left mouse button allows rotation
//	The middle mouse button allows scaling
//	The glui window allows:
//		1. The 3d object to be transformed
//		2. The projection to be changed
//		3. The color of the axes to be changed
//		4. The axes to be turned on and off
//		5. The transformations to be reset
//		6. The program to quit
//
//	Author: Joe Graphics
//


 node_main** grid_main = new node_main * [50];
 std::vector<std::vector<node>> streamlines;
 std::vector<node*> bins[5][5];
 std::vector<segment*> segment_starters;
 int bin_width_segment = 2,
	 bin_num_segment = 2 * 50;
 std::vector<segment*> bins_segment[2][2];

 const int IMG_RES = 512;
 unsigned char noise_tex[IMG_RES][IMG_RES][3];
 unsigned char vec_img[IMG_RES][IMG_RES][3];
 float data_img[50][50][3];
 bool data_check[50][50];
 unsigned char LIC_tex[IMG_RES][IMG_RES][3];
 unsigned char LIC_tex_backup[IMG_RES][IMG_RES][3];

float mag_max = -1000000, mag_min = 1000000, vx_min = 1000000, vx_max = -1000000,
vy_min = 1000000, vy_max = -1000000, angle_min = 0, angle_max = M_PI, 
x_max = -100000, x_min = 1000000, y_max = -100000, y_min = 1000000;

float max_err_mag_sl = -100000, min_err_mag_sl = 100000,
max_err_mag_offset = -100000, min_err_mag_offset = 1000000,
max_err_dir_sl = -100000, min_err_dir_sl = 1000000,
max_err_dir_offset = -100000, min_err_dir_offset = 1000000,
max_err_vx_sl = -100000, min_err_vx_sl = 100000,
max_err_vy_sl = -100000, min_err_vy_sl = 100000,
max_err_vx_offset = -100000, min_err_vx_offset = 100000,
max_err_vy_offset = -100000, min_err_vy_offset = 100000,
max_err_vxy_sl = -100000, min_err_vxy_sl = 100000;

float err_min = 10000, err_max = -100000, err_avg = 0;

Polyhedron* poly = NULL;
int do_draw_arrows = 0, vis_option = 0, do_draw_streamlines = 0;

int GRAPH_RESOLUTION = 1;

int main(int argc, char* argv[])
{
	// turn on the glut package:
	// (do this before checking argc and argv since it might
	// pull some command line arguments out)

	glutInit(&argc, argv);

	// Load the model and data here
	FILE* this_file = fopen("../models/bnoise.ply", "r");
	read_streamline("bnoise_sl.txt");
	poly = new Polyhedron(this_file);
	fclose(this_file);
	//mat_ident( rotmat );	

	poly->initialize(); // initialize everything

	poly->calc_bounding_sphere();
	poly->calc_face_normals_and_area();
	poly->average_normals();



	// setup all the graphics stuff:

	InitGraphics();


	// create the display structures that will not change:

	InitLists();


	// init all the global variables used by Display():
	// this will also post a redisplay
	// it is important to call this before InitGlui()
	// so that the variables that glui will control are correct
	// when each glui widget is created

	Reset();


	// setup all the user interface stuff:

	InitGlui();

	gen_noise_tex();
	calculate_bounds(poly);
	compute_LIC(0);

	// draw the scene once and wait for some interaction:
	// (will never return)

	glutMainLoop();

	// finalize the object if loaded

	//if (poly != NULL)
	//	poly->finalize();

	// this is here to make the compiler happy:

	return 0;
}
