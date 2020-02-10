#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "Reader.h"
#include "Graph.h"
#include "structures.h"
#include "globals.h"
#include <cmath>


/**
 * @brief Read the streamline file, populate the streamlines vetor and put streamline nodes into bins
 * 
 * @param fname file name
 */
void read_streamline(std::string fname) {
	float average_step = 0;
	int nodes = 0;
	//return;
	streamlines.clear();
	std::ifstream file;
	file.open(fname);
	std::string line;
	int counter = 0,
		streamline_index = 0,
		strealine_segment_index = 0;

	int total_nodes = 0;
	int skip_line = 0, skip_node = 0;
	bool adaptive_read = true;
	//flag to add third node only, since first node has no vx vy

	while (std::getline(file, line)) {
		node* last_n = nullptr;
		skip_line++;
		int scounter = 0;
		if (skip_line % 2 != 0)
			continue;
		std::vector<node>streamline;
		std::stringstream ss(line);
		float temp;
		float dvx, dvy;
		int dvx_sign, dvy_sign;
		bool start_read = true,
			crit_point = false;
		//indicator of the sign of the last derivative value, used to detect a change in direction (+ to - or vice versa)
		int dv_sign;
		int n_index = 0, last_added_index = 0;

		//temporary save nodes between feature points, used for adaptive SL reading
		std::vector<node>node_cache;
		while (ss >> temp) {
			counter++;
			if (counter < skip_node) {
				continue;
			}
			
			counter = 0;

			node* n = new node();
			n->x = temp;
			ss >> n->y;
			//ignore z
			ss >> temp;
			//std::cout << n->x << "," << n->y << ", " << temp << std::endl;

			n->x *= 50.;
			n->y *= 50.;
			int binx = (int)n->x % 5,
				biny = (int)n->y % 5;


			//exclude first node since no vx vy
			if (last_n != nullptr) {
				n->vx = n->x - last_n->x;
				n->vy = n->y - last_n->y;

				if (adaptive_read)
					if (start_read) {
						start_read = false;
						bins[binx][biny].push_back(n);
						streamline.push_back(*n);
						node_cache.push_back(*n);

						dvx = n->vx;
						dvy = n->vy;
						dvx_sign = dvx / dvx;
						dvy_sign = dvy / dvy;
					}
					else {
						dvx = n->vx - dvx;
						dvy = n->vy - dvy;
						int new_dvx_sign = dvx / dvx,
							new_dvy_sign = dvy / dvy;
						if (dvx_sign != new_dvx_sign || dvy_sign != new_dvy_sign) {
							crit_point = true;
						}

						dvx_sign = new_dvx_sign;
						dvy_sign = new_dvy_sign;
					
					}

				average_step += sqrt(n->vx * n->vx + n->vy * n->vy);
				nodes++;

				if (!adaptive_read) {
					bins[binx][biny].push_back(n);
					streamline.push_back(*n);
					
				}
				else {
					n_index++;
					node_cache.push_back(*n);
					if (crit_point) {
						std::cout << "crit!" << std::endl;
						crit_point = false;
						//add the middle node between this feature point and the last node that was added
						node* middle_n = (&node_cache[(n_index - last_added_index)/2]);
						bins[binx][biny].push_back(middle_n);
						streamline.push_back(*middle_n);

						//add this feature node
						bins[binx][biny].push_back(n);
						streamline.push_back(*n);

						last_added_index = n_index;
						node_cache.clear();
					}
				}
			}
			last_n = n;
			
		}
		/*node* n1 = &streamline[0],
			* n3 = &streamline[streamline.size()/2],
			*n2 = &streamline[streamline.size()-1];
		int size = streamline.size();
		streamline.clear();
		streamline.push_back(*n1);
		if (size > 1) {
			streamline.push_back(*n3);
			streamline.push_back(*n2);
		}*/

		//std::cout << "sscounter: " << scounter << std::endl;
		if (streamline.size() > 1) {
			streamlines.push_back(streamline);

			streamline_index++;
		}
	}

	std::cout << "total nodes: " << total_nodes << std::endl;
	file.close();
	//update_graph();
}