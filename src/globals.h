#ifndef vec_globals
#define vec_globals
#include "Unused/Skeleton.h"
#include "Unused/kdtree.h"
#include "structures.h"
#include <vtkStructuredPoints.h>

extern std::vector<std::vector<node>> streamlines;
extern std::vector<segment *> segment_starters;
extern segment **all_segments;
extern int total_nodes;
extern float matrix_region_x1, matrix_region_x2, matrix_region_y1, matrix_region_y2, matrix_region_z1, matrix_region_z2;

extern float SL_LENGTH;

extern segment seg_base, *seg_neighbors;
extern int seg_neighbors_num;

extern bool RECON_USE_NEIGHBORS, 
    RECON_RECOUNT_AVG;
extern int RECON_K_NEIGHBORS, RECON_EXCLUDED;
extern float RECON_EXCLUDE_THRESHOLD;


extern float v_diff_average, v_diff_counter, angle_diff_avg ;
extern int angle_diff_c, avg_angle_diff, angle_diff_counter;
extern float stdev_global;

extern float P_RADAR, P_RADAR_LEVEL;
extern bool P_NORMALIZE;
extern float P_THRESHOLD_ANGLE;
extern float P_THRESHOLD_VELOCITY;
extern bool P_USE_SEGMENTS;
extern int P_SKIPLINES, P_SKIPNODES;
extern std::string P_DATASET;
extern std::string P_TESTID,
    P_TESTPARAMS;


//vtk object doesnt like being global, switched back to local
//vtkSmartPointer<vtkStructuredPoints> VFORIGINAL, VFRECON;

//location of two debug segments for legacy viewer (not vtk)
extern float debugA_xyz[3], debugB_xyz[3];

# define M_PI           3.14159265358979323846

#endif
