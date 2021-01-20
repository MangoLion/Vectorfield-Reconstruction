#ifndef rec_reader
#define rec_reader
#include <string>
#include <vector>
#include <Eigen/Dense>
#include <Eigen/Eigenvalues>
#include "../structures.h"
using namespace Eigen;
void trace_sample_streamline(std::vector<std::vector<node>> &streamlines);
void change_profile(int index);
void readStreamlines(std::vector<std::vector<node>> &streamlines, std::string name, int skipNodes, int skipLines);
void readStreamlines(std::vector<std::vector<node>> &streamlines, std::vector<std::vector<Vector3f>> streamlinesV3);
#endif