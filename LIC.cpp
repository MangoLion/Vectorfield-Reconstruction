/**
 * @file LIC.cpp
 * @brief Handles LIC Texture Creation AND Vector Field Reconstruction Methods
 * @version 0.1
 * @date 2020-01-18
 * 
 * @copyright Copyright (c) 2020
 * 
 */

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
#include "Skeleton.h"

#define _USE_MATH_DEFINES
#include <math.h>

void lazy_seeding();

/**
 * @brief set the values vx and vy of the vector field from the encoded texture array data_img and assign it to vx, vy input
 * 
 * @param xx x value on the data_img texture
 * @param yy y value on the data_img texture
 * @param vx the resulting vx value after decoding
 * @param vy the resulting vy value after decoding
 */
void get_vector_truth(int xx, int yy, float& vx, float& vy) {
	vx = data_img[xx][yy][0];// vx_min + (vx_max - vx_min) * data_img[xx][yy][0] / 255.0;
	vy = data_img[xx][yy][1];//vy_min + (vy_max - vy_min) * data_img[xx][yy][1] / 255.0;
}


/**
 * @brief Get the absolute diff between two angles
 * 
 * @param x angle 1
 * @param y angle 2
 * @return double abs difference between two angles
 */
double getAbsoluteDiff2Angles(const double x, const double y)
{
	// c can be PI (for radians) or 180.0 (for degrees);
	float c = M_PI;
	return c - fabs(fmod(fabs(x - y), 2 * c) - c);
}

/**
 * @brief get the minimum distance between the current point and the nearest point's streamline
 * 
 * @param x 
 * @param y 
 * @return float min dist
 */
float min_distance_to_all_steamlines(float x, float y) {
	float dis = 100000;
	//return dis;
	for (int s = 0; s < streamlines.size(); s++) {
		std::vector<node>* streamline = &streamlines[s];
		for (int n = 0; n < streamline->size(); n++) {
			node* nn = &(*streamline)[n];
			float tdis = abs(nn->x - x) + abs(nn->y - y);
			//std::cout << tdis << std::endl;
			dis = min(dis, tdis);
		}
	}
	return dis;
}

//https://helloacm.com/cc-function-to-compute-the-bilinear-interpolation/

/**
 * @brief perform bilinear interpolation
 * 
 * @param q11 bottom left value
 * @param q12 top left value
 * @param q21 bottom right value
 * @param q22 top right value
 * @param x1 left most x
 * @param x2 right most x
 * @param y1 bottom y
 * @param y2 top y
 * @param x x value of the point to be interpolated
 * @param y y value of the point to be interpolated
 * @return float 
 */
float bilinear_interpolate_scalar(float q11, float q12, float q21, float q22, float x1, float x2, float y1, float y2, float x, float y)
{
	float x2x1, y2y1, x2x, y2y, yy1, xx1;
	x2x1 = x2 - x1;
	y2y1 = y2 - y1;
	x2x = x2 - x;
	y2y = y2 - y;
	yy1 = y - y1;
	xx1 = x - x1;
	return 1.0 / (x2x1 * y2y1) * (
		q11 * x2x * y2y +
		q21 * xx1 * y2y +
		q12 * x2x * yy1 +
		q22 * xx1 * yy1
		);
}

/**
 * @brief interpolate the vx and vy values using four nodes. By adding the offset value to node b_left to get the interpolated x and y value and then the 
 *  resulting interpolated vx and vy is assigned into node b_left.vx_offset and vy_offset
 *  the purpose of this function is to use bilinear interpolation to determine the vector value of the offset grid points
 * 
 * @param b_left bottom left node
 * @param b_right bottom right node
 * @param t_right top right node
 * @param t_left top left node
 */
void bilinear_interpolate_node(node_main* b_left, node_main* b_right, node_main* t_right, node_main* t_left) {
	float gridw = b_right->x - b_left->x,
		gridh = t_right->y - b_left->y,
		u = b_left->y + grid_offset_x / gridh,
		t = b_left->x + grid_offset_y / gridw;

	b_left->vx_offset = bilinear_interpolate_scalar(b_left->vx_sl, t_left->vx_sl, b_right->vx_sl, t_right->vx_sl,
		b_left->x, b_right->x, b_left->y, t_left->y, u, t);
	b_left->vy_offset = bilinear_interpolate_scalar(b_left->vy_sl, t_left->vy_sl, b_right->vy_sl, t_right->vy_sl,
		b_left->x, b_right->x, b_left->y, t_left->y, u, t);

}

/**
 * @brief generate the white noise texture for LIC generation
 * 
 */
void gen_noise_tex()
{

	for (int x = 0; x < IMG_RES; x++)
		for (int y = 0; y < IMG_RES; y++)
		{
			noise_tex[x][y][0] =
				noise_tex[x][y][1] =
				noise_tex[x][y][2] = (unsigned char)255 * (rand() % 32768) / 32768.0;
		}
	for (int x = 0; x < 50; x++)
		for (int y = 0; y < 50; y++)
		{
			data_check[x][y] = false;
		}
}

/**
 * @brief get the minimum distance to a specific streamline from a point x,y
 * 
 * @param x 
 * @param y 
 * @param streamline 
 * @return float min distance to streamline
 */
float min_distance_to_steamline(float x, float y, std::vector<node>* streamline) {
	float dis = 100000;
	//return dis;
	for (int n = 0; n < streamline->size(); n++) {
		node* nn = &(*streamline)[n];
		float tdis = abs(nn->x - x) + abs(nn->y - y);
		//std::cout << tdis << std::endl;
		dis = min(dis, tdis);
	}
	return dis;
}

/**
 * @brief get the closest node to a specific streamline from a point
 * 
 * @param x 
 * @param y 
 * @param streamline 
 * @return node* the closest node of that streamline
 */
node* min_node_to_steamline(float x, float y, std::vector<node>* streamline) {
	float dis = 100000;
	node* result = NULL;
	//return dis;
	for (int n = 0; n < streamline->size(); n++) {
		node* nn = &(*streamline)[n];
		float tdis = abs(nn->x - x) + abs(nn->y - y);
		//std::cout << tdis << std::endl;
		if (tdis < dis) {
			dis = tdis;
			result = nn;
		}
	}
	return result;
}

/**
 * @brief trackers for average difference in vector values
 * 
 */
float v_diff_average = 0, v_diff_counter = 0;

/**
 * @brief reconstruct the vector value of the point (xx,yy) on the vector field by using the Selective Weighed Sum method, this method provides the most reliable reconstruction
 * it first uses the nearest streamline node's vector value as the base, then it finds the next closest node and compare the vector value, if the difference is below the average threshold then
 * it computes the weighted sum between two nodes, if the diff is too large then it discard the second node. The process is repeated untill all nodes within radar range is processed
 * 
 * @param xx x coord of the point on the vector field
 * @param yy y coord of the point on the vector field
 * @param vx the resulting vx value after reconstruction
 * @param vy the resulting vy value after reconstruction
 * @param radar the radar diameter to scan for surrounding streamline nodes
 * @return int how many streamline nodes where found within the radar range, used for recursion. If the result is too few then increase radar range and vice versa
 */
int interpolate_streamline_selective(float xx, float yy, float& vx, float& vy, float radar) {
	float total_dis = 0;
	vx = 0;
	vy = 0;
	std::vector<node*> results;

	for (int s = 0; s < streamlines.size(); s++) {
		std::vector<node>* streamline = &streamlines[s];

		for (int n = 0; n < streamline->size(); n++) {
			node* n2 = &(*streamline)[n];
			float tdis = abs(xx - n2->x) + abs(yy - n2->y);
			if (tdis < radar) {
				tdis = sqrt((xx - n2->x) * (xx - n2->x) + (yy - n2->y) * (yy - n2->y));
				if (tdis >= radar)
					continue;
				tdis = radar - tdis;
				n2->dis = tdis;
				total_dis += tdis;

				if (results.size() > 0 && results[0]->dis > n2->dis) {
					results.insert(results.begin(), n2);
				}
				else
					if (results.size() == 0 || results[results.size() - 1]->dis < n2->dis) {
						results.push_back(n2);
					}
					else {
						for (int i = 0; i < results.size() - 1; i++) {
							//std::cout << "a "<<i<<"/"<< results.size()<<std::endl;
							node* n3 = results[i],
								* n4 = results[i + 1];
							//std::cout << "b";
							if (n3->dis < tdis && n4->dis > tdis) {
								results.insert(results.begin() + i, n2);
								break;
							}
						}
					}
			}
		}

	}
	if (results.size() == 0)
		return interpolate_streamline_selective(xx, yy, vx, vy, radar * 1.5);
	//std::cout << "weights: ";
	for (int i = 0; i < results.size(); i++) {
		node* n = results.at(i);
		float weight = n->dis / total_dis;
		//std::cout << weight<< ", ";
		float angle_org = atan(vy / vx),
			angle_next = atan(n->vy / n->vx);
		if (vx == vy && vx == 0)
			angle_org = angle_next;
		float angle_diff = getAbsoluteDiff2Angles(angle_org, angle_next),
			v_diff = abs(vx - n->vx) + abs(vy - n->vy);
		v_diff_average += v_diff;
		v_diff_counter++;
		if (/*angle_diff > 0.6 ||*/ v_diff > 0.5 || isnan(angle_diff)) {
			continue;
		}
		vx += n->vx * weight;
		vy += n->vy * weight;
	}
	std::cout << std::endl;
	return results.size();
}


/**
 * @brief reconstruct the vector value of the point (xx,yy) on the vector field by using the Adaptive Weighed Sum method
 * 
 * @param xx x coord of the point on the vector field
 * @param yy y coord of the point on the vector field
 * @param vx the resulting vx value after reconstruction
 * @param vy the resulting vy value after reconstruction
 * @param radar the radar diameter to scan for surrounding streamline nodes
 * @return int how many streamline nodes where found within the radar range, used for recursion. If the result is too few then increase radar range and vice versa
 */
int interpolate_streamline(float xx, float yy, float& vx, float& vy, float radar) {
	float total_dis = 0;
	vx = 0;
	vy = 0;
	std::vector<node*> results;
	for (int s = 0; s < streamlines.size(); s++) {
		std::vector<node>* streamline = &streamlines[s];
		for (int n = 0; n < streamline->size(); n++) {
			node* n2 = &(*streamline)[n];
			float tdis = abs(xx - n2->x) + abs(yy - n2->y);
			if (tdis < radar) {
				tdis = sqrt((xx - n2->x) * (xx - n2->x) + (yy - n2->y) * (yy - n2->y));
				if (tdis >= radar)
					continue;
				tdis = radar - tdis;
				results.push_back(n2);
				n2->dis = tdis;
				total_dis += tdis;
			}
		}

	}
	if (results.size() == 0)
		return interpolate_streamline(xx, yy, vx, vy, radar * 1.5);
	if (results.size() > 7)
		return interpolate_streamline(xx, yy, vx, vy, radar * 0.8);
	for (int i = 0; i < results.size(); i++) {
		node* n = results.at(i);
		float weight = n->dis / total_dis;//(radar - n->dis) / radar;
		//std::cout << n->dis << std::endl;

		vx += n->vx * weight;
		vy += n->vy * weight;
	}

	//vx /= results.size();
	//vy /= results.size();
	//std::cout << vx << ", " << vy << std::endl;
	return results.size();
}


float avg_angle_diff = 0;
int angle_diff_counter = 0;

/**
 * @brief TODO: figure out what this method does
 * 
 * @param xx 
 * @param yy 
 * @param vx 
 * @param vy 
 * @param radar 
 */
void interpolate_streamline_adv(float xx, float yy, float& vx, float& vy, float radar) {
	float total_dis = 0;
	vx = 0;
	vy = 0;
	std::vector<streamlet*> streamlets;
	streamlet* minlet = nullptr;
	float minlet_dist = 100000;
	for (int s = 0; s < streamlines.size(); s++) {
		std::vector<node>* streamline = &streamlines[s];
		streamlet* st = new streamlet;
		float min_dis = 100000;
		for (int i = 0; i < streamline->size(); i++) {
			node* nn = &(*streamline)[i];
			float dis = abs(nn->x - xx) + abs(nn->y - yy);
			if (dis < radar) {
				dis = sqrt((nn->x - xx) * (nn->x - xx) + (nn->y - yy) * (nn->y - yy));
				if (dis > radar) {
					continue;
				}
				nn->dis = radar - dis;
				st->nodes.push_back(nn);
				st->ttl_dist += nn->dis;
				if (dis < min_dis) {
					st->dist = min_dis = dis;
				}
				if (dis < minlet_dist) {
					minlet = st;
					minlet_dist = dis;
				}
			}
		}

		if (st->nodes.size() > 0) {
			float ttl_w = 0;
			for (int i = 0; i < st->nodes.size(); i++) {
				node* n = st->nodes[i];
				float weight = (n->dis / st->ttl_dist);//(radar - n->dis) / radar;//(n->dis / st->ttl_dist);//(radar - n->dis) / radar;//(total_dis - n->dis) / total_dis;
				ttl_w += weight;
				st->vx += n->vx * weight;
				st->vy += n->vy * weight;
			}
			std::cout << "ttl weight: " << ttl_w << std::endl;
			//st->vx /= st->nodes.size();
			//st->vy /= st->nodes.size();
			streamlets.push_back(st);
		}
	}
	if (minlet == nullptr)
		return;
	//shallowcopy
	streamlet current_streamlet = *minlet;
	for (std::vector<streamlet*>::iterator it = streamlets.end() - 1; it != streamlets.begin(); it--) {
		if ((*it) == minlet) {
			it = streamlets.erase(it);
			break;
		}
	}

	while (streamlets.size() > 0) {
		float min_dis = 10000;
		int index;
		for (int i = 0; i < streamlets.size(); i++) {
			streamlet* st = streamlets[i];
			if (st->dist < min_dis) {
				min_dis = st->dist;
				index = i;
			}
		}
		streamlet* next_streamlet = streamlets[index];
		streamlets.erase(streamlets.begin() + index);
		continue;
		float angle_org = atan(current_streamlet.vy / current_streamlet.vx),
			angle_next = atan(next_streamlet->vy / next_streamlet->vx),
			angle_diff = getAbsoluteDiff2Angles(angle_org, angle_next),
			v_diff = abs(current_streamlet.vx - next_streamlet->vx) + abs(current_streamlet.vy - next_streamlet->vy);
		avg_angle_diff += v_diff;
		angle_diff_counter++;
		if (isnan(angle_diff)) {
			//	std::cout << "["<<angle_org <<", "<< angle_next <<"]"<< std::endl;
		}
		else {
			//avg_angle_diff += angle_diff;
			//angle_diff_counter++;
			//std::cout << angle_diff << std::endl;
		}


		if (angle_diff > 0.4 || v_diff > 1 || isnan(angle_diff)) {
			continue;
		}

		float ttl_dist = (radar - current_streamlet.dist) + (radar - next_streamlet->dist),
			weight_org = (radar - current_streamlet.dist) / ttl_dist,
			weight_next = (radar - next_streamlet->dist) / ttl_dist;
		//std::cout << "[" << weight_org << ", " << weight_next << "]" << std::endl;
		current_streamlet.vx = current_streamlet.vx * weight_org + next_streamlet->vx * weight_next;
		current_streamlet.vy = current_streamlet.vy * weight_org + next_streamlet->vy * weight_next;
	}

	vx = current_streamlet.vx;
	vy = current_streamlet.vy;



}

/**
 * @brief reconstruct the vector value of a point by doing a simple weighted sum of all nearby streamline points. This is the least accurate method
 * 
 * @param xx x coord of the point on the vector field
 * @param yy y coord of the point on the vector field
 * @param vx the resulting vx value after reconstruction
 * @param vy the resulting vy value after reconstruction
 * @param radar the radar diameter to scan for surrounding streamline nodes
 * @return int how many streamline nodes where found within the radar range, used for recursion. If the result is too few then increase radar range and vice versa
 */
int interpolate_streamline_simple(float xx, float yy, float& vx, float& vy, float radar) {
	float total_dis = 0;
	vx = 0;
	vy = 0;
	std::vector<node*>* results = new std::vector<node*>();
	for (int s = 0; s < streamlines.size(); s++) {
		std::vector<node>* streamline = &streamlines[s];
		node* n2 = min_node_to_steamline(xx, yy, streamline);
		if (n2 == NULL)
			continue;
		float dis = abs(n2->x - xx) + abs(n2->y - yy);
		if (dis < radar) {
			results->push_back(n2);
			n2->dis = sqrt((xx - n2->x) * (xx - n2->x) + (yy - n2->y) * (yy - n2->y));
			total_dis += dis;
		}
	}

	if (results->size() == 0)
		return 0;

	for (int i = 0; i < results->size(); i++) {
		node* n = results->at(i);
		float weight = (radar - n->dis) / radar;//(total_dis - n->dis) / total_dis;
		vx += n->vx * weight;
		vy += n->vy * weight;
	}
	vx /= results->size();
	vy /= results->size();


	//std::cout << err << std::endl;
	return results->size();
}

/**
 * @brief Get the nearest streamline node object, search through all streamlines
 * 
 * @param xx x coord of the search
 * @param yy y coord of the search
 * @return node* closest streamline node
 */
node* get_nearest_streamline_node(float xx, float yy) {
	//std::vector<node>* result = nullptr;//new std::vector<node>();
	node* result = nullptr;
	float min_dis = 100000;
	for (int s = 0; s < streamlines.size(); s++) {
		std::vector<node>* streamline = &streamlines[s];
		node* n2 = min_node_to_steamline(xx, yy, streamline);
		if (n2 == NULL)
			continue;
		float dis = sqrt((n2->x - xx) * (n2->x - xx) + (n2->y - yy) * (n2->y - yy));
		if (dis < min_dis) {
			result = n2;
			min_dis = dis;
		}
	}

	return result;
}

/**
 * @brief poor man's raycast implementation, only check four offset points from the desired vector field point. Not very effective
 * 
 * @param xx x coord of the point on the vector field
 * @param yy y coord of the point on the vector field
 * @param vx the resulting vx value after reconstruction
 * @param vy the resulting vy value after reconstruction
 * @param radar the radar diameter to scan for surrounding streamline nodes
 * @return int how many streamline nodes where found within the radar range, used for recursion. If the result is too few then increase radar range and vice versa
 */
int interpolate_streamline_raycast(float xx, float yy, float& vx, float& vy, float radar) {
	float total_dis = 0;
	vx = 0;
	vy = 0;
	node* n1, * n2, * n3, * n4;
	n1 = get_nearest_streamline_node(xx - 2, yy);
	n2 = get_nearest_streamline_node(xx + 2, yy);
	n3 = get_nearest_streamline_node(xx, yy + 2);
	n4 = get_nearest_streamline_node(xx, yy - 2);

	vx = (n1->vx + n2->vx + n3->vx + n4->vx) / 4.0;
	vy = (n1->vy + n2->vy + n3->vy + n4->vy) / 4.0;

	//std::cout << err << std::endl;
	return 0;
}



float min_distance_to_all_steamlines(float x, float y);
int STREAMLINE_LENGTH = 500;

/**
 * @brief trace the streamline based on the given point, will stop when it gets too close to another streamline
 * 
 * @param xx start tracing from this x
 * @param yy start tracing from this y
 * @param min_dis the min distance to another streamline to stop
 * @return std::vector<node>* the streamline
 */
std::vector<node>* trace_streamline_custom(int xx, int yy, float min_dis) {
	int ox = xx,
		oy = yy;

	std::vector<node>* streamline_second = new std::vector<node>();
	std::vector<node>* streamline_first = new std::vector<node>();
	STREAMLINE_LENGTH = 200;
	float y = yy + .5;
	float x = xx + .5;

	int next_x = xx;
	int next_y = yy;

	int last_x = xx, last_y = yy;

	int counter = 0;


	for (int i = 0; i < STREAMLINE_LENGTH / 2; i++) {
		float vx, vy;

		vx = vx_min + (vx_max - vx_min) * vec_img[next_x][next_y][0] / 255.0;
		vy = vy_min + (vy_max - vy_min) * vec_img[next_x][next_y][1] / 255.0;
		x = x + (vy / sqrt(vx * vx + vy * vy));
		y = y + (vx / sqrt(vx * vx + vy * vy));
		if (min_distance_to_all_steamlines(y, x) < min_dis)
			break;

		node* n = new node();
		n->x = y;
		n->vx = vx;
		n->y = x;
		n->vy = vy;

		streamline_second->push_back(*n);

		next_y = int(y);
		next_x = int(x);
		if (next_x == last_x && next_y == last_y) {
			last_x = next_x;
			last_y = next_y;
			continue;
		}

		counter++;

		last_x = next_x;
		last_y = next_y;


		if (next_x < -1 || next_x > IMG_RES ||
			next_y < -1 || next_y > IMG_RES) {
			break;
		}
	}
	//return streamline;

	xx = ox;
	yy = oy;
	y = yy + .5;
	x = xx + .5;

	next_x = xx;
	next_y = yy;
	last_x = xx, last_y = yy;

	for (int i = 0; i < STREAMLINE_LENGTH / 2; i++) {
		float vx, vy;

		vx = vx_min + (vx_max - vx_min) * vec_img[next_x][next_y][0] / 255.0;
		vy = vy_min + (vy_max - vy_min) * vec_img[next_x][next_y][1] / 255.0;
		x = x - (vy / sqrt(vx * vx + vy * vy));
		y = y - (vx / sqrt(vx * vx + vy * vy));
		if (min_distance_to_all_steamlines(y, x) < min_dis)
			break;

		node* n = new node();
		n->x = y;
		n->vx = vx;
		n->y = x;
		n->vy = vy;

		streamline_first->push_back(*n);

		next_x = int(x);
		next_y = int(y);

		if (next_x == last_x && next_y == last_y) {
			last_x = next_x;
			last_y = next_y;
			continue;
		}

		counter++;

		last_x = next_x;
		last_y = next_y;


		if (next_x < -1 || next_x > IMG_RES ||
			next_y < -1 || next_y > IMG_RES) {
			break;
		}
	}
	std::vector<node>* result = new std::vector<node>();
	std::reverse(streamline_first->begin(), streamline_first->end());
	result->reserve(streamline_first->size() + streamline_second->size());
	result->insert(result->end(), streamline_first->begin(), streamline_first->end());
	result->insert(result->end(), streamline_second->begin(), streamline_second->end());
	return result;
}

/**
 * @brief trace the streamlet to produce the LIC texture
 * 
 * @param xx start tracing from this x
 * @param yy start tracing from this y
 * @param enhance use the enhanced LIC texture or not (0 or 1)
 */
void trace_streamline(int xx, int yy, int enhance) {

	STREAMLINE_LENGTH = 40;
	float y = yy + .5;
	float x = xx + .5;

	int a = 0;
	int next_x = xx;
	int next_y = yy;

	int last_x = xx, last_y = yy;

	int rgb_sum[3];
	rgb_sum[0] = 0;
	rgb_sum[1] = 0;
	rgb_sum[2] = 0;
	int counter = 0;


	for (int i = 0; i < STREAMLINE_LENGTH / 2; i++) {
		float vx, vy;

		vx = vx_min + (vx_max - vx_min) * vec_img[next_x][next_y][0] / 255.0;
		vy = vy_min + (vy_max - vy_min) * vec_img[next_x][next_y][1] / 255.0;
		x = x + (vy / sqrt(vx * vx + vy * vy));
		y = y + (vx / sqrt(vx * vx + vy * vy));

		next_y = int(y);
		next_x = int(x);
		if (next_x == last_x && next_y == last_y) {
			last_x = next_x;
			last_y = next_y;
			continue;
		}


		if (enhance == 0) {
			rgb_sum[0] += noise_tex[last_x][last_y][0];
			rgb_sum[1] += noise_tex[last_x][last_y][1];
			rgb_sum[2] += noise_tex[last_x][last_y][2];
		}
		else {
			rgb_sum[0] += LIC_tex_backup[last_x][last_y][0];
			rgb_sum[1] += LIC_tex_backup[last_x][last_y][1];
			rgb_sum[2] += LIC_tex_backup[last_x][last_y][2];
		}
		counter++;

		last_x = next_x;
		last_y = next_y;


		if (next_x < -1 || next_x > IMG_RES ||
			next_y < -1 || next_y > IMG_RES) {
			break;
		}
	}

	y = yy + .5;
	x = xx + .5;

	next_x = xx;
	next_y = yy;

	last_x = xx, last_y = yy;

	for (int i = 0; i < STREAMLINE_LENGTH / 2; i++) {
		float vx, vy;

		vx = vx_min + (vx_max - vx_min) * vec_img[next_x][next_y][0] / 255.0;
		vy = vy_min + (vy_max - vy_min) * vec_img[next_x][next_y][1] / 255.0;
		x = x - (vy / sqrt(vx * vx + vy * vy));
		y = y - (vx / sqrt(vx * vx + vy * vy));

		next_x = int(x);
		next_y = int(y);
		next_x = 20;
		next_x = int(x);

		if (next_x == last_x && next_y == last_y) {
			last_x = next_x;
			last_y = next_y;
			continue;
		}



		if (enhance == 0) {
			rgb_sum[0] += noise_tex[last_x][last_y][0];
			rgb_sum[1] += noise_tex[last_x][last_y][1];
			rgb_sum[2] += noise_tex[last_x][last_y][2];
		}
		else {
			rgb_sum[0] += LIC_tex_backup[last_x][last_y][0];
			rgb_sum[1] += LIC_tex_backup[last_x][last_y][1];
			rgb_sum[2] += LIC_tex_backup[last_x][last_y][2];
		}
		counter++;

		last_x = next_x;
		last_y = next_y;


		if (next_x < -1 || next_x > IMG_RES ||
			next_y < -1 || next_y > IMG_RES) {
			break;
		}
	}

	LIC_tex[xx][yy][0] = rgb_sum[0] / counter;
	LIC_tex[xx][yy][1] = rgb_sum[0] / counter;
	LIC_tex[xx][yy][2] = rgb_sum[0] / counter;


	LIC_tex_backup[xx][yy][0] = rgb_sum[0] / counter;
	LIC_tex_backup[xx][yy][1] = rgb_sum[0] / counter;
	LIC_tex_backup[xx][yy][2] = rgb_sum[0] / counter;

}
void init_grids();

/**
 * @brief obsolete LIC streamline tracing method
 * 
 * @param xx 
 * @param yy 
 */
void trace_streamline_old(int xx, int yy) {

	float y = xx + .5;
	float x = yy + .5;
	int next_x = xx;
	int next_y = yy;

	int last_x = xx, last_y = y;

	float rgb_sum[3];
	rgb_sum[0] = 0;
	rgb_sum[1] = 0;
	rgb_sum[2] = 0;
	int counter = 0;
	for (int i = 0; i < STREAMLINE_LENGTH / 2; i++) {


		float vx, vy;

		vx = vx_min + (vx_max - vx_min) * vec_img[next_x][next_y][0] / 255.0;
		vy = vy_min + (vy_max - vy_min) * vec_img[next_x][next_y][1] / 255.0;

		vx *= 8;
		vy *= 8;

		//std::cout << vx << ", " << vy <<".";
		x = x + vx;
		y = y + vy;

		next_x = int(x);
		next_y = int(y);
		if (next_x == last_x && next_y == last_y)
			continue;

		last_x = next_x;
		last_y = next_y;

		rgb_sum[0] += noise_tex[next_x][next_y][0];
		rgb_sum[1] += noise_tex[next_x][next_y][1];
		rgb_sum[2] += noise_tex[next_x][next_y][2];
		counter++;
		//std::cout << x << ", " << y << "|";

		if (next_x < -1 || next_x > IMG_RES ||
			next_y < -1 || next_y > IMG_RES) {
			break;
		}
	}

	LIC_tex[xx][yy][0] = rgb_sum[0] / counter;
	LIC_tex[xx][yy][1] = rgb_sum[1] / counter;
	LIC_tex[xx][yy][2] = rgb_sum[2] / counter;

	//std::cout << rgb_sum[0] / counter << ", " << rgb_sum[1] / counter << ", " << rgb_sum[2] / counter<<std::endl;
}

/**
 * @brief render the vector field onto a texture for scaling to 512x512
 * 
 * @param this_poly the read vetor field
 */
void render_vec_img(Polyhedron* this_poly)
{
	std::cout << "MM" << vy_max << ", " << vy_min << std::endl;
	glViewport(0, 0, (GLsizei)512, (GLsizei)512);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 1, 0, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawBuffer(GL_BACK);
	int i, j;
	// first search the max_vx, min_vx, max_vy, min_vy through the entire field
	//DONE
		// render the mesh
	if (false) {
		for (i = 0; i < this_poly->ntris; i++) {
			Triangle* temp_t = this_poly->tlist[i];
			float rgb[3];
			rgb[2] = 0.5;
			glBegin(GL_TRIANGLES);
			for (j = 0; j < 3; j++)
			{
				Vertex* v = temp_t->verts[j];
				//determine the color for this vertex based on its vector value
				float rgb[3];
				rgb[0] = (v->vx - vx_min) / (vx_max - vx_min); // red channel
				rgb[1] = (v->vy - vy_min) / (vy_max - vy_min); // green channel
				rgb[2] = 0;
				//std::cout << rgb[0] << "," << rgb[1]<<std::endl;
				glColor3fv(rgb);
				glVertex2f(v->x, v->y);


				//std::cout << (int)(v->x * 50.0) << ", " << (int)v->y * 50.0 << std::endl;
			}
			glEnd();
		}
	}
	else {
		for (i = 0; i < this_poly->ntris; i++) {
			Triangle* temp_t = this_poly->tlist[i];
			glBegin(GL_TRIANGLES);
			for (j = 0; j < 3; j++)
			{
				Vertex* v1 = temp_t->verts[j];
				int xx = round(v1->x * 49),
					yy = round(v1->y * 49);
				node_main* v = &grid_main[xx][yy];
				float rgb[3];
				rgb[0] = (v->vx_sl - vx_min) / (vx_max - vx_min); // red channel
				rgb[1] = (v->vy_sl - vy_min) / (vy_max - vy_min); // green channel
				rgb[2] = 0;
				//std::cout << v->vx_truth << ", " << v->vy_truth << std::endl;
				glColor3fv(rgb);
				glVertex2f(v->x / 50.0, v->y / 50.0);
				//std::cout << v->vx_truth / 50.0 << ", " << v->vy_truth / 50.0 << std::endl;
			}
			glEnd();
		}
	}
	// save the rendered image into the vec_img
	glReadBuffer(GL_BACK);
	//glReadPixels(0, 0, 50, 50, GL_RGB, GL_UNSIGNED_BYTE, data_img);
	glReadPixels(0, 0, 512, 512, GL_RGB, GL_UNSIGNED_BYTE, vec_img);


	//int c = 0;
	//for (int xx = 0; xx < IMG_RES; xx++)
	//	for (int yy = 0; yy < IMG_RES; yy++) {
	//		if (vec_img[xx][yy][0] == 0)
	//			c++;
	//		//std::cout << (int)vec_img[xx][yy][0] << ", " <<(int) vec_img[xx][yy][1]<<", " << (int)vec_img[xx][yy][2]<<", ";
	//		//trace_streamline(xx, yy);

	//	}

	//std::cout << "percentage is " << (c / (512. * 512.))*100. << std::endl;
	//std::cout << "finished";
}

/**
 * @brief main function for setting up critical values of the vector field reconstruction process
 * It initializes average error values, the reconstructed vector field values, and begin the vector field construction process
 * 
 */
void init_grids() {
	//return;
	vx_min = 1000000, vx_max = -1000000,
		vy_min = 1000000, vy_max = -1000000;
	float avg_size = 0;

	for (int i = 0; i < poly->ntris; i++) {
		Triangle* temp_t = poly->tlist[i];
		for (int j = 0; j < 3; j++)
		{
			Vertex* v = temp_t->verts[j];
			int xx = round(v->x * 49),
				yy = round(v->y * 49);
			data_img[xx][yy][0] = v->vx;
			data_img[xx][yy][1] = v->vy;
			data_check[xx][yy] = true;
		}
	}

	for (int x = 0; x < 50; x++)
		for (int y = 0; y < 50; y++)
		{
			if (!data_check[x][y])
				std::cout << "!!" << x << ", " << y << std::endl;
		}

	std::cout << "vx mm: " << vx_min << ", " << vx_max << std::endl;
	max_err_mag_sl = -100000;
	min_err_mag_sl = 100000;
	max_err_mag_offset = -100000;
	min_err_mag_offset = 1000000;
	max_err_dir_sl = -100000;
	min_err_dir_sl = 1000000;
	max_err_dir_offset = -100000;
	min_err_dir_offset = 1000000;

	max_err_vx_sl = -100000;
	min_err_vx_sl = 100000;
	max_err_vy_offset = -100000;
	min_err_vy_offset = 100000;
	max_err_vxy_sl = -100000;
	min_err_vxy_sl = 100000;

	for (int i = 0; i < 50; i++)
		grid_main[i] = new node_main[50];

	float xyerr = 0, mag_err = 0, dir_err = 0;

#pragma omp parallel for
	for (int xx = 0; xx < 50; xx += 1) {
		for (int yy = 0; yy < 50; yy += 1) {
			node_main* n = new node_main();
			n->x = xx;
			n->y = yy;
			get_vector_truth(xx, yy, n->vx_truth, n->vy_truth);
			n->mag_truth = sqrt(n->vx_truth * n->vx_truth + n->vy_truth * n->vy_truth);
			n->dir_truth = atan(n->vy_truth / n->vx_truth);

			//std::cout << n->vx_truth << ", " << n->vy_truth << ", " << n->mag_truth << std::endl;
			//sl_grid
			interpolate_streamline_selective(xx, yy, n->vx_sl, n->vy_sl, 6);
			n->mag_sl = sqrt(n->vx_sl * n->vx_sl + n->vy_sl * n->vy_sl);
			//std::cout << n->vx_sl<<", "<< n->vy_sl<<", "<< n->mag_sl<<std::endl;
			n->err_mag_sl = abs(n->mag_truth - n->mag_sl);
			mag_err += n->err_mag_sl;
			n->err_vx_sl = abs(n->vx_truth - n->vx_sl);
			n->err_vy_sl = abs(n->vy_truth - n->vy_sl);
			n->err_vxy_sl = abs(n->vx_truth - n->vx_sl) + abs(n->vy_truth - n->vy_sl);
			xyerr += n->err_vxy_sl;
			max_err_vx_sl = max(n->err_vx_sl, max_err_vx_sl);
			min_err_vx_sl = min(n->err_vx_sl, min_err_vx_sl);
			max_err_vy_sl = max(n->err_vy_sl, max_err_vy_sl);
			min_err_vy_sl = min(n->err_vy_sl, min_err_vy_sl);
			max_err_vxy_sl = max(n->err_vxy_sl, max_err_vxy_sl);
			min_err_vxy_sl = min(n->err_vxy_sl, min_err_vxy_sl);

			max_err_mag_sl = max(n->err_mag_sl, max_err_mag_sl);
			min_err_mag_sl = min(n->err_mag_sl, min_err_mag_sl);
			vx_min = min(n->vx_sl, vx_min);
			vx_max = max(n->vx_sl, vx_max);
			vy_min = min(n->vy_sl, vy_min);
			vy_max = max(n->vy_sl, vy_max);

			n->dir_sl = atan(n->vy_sl / n->vx_sl);
			n->err_dir_sl = abs(n->dir_truth - n->dir_sl);
			dir_err += n->err_dir_sl;
			max_err_dir_sl = max(n->err_dir_sl, max_err_dir_sl);
			min_err_dir_sl = min(n->err_dir_sl, min_err_dir_sl);

			//offset_sl grid
			interpolate_streamline(xx + grid_offset_x, yy + grid_offset_y, n->vx_offset_sl, n->vy_offset_sl, 6);
			n->mag_offset_sl = sqrt(n->vx_offset_sl * n->vx_offset_sl + n->vy_offset_sl * n->vy_offset_sl);
			n->dir_offset_sl = atan(n->vy_offset_sl / n->vx_offset_sl);


			grid_main[xx][yy] = *n;

			std::cout << "progress: " << xx << ", " << yy << std::endl;
		}

	}
	//std::cout << "AVG SIZE: " << avg_size / 2500 << std::endl;
	std::cout << "MAG ERR: " << mag_err / 2500 << std::endl;
	std::cout << "DIR ERR: " << dir_err / 2500 << std::endl;
	std::cout << "XY ERR: " << xyerr / 2500 << std::endl;
	//bilinear
	for (int xx = 0; xx < 49; xx += 1) {
		for (int yy = 0; yy < 49; yy += 1) {
			node_main* n11 = &grid_main[xx][yy],
				* n21 = &grid_main[xx + 1][yy],
				* n12 = &grid_main[xx][yy + 1],
				* n22 = &grid_main[xx + 1][yy + 1];
			bilinear_interpolate_node(n11, n21, n22, n12);
			n11->mag_offset = sqrt(n11->vx_offset * n11->vx_offset + n11->vy_offset * n11->vy_offset);
			n11->err_mag_offset = abs(n11->mag_offset - n11->mag_offset_sl);
			max_err_mag_offset = max(n11->err_mag_offset, max_err_mag_offset);
			min_err_mag_offset = min(n11->err_mag_offset, min_err_mag_offset);

			n11->dir_offset = atan(n11->vy_offset / n11->vx_offset);
			n11->err_dir_offset = abs(n11->dir_offset - n11->dir_offset_sl);
			max_err_dir_offset = max(n11->err_dir_offset, max_err_dir_offset);
			min_err_dir_offset = min(n11->err_dir_offset, min_err_dir_offset);

			n11->err_vx_offset = abs(n11->vx_truth - n11->vx_offset);
			n11->err_vy_offset = abs(n11->vy_truth - n11->vy_offset);
			max_err_vx_offset = max(n11->err_vx_offset, max_err_vx_offset);
			min_err_vx_offset = min(n11->err_vx_offset, min_err_vx_offset);
			max_err_vy_offset = max(n11->err_vy_offset, max_err_vy_offset);
			min_err_vy_offset = min(n11->err_vy_offset, min_err_vy_offset);
		}
	}

	std::cout << "max_err_mag_sl: " << max_err_mag_sl << std::endl;
	std::cout << "min_err_mag_sl: " << min_err_mag_sl << std::endl;
	std::cout << "avg v diff: " << v_diff_average / v_diff_counter << std::endl;

}

/**
 * @brief set the values of the interpolated grid based on the original grid
 * 
 */
void interpolate_grid() {
	for (int xx = 0; xx < IMG_RES; xx += gridX) {
		for (int yy = 0; yy < IMG_RES; yy += gridY) {
			node n;
			n.x = xx;
			n.y = yy;
		}
	}
}

/**
 * @brief compute the LIC image
 * 
 * @param enhance 
 */
void compute_LIC(int enhance) {
	//return;
	//lazy_seeding();
	init_grids();
	render_vec_img(poly);
	//lazy_seeding();
#pragma omp parallel for
	for (int xx = 0; xx < 512; xx++) {
		for (int yy = 0; yy < 512; yy++) {
			trace_streamline(xx, yy, enhance);
		}
	}

}

/**
 * @brief uniform streamline seeding method that stops tracing the streamline when it gets too cloest to another streamline
 * 
 */
void lazy_seeding() {
	streamlines.clear();
	int min_dis = 10;
	for (int xx = 0; xx < 512; xx += 50) {
		for (int yy = 0; yy < 512; yy += 50) {
			float dis = min_distance_to_all_steamlines(yy, xx);
			//std::cout << dis << std::endl;
			if (dis < min_dis)
				continue;
			std::vector<node>* streamline = trace_streamline_custom(xx, yy, 20);
			if (streamline->size() > 2)
				streamlines.push_back(*streamline);
		}
	}
}