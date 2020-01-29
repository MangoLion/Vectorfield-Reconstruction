#ifndef vec_lic
#define vec_lic
#include "Skeleton.h"
#include "structures.h"
void lazy_seeding();
void compute_LIC(int enhance);
void interpolate_grid();
void init_grids();
void render_vec_img(Polyhedron* this_poly);
void trace_streamline_old(int xx, int yy);
void trace_streamline(int xx, int yy, int enhance);
std::vector<node>* trace_streamline_custom(int xx, int yy, float min_dis);
int interpolate_streamline_raycast(float xx, float yy, float& vx, float& vy, float radar);
node* get_nearest_streamline_node(float xx, float yy);
int interpolate_streamline_simple(float xx, float yy, float& vx, float& vy, float radar);
void interpolate_streamline_adv(float xx, float yy, float& vx, float& vy, float radar);
int interpolate_streamline(float xx, float yy, float& vx, float& vy, float radar);
int interpolate_streamline_selective(float xx, float yy, float& vx, float& vy, float radar);
node* min_node_to_steamline(float x, float y, std::vector<node>* streamline);
float min_distance_to_steamline(float x, float y, std::vector<node>* streamline);
void gen_noise_tex();
void bilinear_interpolate_node(node_main* b_left, node_main* b_right, node_main* t_right, node_main* t_left);
float bilinear_interpolate_scalar(float q11, float q12, float q21, float q22, float x1, float x2, float y1, float y2, float x, float y);
float min_distance_to_all_steamlines(float x, float y);
double getAbsoluteDiff2Angles(const double x, const double y);
void get_vector_truth(int xx, int yy, float& vx, float& vy);
#endif // !vec_lic
