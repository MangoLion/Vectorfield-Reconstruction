#include <windows.h>
#include "GL\include\GL.H"
#include "GL\include\glu.h"
#include "GL\include\glut.h"
#include "GL\include\glui.h"
#include <ctype.h>

#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "Reader.h"
#include "structures.h"
#include "globals.h"

/**
 * @brief direction constants, for 2D
 * 
 */
const int DIR_LEFT = 0, DIR_RIGHT = 1, DIR_NA = -1;

/**
 * @brief constant for the search range of each graph node to find its neighbors
 * 
 */
const float search_range = 3;

/**
 * @brief get the determinant of a 2x2 matrix
 */
inline double Det(double a, double b, double c, double d)
{
	return a * d - b * c;
}

/**
 * @brief Returns true if x is in range [low..high], else false 
 * 
 */
bool inRange(int low, int high, int x)
{
	return ((x - high) * (x - low) <= 0);
}

/**
 * @brief Calculate intersection of two lines.
 * http://mathworld.wolfram.com/Line-LineIntersection.html
 * 
 * @param x1 Line 1 start
 * @param y1 Line 1 start
 * @param x2 Line 1 end
 * @param y2 Line 1 end
 * @param x3 Line 2 start
 * @param y3 Line 2 start
 * @param x4 Line 2 end
 * @param y4 Line 2 end
 * @param ixOut intersection x result
 * @param iyOut intersection y result
 * @return true if intersection is found
 * @return false if intersection is not found or error
 */
bool LineLineIntersect(double x1, double y1, //Line 1 start
	double x2, double y2, //Line 1 end
	double x3, double y3, //Line 2 start
	double x4, double y4, //Line 2 end
	double& ixOut, double& iyOut) //Output 
{

	double detL1 = Det(x1, y1, x2, y2);
	double detL2 = Det(x3, y3, x4, y4);
	double x1mx2 = x1 - x2;
	double x3mx4 = x3 - x4;
	double y1my2 = y1 - y2;
	double y3my4 = y3 - y4;

	double xnom = Det(detL1, x1mx2, detL2, x3mx4);
	double ynom = Det(detL1, y1my2, detL2, y3my4);
	double denom = Det(x1mx2, y1my2, x3mx4, y3my4);
	if (denom == 0.0)//Lines don't seem to cross
	{
		ixOut = NAN;
		iyOut = NAN;
		return false;
	}

	ixOut = xnom / denom;
	iyOut = ynom / denom;
	if (!isfinite(ixOut) || !isfinite(iyOut)) //Probably a numerical issue
		return false;

	return true; //All OK
}

/**
 * @brief Get the graph segments within a rectangular area
 * 
 * @param x1 first x of the rect area
 * @param y1 first y of the rect area
 * @param x2 second x  of the rect area
 * @param y2 second y of the rect area
 * @return std::vector<segment*>* 
 */
std::vector<segment*>* get_segments_in_area(float x1, float y1, float x2, float y2) {
	std::vector<segment*>* results = new std::vector<segment*>();
	for (int i = 0; i < segment_starters.size(); i++) {
		segment* s = segment_starters[i];
		while (s != nullptr) {
			if (inRange(x1, x2, s->middle->x) &&
				inRange(y1, y2, s->middle->y)) {
				results->push_back(s);
			}
			s = s->next;
		}
	}
	return results;
}

/**
 * @brief from the center of the segment, raycast into two perpendicular directions and check for intersections with surrounding segments
 * 
 * @param s 
 * @param direction 
 */
void raycast_segment(segment* s, int direction) {
	int dir;
	float perp_vx = -1 * s->start->vy,
		perp_vy = s->start->vx;
	switch (direction) {
	case DIR_LEFT:
		perp_vx = s->start->vy;
		perp_vy = -1 * s->start->vx;
		break;
	case DIR_RIGHT:
		perp_vx = -1 * s->start->vy;
		perp_vy = s->start->vx;
		break;
	}


	float magnitude = sqrt(perp_vx * perp_vx + perp_vy * perp_vy),
		search_vx = perp_vx / magnitude * search_range,
		search_vy = perp_vy / magnitude * search_range;

	std::vector<segment*>* segments_in_area =
		get_segments_in_area(s->middle->x - search_vx * 1.4, s->middle->y - search_vy * 1.4,
			s->middle->x + search_vx * 1.4,
			s->middle->y + search_vy * 1.4);
	float min_dist = 1000000;
	segment* result = nullptr;
	double int_x, int_y;
	for (int c = 0; c < segments_in_area->size(); c++) {
		segment* s2 = (*segments_in_area)[c];
		if (s2 == s || s2->streamline_index == s->streamline_index)
			continue;

		if (LineLineIntersect(
			s->middle->x, s->middle->y,
			s->middle->x + search_vx, s->middle->y + search_vy,
			s2->start->x, s2->start->y,
			s2->end->x, s2->end->y,
			int_x, int_y)) {
			float dist = sqrt((int_x - s->middle->x) * (int_x - s->middle->x) + (int_y - s->middle->y) * (int_y - s->middle->y));
			//std::cout <<dist << ",";
			if (dist < min_dist) {
				min_dist = dist;
				result = s2;
			}
		}

	}
	if (result != nullptr) {
		neighbor* n = new neighbor;
		n->dist = min_dist;
		n->direction = direction;
		n->seg = result;
		n->vx = int_x - s->middle->vx;
		n->vy = int_y - s->middle->vy;
		s->neighbors.push_back(n);

		neighbor* n2 = new neighbor;
		n2->dist = min_dist;
		n2->direction = DIR_NA;
		n2->seg = s;
		n2->vx = -(int_x - s->middle->vx);
		n2->vy = -(int_y - s->middle->vy);
		result->neighbors.push_back(n2);
		//std::cout << s->neighbors.size()<<",";
	}
	segments_in_area->clear();
	delete segments_in_area;
}

int skip_lines = 1, skip_nodes = 1;

/**
 * @brief loop through all segments (depend on the current segment resolution) to find their neighbors
 * 
 */
void update_graph() {
	int counter = 0;
	//#pragma omp parallel for
	for (int i = 0; i < segment_starters.size(); i++) {
		if (i % skip_lines != 0)
			continue;
		segment* s = segment_starters[i];
		int node_counter = 0;
		while (s != nullptr) {
			node_counter++;
			if (node_counter % skip_nodes != 0)
				continue;
			raycast_segment(s, DIR_LEFT);
			raycast_segment(s, DIR_RIGHT);
			s = s->next;

		}
		counter++;
		std::cout << "Progress " << counter << "/" << segment_starters.size() << std::endl;
	}

	//std::cout << "SEG SIZE " << segment_starters.size() << std::endl;
}

/**
 * @brief draw segment matrix, only meant for very small resolution to confirm the accuracy of the neighbors finding algorithm
 * 
 */
void draw_segment_matrix() {
	glColor3f(0, 0, 0);
	glBegin(GL_QUADS);
	glVertex2f(0, 1);
	glVertex2f(1, 1);
	glVertex2f(1, 0);
	glVertex2f(0, 0);
	glEnd();


	int progress = 0;
	float total = 0;
	for (int i2 = 0; i2 < segment_starters.size(); i2++) {
		float i = i2;
		int segment_draw_index = i2 / skip_lines;
		float rgb[3];
		if ((int)segment_draw_index % 2 == 0) {
			rgb[0] = 1;
			rgb[1] = 0;
			rgb[2] = 0;
		}
		else {
			rgb[0] = 0;
			rgb[1] = 0;
			rgb[2] = 1;
		}

		float scounter = 0;
		int node_counter2 = 0;
		segment* s = segment_starters[i];
		while (s != nullptr) {
			node_counter2++;
			int sscounter2 = 0;
			scounter++;
			//#pragma omp parallel for
			for (int a2 = 0; a2 < segment_starters.size(); a2++) {
				float a = a2;
				//if (a == i)
				//	continue;
				int segment_draw_index2 = a2 / skip_lines;

				segment* s2 = segment_starters[a];
				while (s2 != nullptr) {
					sscounter2++;

					for (float n = 0; n < s->neighbors.size(); n++) {
						if (s->neighbors[n]->seg == s2 && s->neighbors[n]->direction != DIR_NA) {
							glPointSize(1);
							glBegin(GL_POINTS);
							glColor3fv(rgb);
							glVertex2f((total + scounter + 1.) / IMG_RES,
								(sscounter2 + 1.) / IMG_RES);
							//std::cout << "drawing " << segment_draw_index << " + " << node_draw_index << ", "
							//	<< segment_draw_index2 << ", " << node_draw_index2 << std::endl;

							glEnd();
							break;
						}
					}
					s2 = s2->next;
				}
			}
			s = s->next;
		}
		if (scounter == 0)
			continue;



		//std::cout << "scounter: " << scounter << std::endl;
		glColor3fv(rgb);
		glBegin(GL_LINES);
		glVertex2f((total + 2) / IMG_RES, 0);
		glVertex2f((total + scounter + 2) / IMG_RES, 0);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f(0, (total + 2) / IMG_RES);
		glVertex2f(0, (total + scounter + 2) / IMG_RES);
		glEnd();
		/*
		glColor4f(1,1,1,0.2);
		glBegin(GL_LINES);
		glVertex2f((total) / IMG_RES, 0);
		glVertex2f((total) / IMG_RES, 1);
		glVertex2f((total + scounter) / IMG_RES, 0);
		glVertex2f((total + scounter) / IMG_RES, 1);
		glEnd();

		glBegin(GL_LINES);
		glVertex2f(1, total / IMG_RES);
		glVertex2f(0, total / IMG_RES);
		glVertex2f(1, (total + scounter) / IMG_RES);
		glVertex2f(0, (total + scounter) / IMG_RES);
		glEnd();
		*/
		total += scounter;

		progress++;
		std::cout << "progress: " << progress << "/" << segment_starters.size() << std::endl
			<< "total: " << total << std::endl;
	}
}


void draw_line_color(float x1, float y1, float x2, float y2, float* rgb) {
	//std::cout << x1 1<< ", " << x2 << std::endl;
	glColor3fv(rgb);
	glLineWidth(2);
	glBegin(GL_LINES);
	glVertex2f(x1, y1);
	glVertex2f(x2, y2);
	glEnd();
}


void draw_segment() {
	float* rgb = new float[3];
	for (int i = 0; i < segment_starters.size(); i++) {
		segment* s = segment_starters[i];
		while (s != nullptr) {
			rgb[0] = 1;
			rgb[1] = 0;
			rgb[2] = 0;
			draw_line_color(s->start->x / 50, s->start->y / 50, s->end->x / 50, s->end->y / 50, rgb);
			s = s->next;
		}
	}


	segment* s = segment_starters[segment_current];
	while (s != nullptr) {
		rgb[0] = 0;
		rgb[1] = 1;
		rgb[2] = 0;
		draw_line_color(s->start->x / 50, s->start->y / 50, s->end->x / 50, s->end->y / 50, rgb);
		std::cout << "---" << std::endl <<
			s->start->x << ", " << s->start->y << ", " << s->end->x << ", " << s->end->y << ". Neighbors:" << std::endl;

		for (int a = 0; a < s->neighbors.size(); a++) {
			neighbor* n = s->neighbors[a];
			rgb[0] = 1;
			rgb[1] = 1;
			rgb[2] = 0;
			segment* s2 = n->seg;
			draw_line_color(s2->start->x / 50, s2->start->y / 50, s2->end->x / 50, s2->end->y / 50, rgb);
			std::cout << s2->start->x << ", " << s2->start->y << ", " << s2->end->x << ", " << s2->end->y << std::endl;
		}
		s = s->next;
	}
}