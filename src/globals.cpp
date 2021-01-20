#include "globals.h"

/// <summary>
///trackers for average difference in vector values
/// </summary>
///@{ 

float v_diff_average = 0, v_diff_counter = 0, angle_diff_avg = 0;
int angle_diff_c = 0, avg_angle_diff = 0, angle_diff_counter = 0;
float stdev_global = 0;
///@}
/// <summary>
/// global parameters for use in all reconstruction methods
/// </summary>
bool RECON_USE_NEIGHBORS,
RECON_RECOUNT_AVG;
int RECON_K_NEIGHBORS, RECON_EXCLUDED = 0;
float RECON_EXCLUDE_THRESHOLD;
float P_RADAR, P_RADAR_LEVEL;
bool P_NORMALIZE;
float P_THRESHOLD_ANGLE;
float P_THRESHOLD_VELOCITY;
bool P_USE_SEGMENTS;
int P_SKIPLINES, P_SKIPNODES;
std::string P_DATASET;
std::string P_TESTID,
P_TESTPARAMS;