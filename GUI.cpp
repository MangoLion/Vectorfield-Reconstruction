

#include <stdio.h>
#include <iostream>
#include<fstream>
// yes, I know stdio.h is not good C++, but I like the *printf()
#include <stdlib.h>
#include <ctype.h>
#include <sstream>
#include <cmath>

#include <windows.h>
//#include <freeglut.h>
#include "GL\include\GL.H"
#include "GL\include\glu.h"
#include "GL\include\glut.h"
#include "GL\include\glui.h"

#include "Reader.h"
#include "GUI.h"
#include "LIC.h"
#include "Skeleton.h"
#include "structures.h"
#include "globals.h"
#include <time.h>

#define _USE_MATH_DEFINES
#include <math.h>

// This source code has been modified by Guoning Chen since its release


// title of these windows:

const char* WINDOWTITLE = { "OpenGL / GLUT / GLUI Sample -- Nguyen Phan" };
const char* GLUITITLE = { "User Interface Window" };


// what the glui package defines as true and false:

const int GLUITRUE = { true };
const int GLUIFALSE = { false };


// the escape key:

#define ESCAPE		0x1b


// initial window size:

const int INIT_WINDOW_SIZE = { 512 };


// size of the box:

const float BOXSIZE = { 2.f };



// multiplication factors for input interaction:
//  (these are known from previous experience)

const float ANGFACT = { 1. };
const float SCLFACT = { 0.005f };


// able to use the left mouse for either rotation or scaling,
// in case have only a 2-button mouse:

enum LeftButton
{
	ROTATE,
	SCALE
};


// minimum allowable scale factor:

const float MINSCALE = { 0.05f };


// active mouse buttons (or them together):

const int LEFT = { 4 };
const int MIDDLE = { 2 };
const int RIGHT = { 1 };


// which projection:

enum Projections
{
	ORTHO,
	PERSP
};


// which button:

enum ButtonVals
{
	RESET,
	QUIT
};


// window background color (rgba):

const float BACKCOLOR[] = { 0., 0., 0., 0. };


// line width for the axes:

const GLfloat AXES_WIDTH = { 3. };


// the color numbers:
// this order must match the radio button order

enum Colors
{
	RED,
	YELLOW,
	GREEN,
	CYAN,
	BLUE,
	MAGENTA,
	WHITE,
	BLACK
};


// the object numbers:
// 
enum MODELS
{
	BUNNY,
	FELINE,
	DRAGON,
	HAPPY,
	SPHERE,
	TORUS,
};

// the color definitions:
// this order must match the radio button order

const GLfloat Colors[8][3] =
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};


const float discerete_colors[8][3] =
{
	{ 1., 0., 0. },		// red
	{ 1., 1., 0. },		// yellow
	{ 0., 1., 0. },		// green
	{ 0., 1., 1. },		// cyan
	{ 0., 0., 1. },		// blue
	{ 1., 0., 1. },		// magenta
	{ 1., 1., 1. },		// white
	{ 0., 0., 0. },		// black
};


// fog parameters:

const GLfloat FOGCOLOR[4] = { .0, .0, .0, 1. };
const GLenum  FOGMODE = { GL_LINEAR };
const GLfloat FOGDENSITY = { 0.30f };
const GLfloat FOGSTART = { 1.5 };
const GLfloat FOGEND = { 4. };

//
// non-constant global variables:
//

int	ActiveButton;		// current button that is down
GLuint	AxesList;		// list to hold the axes
int	AxesOn;			// != 0 means to draw the axes
int	DebugOn;			// != 0 means to print debugging info
int	DepthCueOn;		// != 0 means to use intensity depth cueing
GLUI* Glui;			// instance of glui window
int	GluiWindow;		// the glut id for the glui window
int	LeftButton;		// either ROTATE or SCALE
GLuint	BoxList;		// object display list
int	MainWindow;		// window id for main graphics window
GLfloat	RotMatrix[4][4];	// set by glui rotation widget
float	Scale, Scale2;		// scaling factors
int	WhichColor;		// index into Colors[]
int	WhichProjection;	// ORTHO or PERSP
int	Xmouse, Ymouse;		// mouse values
float	Xrot, Yrot;		// rotation angles in degrees
float	TransXYZ[3];		// set by glui translation widgets

double radius_factor = 0.9;
int display_mode = 0;

int ObjectId = 0;
char object_name[128] = "dipole";



//
// function prototypes:
//

void	Animate(void);
void	Buttons(int);
void	Display(void);
void	DoRasterString(float, float, float, char*);
void	DoStrokeString(float, float, float, float, char*);
float	ElapsedSeconds(void);
void	InitGlui(void);
void	InitGraphics(void);
void	InitLists(void);
void	Keyboard(unsigned char, int, int);
void	MouseButton(int, int, int, int);
void	MouseMotion(int, int);
void	Reset(void);
void	Resize(int, int);
void	Visibility(int);

void	Arrow(float[3], float[3]);
void	Cross(float[3], float[3], float[3]);
float	Dot(float[3], float[3]);
float	Unit(float[3], float[3]);
void	Axes(float);
void	HsvRgb(float[3], float[3]);

void    set_view(GLenum mode, Polyhedron* poly);
void    set_scene(GLenum mode, Polyhedron* poly);
void    Choose_Object();


//
// this is where one would put code that is to be called
// everytime the glut main loop has nothing to do
//
// this is typically where animation parameters are set
//
// do not call Display() from here -- let glutMainLoop() do it
//


void Animate(void)
{
	// put animation stuff in here -- change some global variables
	// for Display() to find:



	// force a call to Display() next time it is convenient:

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}




//
// glui buttons callback:
//

void
Buttons(int id)
{
	switch (id)
	{
	case RESET:
		Reset();
		Glui->sync_live();
		glutSetWindow(MainWindow);
		glutPostRedisplay();
		break;

	case QUIT:
		// gracefully close the glui window:
		// gracefully close out the graphic
		// gracefully close the graphics window:
		// gracefully exit the program:

		Glui->close();
		glutSetWindow(MainWindow);
		glFinish();
		glutDestroyWindow(MainWindow);
		exit(0);
		break;

	default:
		fprintf(stderr, "Don't know what to do with Button ID %d\n", id);
	}

}

//
// use glut to display a string of characters using a raster font:
//

void
DoRasterString(float x, float y, float z, char* s)
{
	char c;			// one character to print

	glRasterPos3f((GLfloat)x, (GLfloat)y, (GLfloat)z);
	for (; (c = *s) != '\0'; s++)
	{
		glutBitmapCharacter(GLUT_BITMAP_TIMES_ROMAN_24, c);
	}
}



//
// use glut to display a string of characters using a stroke font:
//

void
DoStrokeString(float x, float y, float z, float ht, char* s)
{
	char c;			// one character to print
	float sf;		// the scale factor

	glPushMatrix();
	glTranslatef((GLfloat)x, (GLfloat)y, (GLfloat)z);
	sf = ht / (119.05 + 33.33);
	glScalef((GLfloat)sf, (GLfloat)sf, (GLfloat)sf);
	for (; (c = *s) != '\0'; s++)
	{
		glutStrokeCharacter(GLUT_STROKE_ROMAN, c);
	}
	glPopMatrix();
}



//
// return the number of seconds since the start of the program:
//

float
ElapsedSeconds(void)
{
	// get # of milliseconds since the start of the program:

	int ms = glutGet(GLUT_ELAPSED_TIME);

	// convert it to seconds:

	return (float)ms / 1000.;
}

float s_middle = 0.5;
void set_bwr(int c);
void set_slice(int c);
void update_range(int c);
int do_enhance = 0;
//
// initialize the glui window:
//

//GLUI_EditText* tfmin;
//GLUI_EditText* tfmax;
//GLUI_EditText* tfsstart;
//GLUI_EditText* tfssend;


GLUI_EditText* tfbwr;

GLUI_Spinner* tfBWR, * tfMin, * tfMax, * tfsstart, * tfssend, * tfAlpha, * tfGridX, * tfGridY, * tfGridOffsetX, * tfGridOffsetY, * tfRadarRange,
* tfSegment;

int segment_current = 0;

int gridX = 1, gridY = 1, radar_range;

float range, s_min, s_max, grid_offset_x = 0.3, grid_offset_y = 0.3;



void update_vis(int i) {
	if (vis_option == 0) {
		//Choose_Object();
	}
}

float alpha = 0.5;
float slice_start = 0;
float slice_end = 0.1;

void update_enhance(int i);


int whichcolorscheme, err_type = 0;

char buffer[sizeof(GLUI_String)];
char buffer2[sizeof(GLUI_String)];
char buffer3[sizeof(GLUI_String)];

char buffer4[sizeof(GLUI_String)];
char buffer5[sizeof(GLUI_String)];


void interpolate(int n) {
};

void InitGlui(void)
{
	GLUI_Panel* panel;
	GLUI_RadioGroup* group, * group2;
	GLUI_Rotation* rot;
	GLUI_Translation* trans, * scale;


	// setup the glui window:

	glutInitWindowPosition(INIT_WINDOW_SIZE + 50, 0);
	Glui = GLUI_Master.create_glui((char*)GLUITITLE);


	Glui->add_statictext((char*)GLUITITLE);
	Glui->add_separator();

	//Glui->add_checkbox("Axes", &AxesOn);

	//Glui->add_checkbox("Perspective", &WhichProjection);

	//Glui->add_checkbox("Intensity Depth Cue", &DepthCueOn);

	// Add a rollout for the axes color

	Glui->add_checkbox("Draw Streamlines", &do_draw_streamlines);

	GLUI_Rollout* rollout = Glui->add_rollout(" Axes Color ", 0);

	//panel = Glui->add_panel(  "Axes Color" );
	//GLUI_Rollout *rollout = Glui->add_rollout_to_panel(panel,  "Axes Color", 1 );
		//group = Glui->add_radiogroup_to_panel( panel, &WhichColor );
	/*
	group = Glui->add_radiogroup_to_panel(rollout, &WhichColor);
	Glui->add_radiobutton_to_group(group, "Red");
	Glui->add_radiobutton_to_group(group, "Yellow");
	Glui->add_radiobutton_to_group(group, "Green");
	Glui->add_radiobutton_to_group(group, "Cyan");
	Glui->add_radiobutton_to_group(group, "Blue");
	Glui->add_radiobutton_to_group(group, "Magenta");
	Glui->add_radiobutton_to_group(group, "White");
	Glui->add_radiobutton_to_group(group, "Black");*/

	GLUI_Rollout* rollout2 = Glui->add_rollout(" Err Type ", 0);
	group2 = Glui->add_radiogroup_to_panel(rollout2, &err_type);
	Glui->add_radiobutton_to_group(group2, "Original Grid");
	Glui->add_radiobutton_to_group(group2, "Offset Grid");

	rollout2 = Glui->add_rollout(" Color Scheme ", 0);
	group2 = Glui->add_radiogroup_to_panel(rollout2, &whichcolorscheme);
	Glui->add_radiobutton_to_group(group2, "Rainbow");
	Glui->add_radiobutton_to_group(group2, "BWR");
	Glui->add_radiobutton_to_group(group2, "Heatmap");
	Glui->add_radiobutton_to_group(group2, "discrete");
	Glui->add_radiobutton_to_group(group2, "slice");

	rollout2 = Glui->add_rollout(" Visualize Type ", 0);

	//panel = Glui->add_panel(  "Axes Color" );
	//GLUI_Rollout *rollout = Glui->add_rollout_to_panel(panel,  "Axes Color", 1 );
		//group = Glui->add_radiogroup_to_panel( panel, &WhichColor );

	group2 = Glui->add_radiogroup_to_panel(rollout2, &vis_option, -1, &update_vis);
	Glui->add_radiobutton_to_group(group2, "None");
	Glui->add_radiobutton_to_group(group2, "Magnitude");
	Glui->add_radiobutton_to_group(group2, "Angle");
	Glui->add_radiobutton_to_group(group2, "X component");
	Glui->add_radiobutton_to_group(group2, "Y component");
	Glui->add_radiobutton_to_group(group2, "XY component");

	Glui->add_checkbox("Show Arrows", &do_draw_arrows);
	Glui->add_checkbox("Enhance!", &do_enhance, -1, &update_enhance);
	//tfAlpha = Glui->add_spinner("Color Vis Alpha:", GLUI_SPINNER_FLOAT, &alpha, -1,update_range);
	//tfAlpha->set_float_limits(0, 1);

	tfBWR = Glui->add_spinner("Color Vis Alpha:", GLUI_SPINNER_FLOAT, &alpha, -1, set_bwr);
	tfBWR->set_float_limits(0, 1);

	tfBWR = Glui->add_spinner("BWR Threshold (0-1):", GLUI_SPINNER_FLOAT, &s_middle, -1, set_bwr);
	tfBWR->set_float_limits(0, 1);

	tfBWR = Glui->add_spinner("Segment Index:", GLUI_SPINNER_INT, &segment_current, -1, update_range);


	/*
	tfmin =
		Glui->add_edittext("Range min:", GLUI_EDITTEXT_FLOAT, buffer, -1, set_range);
	//tfmin->set_float_val(0);

	tfmax =
		Glui->add_edittext("Range max:", GLUI_EDITTEXT_FLOAT, buffer2, -1, set_range);
	//tfmax->set_float_val(0);
	Glui->add_button("Set Range",	0, set_range);

	*/
	/*tfbwr =
		Glui->add_edittext("BWR threshold(0-1):", GLUI_EDITTEXT_FLOAT, buffer3);
	tfbwr->set_float_val(0.5);
	Glui->add_button("Set BWR threshold", 0, set_bwr);
	*/

	/*
	tfMin = Glui->add_spinner("Min:", GLUI_SPINNER_FLOAT, &s_min, -1, update_range);


	tfMax = Glui->add_spinner("Max:", GLUI_SPINNER_FLOAT, &s_max, -1, update_range);

	*/
	//tfmax->set_float_val(0);
	//Glui->add_button("Set slices Range", 0, set_slice);

	//tfssend->set_float_val(0.1);
	// Add a list for the different models
	//rollout = Glui->add_rollout(" Models ", 0);
	panel = Glui->add_panel("Choose object to open ");
	GLUI_Listbox* obj_list = Glui->add_listbox_to_panel(panel, "Objects", &ObjectId, -1, (GLUI_Update_CB)Choose_Object);
	obj_list->add_item(0, "dipole");
	obj_list->add_item(1, "bnoise");
	obj_list->add_item(2, "bruno3");
	obj_list->add_item(3, "cnoise");
	obj_list->add_item(4, "vnoise");
	//obj_list->add_item (5, "torus");

	tfMax = Glui->add_spinner("Max:", GLUI_SPINNER_FLOAT, &s_max, -1, update_range);
	/*
	tfGridX = Glui->add_spinner("Grid cell width:", GLUI_EDITTEXT_INT, &gridX, -1);
	tfGridX->set_int_val(25);
	tfGridY = Glui->add_spinner("Grid cell height:", GLUI_EDITTEXT_INT, &gridY, -1);
	tfGridY->set_int_val(25);*/
	/*tfGridOffsetX = Glui->add_spinner("Offset Grid X:", GLUI_EDITTEXT_INT, &grid_offset_x, -1);
	tfGridOffsetX->set_int_val(3);
	tfGridOffsetY = Glui->add_spinner("Offset Grid Y:", GLUI_EDITTEXT_INT, &grid_offset_y, -1);
	tfGridOffsetY->set_int_val(3);*/
	tfRadarRange = Glui->add_spinner("Radar Range:", GLUI_EDITTEXT_FLOAT, &radar_range, -1);
	tfRadarRange->set_float_val(30);

	Glui->add_button("Interpolate!", 0, interpolate);


	panel = Glui->add_panel("Object Transformation");

	rot = Glui->add_rotation_to_panel(panel, "Rotation", (float*)RotMatrix);

	// allow the object to be spun via the glui rotation widget:

	rot->set_spin(1.0);


	Glui->add_column_to_panel(panel, GLUIFALSE);
	scale = Glui->add_translation_to_panel(panel, "Scale", GLUI_TRANSLATION_Y, &Scale2);
	scale->set_speed(0.005f);

	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Trans XY", GLUI_TRANSLATION_XY, &TransXYZ[0]);
	trans->set_speed(0.05f);

	Glui->add_column_to_panel(panel, GLUIFALSE);
	trans = Glui->add_translation_to_panel(panel, "Trans Z", GLUI_TRANSLATION_Z, &TransXYZ[2]);
	trans->set_speed(0.05f);

	Glui->add_checkbox("Debug", &DebugOn);


	panel = Glui->add_panel("", GLUIFALSE);

	Glui->add_button_to_panel(panel, "Reset", RESET, (GLUI_Update_CB)Buttons);

	Glui->add_column_to_panel(panel, GLUIFALSE);

	Glui->add_button_to_panel(panel, "Quit", QUIT, (GLUI_Update_CB)Buttons);


	// tell glui what graphics window it needs to post a redisplay to:

	Glui->set_main_gfx_window(MainWindow);


	// set the graphics window's idle function:

	GLUI_Master.set_glutIdleFunc(NULL);
}



//
// initialize the glut and OpenGL libraries:
//	also setup display lists and callback functions
//


void InitGraphics(void)
{
	// setup the display mode:
	// ( *must* be done before call to glutCreateWindow() )
	// ask for color, double-buffering, and z-buffering:

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);


	// set the initial window configuration:

	glutInitWindowPosition(0, 0);
	glutInitWindowSize(INIT_WINDOW_SIZE, INIT_WINDOW_SIZE);


	// open the window and set its title:

	MainWindow = glutCreateWindow(WINDOWTITLE);
	glutSetWindowTitle(WINDOWTITLE);


	// setup the clear values:

	glClearColor(BACKCOLOR[0], BACKCOLOR[1], BACKCOLOR[2], BACKCOLOR[3]);


	// setup the callback routines:


	// DisplayFunc -- redraw the window
	// ReshapeFunc -- handle the user resizing the window
	// KeyboardFunc -- handle a keyboard input
	// MouseFunc -- handle the mouse button going down or up
	// MotionFunc -- handle the mouse moving with a button down
	// PassiveMotionFunc -- handle the mouse moving with a button up
	// VisibilityFunc -- handle a change in window visibility
	// EntryFunc	-- handle the cursor entering or leaving the window
	// SpecialFunc -- handle special keys on the keyboard
	// SpaceballMotionFunc -- handle spaceball translation
	// SpaceballRotateFunc -- handle spaceball rotation
	// SpaceballButtonFunc -- handle spaceball button hits
	// ButtonBoxFunc -- handle button box hits
	// DialsFunc -- handle dial rotations
	// TabletMotionFunc -- handle digitizing tablet motion
	// TabletButtonFunc -- handle digitizing tablet button hits
	// MenuStateFunc -- declare when a pop-up menu is in use
	// TimerFunc -- trigger something to happen a certain time from now
	// IdleFunc -- what to do when nothing else is going on
	glutSetWindow(MainWindow);
	glutDisplayFunc(Display);
	//glutDisplayFunc( Display_Model );
	glutReshapeFunc(Resize);
	glutKeyboardFunc(Keyboard);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(NULL);
	glutVisibilityFunc(Visibility);
	glutEntryFunc(NULL);
	glutSpecialFunc(NULL);
	glutSpaceballMotionFunc(NULL);
	glutSpaceballRotateFunc(NULL);
	glutSpaceballButtonFunc(NULL);
	glutButtonBoxFunc(NULL);
	glutDialsFunc(NULL);
	glutTabletMotionFunc(NULL);
	glutTabletButtonFunc(NULL);
	glutMenuStateFunc(NULL);
	glutTimerFunc(0, NULL, 0);

	// DO NOT SET THE GLUT IDLE FUNCTION HERE !!
	// glutIdleFunc( NULL );
	// let glui take care of it in InitGlui()
}




//
// initialize the display lists that will not change:
//


void InitLists(void)
{
	float dx = BOXSIZE / 2.;
	float dy = BOXSIZE / 2.;
	float dz = BOXSIZE / 2.;

	// create the object:

	BoxList = glGenLists(1);
	glNewList(BoxList, GL_COMPILE);

	glBegin(GL_QUADS);

	glColor3f(0., 0., 1.);
	glNormal3f(0., 0., 1.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(dx, -dy, dz);
	glVertex3f(dx, dy, dz);
	glVertex3f(-dx, dy, dz);

	glNormal3f(0., 0., -1.);
	glTexCoord2f(0., 0.);
	glVertex3f(-dx, -dy, -dz);
	glTexCoord2f(0., 1.);
	glVertex3f(-dx, dy, -dz);
	glTexCoord2f(1., 1.);
	glVertex3f(dx, dy, -dz);
	glTexCoord2f(1., 0.);
	glVertex3f(dx, -dy, -dz);

	glColor3f(1., 0., 0.);
	glNormal3f(1., 0., 0.);
	glVertex3f(dx, -dy, dz);
	glVertex3f(dx, -dy, -dz);
	glVertex3f(dx, dy, -dz);
	glVertex3f(dx, dy, dz);

	glNormal3f(-1., 0., 0.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(-dx, dy, dz);
	glVertex3f(-dx, dy, -dz);
	glVertex3f(-dx, -dy, -dz);

	glColor3f(0., 1., 0.);
	glNormal3f(0., 1., 0.);
	glVertex3f(-dx, dy, dz);
	glVertex3f(dx, dy, dz);
	glVertex3f(dx, dy, -dz);
	glVertex3f(-dx, dy, -dz);

	glNormal3f(0., -1., 0.);
	glVertex3f(-dx, -dy, dz);
	glVertex3f(-dx, -dy, -dz);
	glVertex3f(dx, -dy, -dz);
	glVertex3f(dx, -dy, dz);

	glEnd();

	glEndList();


	// create the axes:

	AxesList = glGenLists(1);
	glNewList(AxesList, GL_COMPILE);
	glLineWidth(AXES_WIDTH);
	Axes(1.5);
	glLineWidth(1.);
	glEndList();
}



//
// the keyboard callback:
//


void Keyboard(unsigned char c, int x, int y)
{
	if (DebugOn != 0)
		fprintf(stderr, "Keyboard: '%c' (0x%0x)\n", c, c);

	switch (c)
	{
	case 'o':
	case 'O':
		WhichProjection = ORTHO;
		break;

	case 'p':
	case 'P':
		WhichProjection = PERSP;
		break;

	case 'q':
	case 'Q':
	case ESCAPE:
		Buttons(QUIT);	// will not return here
		break;			// happy compiler

	case 'r':
	case 'R':
		LeftButton = ROTATE;
		break;

	case 's':
	case 'S':
		LeftButton = SCALE;
		break;

	default:
		fprintf(stderr, "Don't know what to do with keyboard hit: '%c' (0x%0x)\n", c, c);
	}


	// synchronize the GLUI display with the variables:

	Glui->sync_live();


	// force a call to Display():

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}



//
// called when the mouse button transitions down or up:
//

void MouseButton(int button, int state, int x, int y)
{
	int b;			// LEFT, MIDDLE, or RIGHT

	if (DebugOn != 0)
		fprintf(stderr, "MouseButton: %d, %d, %d, %d\n", button, state, x, y);


	// get the proper button bit mask:

	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		b = LEFT;		break;

	case GLUT_MIDDLE_BUTTON:
		b = MIDDLE;		break;

	case GLUT_RIGHT_BUTTON:
		b = RIGHT;		break;

	default:
		b = 0;
		fprintf(stderr, "Unknown mouse button: %d\n", button);
	}


	// button down sets the bit, up clears the bit:

	if (state == GLUT_DOWN)
	{
		Xmouse = x;
		Ymouse = y;
		ActiveButton |= b;		// set the proper bit
	}
	else
	{
		ActiveButton &= ~b;		// clear the proper bit
	}
}



//
// called when the mouse moves while a button is down:
//


void MouseMotion(int x, int y)
{
	int dx, dy;		// change in mouse coordinates

	if (DebugOn != 0)
		fprintf(stderr, "MouseMotion: %d, %d\n", x, y);


	dx = x - Xmouse;		// change in mouse coords
	dy = y - Ymouse;

	if ((ActiveButton & LEFT) != 0)
	{
		switch (LeftButton)
		{
		case ROTATE:
			Xrot += (ANGFACT * dy);
			Yrot += (ANGFACT * dx);
			break;

		case SCALE:
			Scale += SCLFACT * (float)(dx - dy);
			if (Scale < MINSCALE)
				Scale = MINSCALE;
			break;
		}
	}


	if ((ActiveButton & MIDDLE) != 0)
	{
		Scale += SCLFACT * (float)(dx - dy);

		// keep object from turning inside-out or disappearing:

		if (Scale < MINSCALE)
			Scale = MINSCALE;
	}

	Xmouse = x;			// new current position
	Ymouse = y;

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}



//
// reset the transformations and the colors:
//
// this only sets the global variables --
// the glut main loop is responsible for redrawing the scene
//


void Reset(void)
{
	ActiveButton = 0;
	AxesOn = GLUITRUE;
	DebugOn = GLUIFALSE;
	DepthCueOn = GLUIFALSE;
	LeftButton = ROTATE;
	Scale = 1.0;
	Scale2 = 0.0;		// because we add 1. to it in Display()
	WhichColor = BLUE;
	WhichProjection = PERSP;
	Xrot = Yrot = 0.;
	TransXYZ[0] = TransXYZ[1] = TransXYZ[2] = 0.;

	RotMatrix[0][1] = RotMatrix[0][2] = RotMatrix[0][3] = 0.;
	RotMatrix[1][0] = RotMatrix[1][2] = RotMatrix[1][3] = 0.;
	RotMatrix[2][0] = RotMatrix[2][1] = RotMatrix[2][3] = 0.;
	RotMatrix[3][0] = RotMatrix[3][1] = RotMatrix[3][3] = 0.;
	RotMatrix[0][0] = RotMatrix[1][1] = RotMatrix[2][2] = RotMatrix[3][3] = 1.;
}



//
// called when user resizes the window:
//

void
Resize(int width, int height)
{
	if (DebugOn != 0)
		fprintf(stderr, "ReSize: %d, %d\n", width, height);

	// don't really need to do anything since window size is
	// checked each time in Display():

	glutSetWindow(MainWindow);
	glutPostRedisplay();
}


//
// handle a change to the window's visibility:
//

void
Visibility(int state)
{
	if (DebugOn != 0)
		fprintf(stderr, "Visibility: %d\n", state);

	if (state == GLUT_VISIBLE)
	{
		glutSetWindow(MainWindow);
		glutPostRedisplay();
	}
	else
	{
		// could optimize by keeping track of the fact
		// that the window is not visible and avoid
		// animating or redrawing it ...
	}
}




//////////////////////////////////////////  EXTRA HANDY UTILITIES:  /////////////////////////////

// size of wings as fraction of length:

#define WINGS	0.10


// axes:

#define X	1
#define Y	2
#define Z	3


// x, y, z, axes:

static float axx[3] = { 1., 0., 0. };
static float ayy[3] = { 0., 1., 0. };
static float azz[3] = { 0., 0., 1. };


void
Arrow(float tail[3], float head[3])
{
	float u[3], v[3], w[3];		// arrow coordinate system
	float d;			// wing distance
	float x, y, z;			// point to plot
	float mag;			// magnitude of major direction
	float f;			// fabs of magnitude
	int axis;			// which axis is the major


	// set w direction in u-v-w coordinate system:

	w[0] = head[0] - tail[0];
	w[1] = head[1] - tail[1];
	w[2] = head[2] - tail[2];


	// determine major direction:

	axis = X;
	mag = fabs(w[0]);
	if ((f = fabs(w[1])) > mag)
	{
		axis = Y;
		mag = f;
	}
	if ((f = fabs(w[2])) > mag)
	{
		axis = Z;
		mag = f;
	}


	// set size of wings and turn w into a Unit vector:

	d = WINGS * Unit(w, w);


	// draw the shaft of the arrow:

	glBegin(GL_LINE_STRIP);
	glVertex3fv(tail);
	glVertex3fv(head);
	glEnd();

	// draw two sets of wings in the non-major directions:

	if (axis != X)
	{
		Cross(w, axx, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}


	if (axis != Y)
	{
		Cross(w, ayy, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}



	if (axis != Z)
	{
		Cross(w, azz, v);
		(void)Unit(v, v);
		Cross(v, w, u);
		x = head[0] + d * (u[0] - w[0]);
		y = head[1] + d * (u[1] - w[1]);
		z = head[2] + d * (u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
		x = head[0] + d * (-u[0] - w[0]);
		y = head[1] + d * (-u[1] - w[1]);
		z = head[2] + d * (-u[2] - w[2]);
		glBegin(GL_LINE_STRIP);
		glVertex3fv(head);
		glVertex3f(x, y, z);
		glEnd();
	}
}



float
Dot(float v1[3], float v2[3])
{
	return v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2];
}



void
Cross(float v1[3], float v2[3], float vout[3])
{
	float tmp[3];

	tmp[0] = v1[1] * v2[2] - v2[1] * v1[2];
	tmp[1] = v2[0] * v1[2] - v1[0] * v2[2];
	tmp[2] = v1[0] * v2[1] - v2[0] * v1[1];

	vout[0] = tmp[0];
	vout[1] = tmp[1];
	vout[2] = tmp[2];
}



float
Unit(float vin[3], float vout[3])
{
	float dist, f;

	dist = vin[0] * vin[0] + vin[1] * vin[1] + vin[2] * vin[2];

	if (dist > 0.0)
	{
		dist = sqrt(dist);
		f = 1. / dist;
		vout[0] = f * vin[0];
		vout[1] = f * vin[1];
		vout[2] = f * vin[2];
	}
	else
	{
		vout[0] = vin[0];
		vout[1] = vin[1];
		vout[2] = vin[2];
	}

	return dist;
}



// the stroke characters 'X' 'Y' 'Z' :

static float xx[] = {
		0.f, 1.f, 0.f, 1.f
};

static float xy[] = {
		-.5f, .5f, .5f, -.5f
};

static int xorder[] = {
		1, 2, -3, 4
};


static float yx[] = {
		0.f, 0.f, -.5f, .5f
};

static float yy[] = {
		0.f, .6f, 1.f, 1.f
};

static int yorder[] = {
		1, 2, 3, -2, 4
};


static float zx[] = {
		1.f, 0.f, 1.f, 0.f, .25f, .75f
};

static float zy[] = {
		.5f, .5f, -.5f, -.5f, 0.f, 0.f
};

static int zorder[] = {
		1, 2, 3, 4, -5, 6
};


// fraction of the length to use as height of the characters:

const float LENFRAC = 0.10f;


// fraction of length to use as start location of the characters:

const float BASEFRAC = 1.10f;


//
//	Draw a set of 3D axes:
//	(length is the axis length in world coordinates)
//

void Axes(float length)
{
	int i, j;			// counters
	float fact;			// character scale factor
	float base;			// character start location


	glBegin(GL_LINE_STRIP);
	glVertex3f(length, 0., 0.);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., length, 0.);
	glEnd();
	glBegin(GL_LINE_STRIP);
	glVertex3f(0., 0., 0.);
	glVertex3f(0., 0., length);
	glEnd();

	fact = LENFRAC * length;
	base = BASEFRAC * length;

	glBegin(GL_LINE_STRIP);
	for (i = 0; i < 4; i++)
	{
		j = xorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(base + fact * xx[j], fact * xy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (i = 0; i < 5; i++)
	{
		j = yorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(fact * yx[j], base + fact * yy[j], 0.0);
	}
	glEnd();

	glBegin(GL_LINE_STRIP);
	for (i = 0; i < 6; i++)
	{
		j = zorder[i];
		if (j < 0)
		{

			glEnd();
			glBegin(GL_LINE_STRIP);
			j = -j;
		}
		j--;
		glVertex3f(0.0, fact * zy[j], base + fact * zx[j]);
	}
	glEnd();

}


//
// routine to convert HSV to RGB
//
// Reference:  Foley, van Dam, Feiner, Hughes,
//		"Computer Graphics Principles and Practices,"
//		Additon-Wesley, 1990, pp592-593.


void HsvRgb(float hsv[3], float rgb[3])
{
	float h, s, v;			// hue, sat, value
	float r = 0, g = 0, b = 0;			// red, green, blue
	float i, f, p, q, t;		// interim values


	// guarantee valid input:

	h = hsv[0] / 60.;
	while (h >= 6.)	h -= 6.;
	while (h < 0.) 	h += 6.;

	s = hsv[1];
	if (s < 0.)
		s = 0.;
	if (s > 1.)
		s = 1.;

	v = hsv[2];
	if (v < 0.)
		v = 0.;
	if (v > 1.)
		v = 1.;


	// if sat==0, then is a gray:

	if (s == 0.0)
	{
		rgb[0] = rgb[1] = rgb[2] = v;
		return;
	}


	// get an rgb from the hue itself:

	i = floor(h);
	f = h - i;
	p = v * (1. - s);
	q = v * (1. - s * f);
	t = v * (1. - (s * (1. - f)));

	switch ((int)i)
	{
	case 0:
		r = v;	g = t;	b = p;
		break;

	case 1:
		r = q;	g = v;	b = p;
		break;

	case 2:
		r = p;	g = v;	b = t;
		break;

	case 3:
		r = p;	g = q;	b = v;
		break;

	case 4:
		r = t;	g = p;	b = v;
		break;

	case 5:
		r = v;	g = p;	b = q;
		break;
	}


	rgb[0] = r;
	rgb[1] = g;
	rgb[2] = b;
}

// Set the view and the lighting properties
//

void set_view(GLenum mode, Polyhedron* poly)
{
	icVector3 up, ray, view;
	GLfloat light_ambient0[] = { 0.3, 0.3, 0.3, 1.0 };
	GLfloat light_diffuse0[] = { 0.7, 0.7, 0.7, 1.0 };
	GLfloat light_specular0[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_ambient1[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_diffuse1[] = { 0.5, 0.5, 0.5, 1.0 };
	GLfloat light_specular1[] = { 0.0, 0.0, 0.0, 1.0 };
	GLfloat light_ambient2[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_diffuse2[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_specular2[] = { 1.0, 1.0, 1.0, 1.0 };
	GLfloat light_position[] = { 0.0, 0.0, 0.0, 1.0 };

	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse0);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular0);
	glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular1);


	glMatrixMode(GL_PROJECTION);
	if (mode == GL_RENDER)
		glLoadIdentity();

	if (WhichProjection == ORTHO)
		glOrtho(-radius_factor, radius_factor, -radius_factor, radius_factor, 0.0, 40.0);
	else
		gluPerspective(45.0, 1.0, 0.1, 40.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	light_position[0] = 5.5;
	light_position[1] = 0.0;
	light_position[2] = 0.0;
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	light_position[0] = -0.1;
	light_position[1] = 0.0;
	light_position[2] = 0.0;
	glLightfv(GL_LIGHT2, GL_POSITION, light_position);
}

//
// Set the scene for the object based on the center of the object

void set_scene(GLenum mode, Polyhedron* poly)
{
	glTranslatef(0.0, 0.0, -3.0);

	glScalef(1.0 / poly->radius, 1.0 / poly->radius, 1.0 / poly->radius);
	glTranslatef(-poly->center.entry[0], -poly->center.entry[1], -poly->center.entry[2]);
}

//
// OpenGL rendering of the object

void rainbow_color(float s,float s_max,float s_min,float rgb[3])
{

	float t = (s - s_min) / (s_max - s_min);
	// make sure t is between 0 and 1, if not, rgb should be black
	if (t < 0 || t>1) {
		rgb[0] = rgb[1] = rgb[2] = 0.;
		return;
	}
	float hsv[3] = { 1. };
	// map the scalar value linearly to the hue channel of the HSV
	hsv[0] = (1. - t) * 240;
	hsv[1] = hsv[2] = 1.; // set the saturation and value as 1
	// Call the HSV to RGB conversion function
	HsvRgb(hsv, rgb);
}


void set_bwr(int c) {
	Animate();
}

void bwr_color(float s,float s_max,float s_min,float rgb[3])
{

	float t = (s - s_min) / (s_max - s_min);
	// make sure t is between 0 and 1, if not, rgb should be black
	if (t < 0 || t>1) {
		rgb[0] = rgb[1] = rgb[2] = 0.;
		return;
	}

	float hsv[3] = { 1. };
	// map the scalar value linearly to the hue channel of the HSV

	float t_middle = s_middle * (s_max - s_min) + s_min;
	// map the scalar value linearly to the hue channel of the HSV

	if (s < t_middle) {
		hsv[0] = 240;	//(1. - t) * 240;
		hsv[1] = (t_middle - s) / (t_middle - s_min);
		//std::cout << t << std::endl;
	}
	else if (s > t_middle) {
		hsv[0] = 0;	//(1. - t) * 240;
		hsv[1] = (s - t_middle) / (s_max - t_middle);
	}
	else {
		hsv[1] = 0;
	}
	//hsv[0] = 240;	//(1. - t) * 240;
	//hsv[1] = (1-t);

	hsv[2] = 1.; // set the saturation and value as 1
	// Call the HSV to RGB conversion function
	HsvRgb(hsv, rgb);
}


void heatmap_color(float s,float s_max,float s_min,float rgb[3]) {
	float t = (s - s_min) / (s_max - s_min);

	if (t < 0 || t>1) {
		rgb[0] = rgb[1] = rgb[2] = 0.;
		return;
	}

	rgb[2] = 0;


	if (t > 10 / 3) {
		rgb[0] = 1;
	}
	else {
		rgb[0] = t * 10 / 3;
	}

	if (t > 10 / 6) {
		rgb[1] = 1;
		rgb[2] = (t - 0.6) * 10 / 3;
	}
	else {
		rgb[1] = (t - 0.3) * 10 / 3;
	}

}

void set_slice(int c) {
	slice_start = tfsstart->get_float_val();
	slice_end = tfssend->get_float_val();
	Animate();
}

void slicing(float s, float s_max,float s_min,float rgb[3]) {
	float t = (s - s_min) / (s_max - s_min);

	if (t < 0 || t>1) {
		rgb[0] = rgb[1] = rgb[2] = 0.;
		return;
	}

	rgb[2] = 0;


	if (s > slice_start && s < slice_end) {
		rgb[0] = 1;
		//std::cout << s << ", ";
	}
	else {
		rgb[0] = rgb[1] = rgb[2] = 0.;
		//std::cout << s << ", ";
	}

}
double fmod(double x, double y)
{
#pragma STDC FENV_ACCESS ON
	double result = std::remainder(std::fabs(x), (y = std::fabs(y)));
	if (std::signbit(result)) result += y;
	return std::copysign(result, x);
}
float color_num = 8;

void discrete_color(float s, float s_max, float s_min, float rgb[3])
{
	float thresholds[8];
	//std::cout << s_max-s_min;

	float t = (s - s_min) / (s_max - s_min);
	if (t < 0 || t>1) {
		rgb[0] = rgb[1] = rgb[2] = 0.;
		return;
	}
	float inc = (s_max - s_min) / color_num;
	for (float a = 0; a < color_num; a++) {
		thresholds[(int)a] = a * inc;

	}

	//std::cout<<std::endl;
	for (int a = 0; a < color_num - 1; a++) {
		if (t > thresholds[a] && t < thresholds[a + 1]) {
			//std::cout << t << ", ";
			t = thresholds[a];

			//std::cout << t << ", " << thresholds[a + 1] << std::endl;
			rgb[0] = discerete_colors[a][0];
			rgb[1] = discerete_colors[a][1];
			rgb[2] = discerete_colors[a][2];
		}
	}

}






void draw_arrow_head(double head[2], float direct[2])
{
	glPushMatrix();
	glTranslatef(head[0], head[1], 0);
	glRotatef(atan2(direct[1], direct[0]) * 360 / (2 * M_PI), 0, 0, 1);
	glScalef(0.03, 0.03, 1);
	glBegin(GL_TRIANGLES);
	glVertex2f(0, 0);
	glVertex2f(-0.35, 0.12);
	glVertex2f(-0.35, -0.12);
	glEnd();
	glPopMatrix();
}


void draw_line(float x1, float y1, float x2, float y2) {
	//std::cout << x1 << ", " << x2 << std::endl;
	glLineWidth(2);
	glBegin(GL_LINES);
	glColor3f(1, 0, 0);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}

float abs_min, abs_max;

void update_range(int c) {

	Animate();
}
bool update_r = true;

void calculate_bounds(Polyhedron* this_poly) {
	mag_max = -1000000;
	mag_min = 1000000;
	vx_min = 1000000;
	vx_max = -1000000;

	vy_min = 1000000;
	vy_max = -1000000;
	angle_min = 0;
	angle_max = M_PI;
	x_max = -100000;
	x_min = 1000000;
	y_max = -100000;
	y_min = 1000000;

	for (int i = 0; i < this_poly->ntris; i++) {

		Triangle* temp_t = this_poly->tlist[i];


		for (int j = 0; j < 3; j++) {

			Vertex* temp_v = temp_t->verts[j];
			float magnitude = sqrt(temp_v->vx * temp_v->vx + temp_v->vy * temp_v->vy);

			//std::cout << temp_v->s<<", ";

			if (magnitude < mag_min)
				mag_min = magnitude;

			if (magnitude > mag_max)
				mag_max = magnitude;


			if (temp_v->vx > vx_max)
				vx_max = temp_v->vx;

			if (temp_v->vx < vx_min)
				vx_min = temp_v->vx;


			if (temp_v->vy < vy_min)
				vy_min = temp_v->vy;

			if (temp_v->vy > vy_max)
				vy_max = temp_v->vy;

			x_max = max(x_max, temp_v->x);
			x_min = min(x_min, temp_v->x);

			y_max = max(y_max, temp_v->y);
			y_min = min(y_min, temp_v->y);

		}


	}
	//std::cout << "vxmax: " << vx_max;
}


void update_enhance(int i) {
	Choose_Object();
}


void Choose_Object()
{
	update_r = true;

	int w, h;
	switch (ObjectId) {
	case BUNNY:
		strcpy(object_name, "dipole");
		break;

	case FELINE:
		strcpy(object_name, "bnoise");
		break;

	case DRAGON:
		strcpy(object_name, "bruno3");
		break;

	case HAPPY:
		strcpy(object_name, "cnoise");
		break;

	case SPHERE:
		strcpy(object_name, "vnoise");
		break;
	}

	poly->finalize();

	Reset();

	char tmp_str[512];
	std::string fname = "";
	sprintf(tmp_str, "../models/%s.ply", object_name);
	fname.append(object_name);
	fname += "_sl.txt";
	read_streamline(fname);

	FILE* this_file = fopen(tmp_str, "r");
	poly = new Polyhedron(this_file);
	fclose(this_file);

	////Following codes build the edge information
	clock_t start, finish; //Used to show the time assumed
	start = clock(); //Get the first time

	poly->initialize(); // initialize everything

	poly->calc_bounding_sphere();
	poly->calc_face_normals_and_area();
	poly->average_normals();

	gen_noise_tex();
	calculate_bounds(poly);

	compute_LIC(do_enhance);



	finish = clock(); //Get the current time after finished
	double t = (double)(finish - start) / CLOCKS_PER_SEC;

	printf("\n");
	printf("The number of the edges of the object %s is %d \n", object_name, poly->nedges);
	printf("The Euler Characteristics of the object %s is %d \n", object_name, (poly->nverts - poly->nedges + poly->ntris));

	printf("Time to building the edge link is %f seconds\n", t);


	Glui->sync_live();
	glutSetWindow(MainWindow);
	glutPostRedisplay();


}

void draw_arrows_pls() {
	for (int i = 0; i < poly->ntris; i++) {

		Triangle* temp_t = poly->tlist[i];

		for (int j = 0; j < 3; j++) {
			double head[2];
			float dir[2];
			Vertex* temp_v = temp_t->verts[j];
			float magnitude = sqrt(temp_v->vx * temp_v->vx + temp_v->vy * temp_v->vy) * 25;
			//::cout << temp_v->vx/magnitude << std::endl;
			draw_line(temp_v->x, temp_v->y, temp_v->x + (temp_v->vx / magnitude), temp_v->y + (temp_v->vy / magnitude));


			head[0] = temp_v->x + (temp_v->vx / magnitude);
			head[1] = temp_v->y + (temp_v->vy / magnitude);
			dir[0] = temp_v->vx;
			dir[1] = temp_v->vy;
			draw_arrow_head(head, dir);
			//std::cout << temp_v->z<<", ";
		}


	}
}


void draw_color_at(int xx, int yy) {
	float rgb[4];
	rgb[3] = alpha;

	node_main* n = &grid_main[xx][yy];
	float val = 0, val_max = 1, val_min = 0;

	switch (vis_option) {
	case 1:
		if (err_type == 0) {
			val = n->err_mag_sl;
			//std::cout << n->err_mag_sl << std::endl;
			val_max = max_err_mag_sl;
			val_min = min_err_mag_sl;
		}
		else {
			val = n->err_mag_offset;
			val_max = max_err_mag_offset;
			val_min = min_err_mag_offset;
		}
		//if (n->err_mag_sl < 0)
			//std::cout << n->err_mag_sl<<", "<<xx<<", "<<yy << std::endl;
		break;

	case 2:
		if (err_type == 0) {
			val = n->err_dir_sl;
			val_max = max_err_dir_sl;
			val_min = min_err_dir_sl;
		}
		else {
			val = n->err_dir_offset;
			val_max = max_err_dir_offset;
			val_min = min_err_dir_offset;
		}
		break;

	case 3:
		if (err_type == 0) {
			val = n->err_vx_sl;
			val_max = max_err_vx_sl;
			val_min = min_err_vx_sl;
		}
		else {
			val = n->err_vx_offset;
			val_max = max_err_vx_offset;
			val_min = min_err_vx_offset;
		}
		break;

	case 4:
		if (err_type == 0) {
			val = n->err_vy_sl;
			val_max = max_err_vy_sl;
			val_min = min_err_vy_sl;
		}
		else {
			val = n->err_vy_offset;
			val_max = max_err_vy_offset;
			val_min = min_err_vy_offset;
		}
		break;
	case 5:
		if (err_type == 0) {
			val = n->err_vxy_sl;
			val_max = max_err_vxy_sl;
			val_min = min_err_vxy_sl;
		}
		else {
			/*val = n->err_vy_offset;
			val_max = max_err_vy_offset;
			val_min = min_err_vy_offset;*/
		}
		break;
	}


	if (whichcolorscheme == 0)
		rainbow_color(val, val_max, val_min, rgb);
	if (whichcolorscheme == 1)
		bwr_color(val, val_max, val_min, rgb);
	if (whichcolorscheme == 2)
		heatmap_color(val, val_max, val_min, rgb);
	if (whichcolorscheme == 3)
		discrete_color(val, val_max, val_min, rgb);
	if (whichcolorscheme == 4)
		slicing(val, val_max, val_min, rgb);
	//std::cout << rgb[0] << ", " << rgb[1] << ", " << rgb[2] << std::endl;
	glColor4fv(rgb);//s (rgb[0], rgb[1], rgb[2]);
	//glNormal3d(temp_v->normal.entry[0], temp_v->normal.entry[1], temp_v->normal.entry[2]);
	glVertex2d(n->x / 50, n->y / 50);
	//std::cout << rgb[0] << ", " << rgb[1] << ", " << rgb[2] << std::endl;
}


void draw_color() {
	for (int xx = 0; xx < 49; xx += 1) {
		for (int yy = 0; yy < 49; yy += 1) {
			glBegin(GL_QUADS);
			draw_color_at(xx, (yy + 1));
			draw_color_at(xx + 1, (yy + 1));
			draw_color_at(xx + 1, yy);
			draw_color_at(xx, yy);



			glEnd();
		}


	}
}

void draw_streamline(std::vector<node>* streamline) {
	//std::cout << "drawing "<< streamline->size()<<std::endl;
	if (streamline->size() == 0)
		return;
	if (streamline->size() == 1) {
		glBegin(GL_POINTS);
		node* n = &(*streamline)[0]; 
		glColor3f(1, 0.0, 0.0);   //this one		
		glVertex2d(n->x / 50, n->y / 50);
		glEnd();
		return;
	}

	for (int a = 0; a < (streamline->size() - 1); a++) {
		node* n = &(*streamline)[a];
		node* n1 = &(*streamline)[a + 1];
		//std::cout << a << "/" << streamline->size() <<", ";
		draw_line(n->x / 50, n->y / 50, n1->x / 50, n1->y / 50);/// IMG_RES
	}
	//std::cout << "finished" << std::endl;
}




void draw_strealines() {
	for (int s = 0; s < streamlines.size(); s++) {
		std::vector<node>* streamline = &streamlines[s];
		//std::cout << streamline->size();
		draw_streamline(streamline);
	}
}


void Display()
{

	glViewport(0, 0, (GLsizei)IMG_RES, (GLsizei)IMG_RES);
	glPointSize(5);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);


	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_TEXTURE_2D);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, IMG_RES, IMG_RES, 0,
		GL_RGB, GL_UNSIGNED_BYTE, LIC_tex);

	glBegin(GL_QUAD_STRIP);
	glTexCoord2f(0.0, 0.0); glVertex2f(0.0, 0.0);
	glTexCoord2f(0.0, 1.0); glVertex2f(0.0, 1.0);
	glTexCoord2f(1.0, 0.0); glVertex2f(1.0, 0.0);
	glTexCoord2f(1.0, 1.0); glVertex2f(1.0, 1.0);
	glEnd();


	glDisable(GL_TEXTURE_2D);

	//trace_and_draw(0, 0);
	if (do_draw_streamlines)
		draw_strealines();

	//glShadeModel(GL_SMOOTH);



	if (vis_option != 0) {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		draw_color();
	}

	// Add you arrow plot here and use a checkbox to enable its visualization
	if (do_draw_arrows)
		draw_arrows_pls();

	//draw_segment();
	//draw_segment_matrix();

	glutSwapBuffers();
	glFlush();


}