#ifndef rec_graph
#define rec_graph
#include "structures.h"
void draw_segment();
void draw_line_color(float x1, float y1, float x2, float y2, float* rgb);
void draw_segment_matrix();
void update_graph();
void raycast_segment(segment* s, int direction);
std::vector<segment*>* get_segments_in_area(float x1, float y1, float x2, float y2);
#endif // !rec_graph
