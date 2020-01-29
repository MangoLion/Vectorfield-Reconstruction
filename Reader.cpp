#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include "Reader.h"
#include "Graph.h"
#include "structures.h"
#include "globals.h"

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
	int skip_line = 0;
	//flag to add third node only, since first node has no vx vy

	while (std::getline(file, line)) {
		node* last_n = nullptr;
		segment* last_s = nullptr;
		skip_line++;
		int scounter = 0;
		if (skip_line % 2 != 0)
			continue;
		bool new_segment_start = true;
		std::vector<node>streamline;
		std::stringstream ss(line);
		float temp;
		while (ss >> temp) {
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
				average_step += sqrt(n->vx * n->vx + n->vy * n->vy);
				nodes++;


				if (counter > 0) {
					bins[binx][biny].push_back(n);
					streamline.push_back(*n);
					counter = 0;
					if (streamline.size() > 2) {
						segment* s = new segment;
						s->start = last_n;
						//std::cout << s->start->x << std::endl;
						s->end = n;
						s->streamline_index = streamline_index;
						s->streamline_segment_index = strealine_segment_index;

						node* n_middle = new node;
						n_middle->x = last_n->x + last_n->vx / 2;
						n_middle->y = last_n->y + last_n->vy / 2;
						n_middle->vx = last_n->vx;
						n_middle->vy = last_n->vy;

						s->middle = n_middle;
						if (last_s != nullptr) {
							s->previous = last_s;
							last_s->next = s;
							last_s = s;
							scounter++;
							total_nodes++;
						}
						strealine_segment_index++;
						if (new_segment_start) {
							segment_starters.push_back(s);
							new_segment_start = false;
							last_s = s;
						}

						//add to bin
						binx = (int)n->x % bin_width_segment;
						biny = (int)n->y % bin_width_segment;
						bins_segment[binx][biny].push_back(s);
					}
				}
				counter++;
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