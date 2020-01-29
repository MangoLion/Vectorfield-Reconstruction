/**
 * @file structures.h
 * @brief Store the global initialization of needed structs by all cpp files
 * @version 0.1
 * @date 2020-01-18
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef rec_structures
#define rec_structures
#include <vector>
struct node;
struct segment;

/*
 * @brief  store data on an adjacent node and it's vector, distance, and direction relative to the current node
 */
struct neighbor {
	segment* seg;
	float vx, vy, angle, dist;
	int direction;
};


/**
 * @brief main unit of the graph representation, stire two streamline nodes and the list of neighboring nodes to the segment
 * 
 */
struct segment {
	node* start, * end, * middle;
	segment* previous = nullptr, * next = nullptr;
	int streamline_index, streamline_segment_index;
	std::vector<neighbor*> neighbors;
};

/**
 * @brief a node_main is a basic struct that represents a point on the vector field. This struct
 * stores all values needed in the reconstructed field, including error values, vector value of the original vector field point, and of the reconstructed value
 * 
 */
struct node_main {
	float x, y,
		vx_truth, vy_truth,
		vx_sl, vy_sl,
		vx_offset, vy_offset,
		vx_offset_sl, vy_offset_sl,
		err_vx_sl, err_vy_sl,
		err_vxy_sl, err_vxy_offset,
		err_vx_offset, err_vy_offset,
		mag_truth, mag_sl, mag_offset, mag_offset_sl,
		err_mag_sl, err_mag_offset,
		dir_truth, dir_sl, dir_offset, dir_offset_sl,
		err_dir_sl, err_dir_offset,
		weight = 0;
};


/**
 * @brief Simple node struct that only sture the vector value on the field
 * 
 */
struct node {
	float x, y, vx, vy, mag, dis = 0, weight, err;
};

/**
 * @brief Used to trace LIC texture, one streamlet backward and one forward
 * 
 */
struct streamlet {
	std::vector<node*> nodes;
	float weighted_sum = 0,
		dist, vx = 0, vy = 0,
		ttl_dist = 0;
};
#endif