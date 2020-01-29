#ifndef vec_GUI
#define vec_GUI
#include "Skeleton.h"
#include "structures.h"
void draw_strealines();
void draw_streamline(std::vector<node>* streamline);
void draw_color();
void draw_color_at(int xx, int yy);
void draw_arrows_pls();
void Choose_Object();
void update_enhance(int i);
void calculate_bounds(Polyhedron* this_poly);
void draw_line(float x1, float y1, float x2, float y2);
void draw_arrow_head(double head[2], float direct[2]);
void discrete_color(float s, float s_max, float s_min, float rgb[3]);
void slicing(float s, float s_max, float s_min, float rgb[3]);
void set_slice(int c);
void heatmap_color(float s, float s_max, float s_min, float rgb[3]);
void bwr_color(float s, float s_max, float s_min, float rgb[3]);
void set_bwr(int c);
void rainbow_color(float s, float s_max, float s_min, float rgb[3]);
void set_scene(GLenum mode, Polyhedron* poly);
void set_view(GLenum mode, Polyhedron* poly);
void HsvRgb(float hsv[3], float rgb[3]);
void Axes(float length);
void Reset(void);
void MouseMotion(int x, int y);
void MouseButton(int button, int state, int x, int y);
void Keyboard(unsigned char c, int x, int y);
void InitLists(void);
void InitGraphics(void);
void InitGlui(void);
void interpolate(int n);
void Animate(void);
#endif