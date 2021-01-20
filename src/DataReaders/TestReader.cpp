#include "TestReader.h"
#include "./../globals.h"
#include <fstream>
#include <iostream>
#include <sstream>

using namespace std;

string HEADER;

/// <summary>
/// The first line in the test file shows what variable each column represents. We set it as the header here
/// </summary>
/// <param name="header"></param>
void setTestHeader(string header) {
	HEADER = header;
}

/// <summary>
/// Based on the HEADER line, we reach each following line and assign the correct value to the variable name
/// </summary>
/// <param name="line"></param>
void readTestNextLine(string line) {
	istringstream SSHeader(HEADER),
			SSLine(line);
	string varname;
	while (SSHeader >> varname) {
		if (varname == "TESTID") {
			int testid;
			SSLine >>testid;
		}
		if (varname == "DATASET") {
			SSLine >> P_DATASET;
		}
		if (varname == "SKIPLINES") {
			SSLine >> P_SKIPLINES;
		}
		if (varname == "SKIPNODES") {
			SSLine >> P_SKIPNODES;
		}
		if (varname == "EXCLUDE_THRESHOLD") {
			SSLine >> RECON_EXCLUDE_THRESHOLD;
		}
		if (varname == "K_NEIGHBORS") {
			SSLine >> RECON_K_NEIGHBORS;
		}
		if (varname == "USE_NEIGHBORS") {
			int use;
			SSLine >> use;
			if (use == 1)
				RECON_USE_NEIGHBORS = true;
			else
				RECON_USE_NEIGHBORS = false;
		}
		if (varname == "RADAR") {
			SSLine >> P_RADAR_LEVEL;
		}
	}
}