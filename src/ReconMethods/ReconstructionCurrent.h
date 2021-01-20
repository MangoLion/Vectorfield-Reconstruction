#ifndef vec_rec
#define vec_rec
int interpolate_streamline_selective_weighted_sum(float xx, float yy, float zz, float& vx, float& vy, float& vz);
float interpolate_streamline_selective_weighted_sum_stdev(float &stdev, float xx, float yy, float zz, float &vx, float &vy, float &vz);
double getAbsoluteDiff3Angles(float x1, float y1, float z1, float x2, float y2, float z2);
float dot_product(float v1[3], float v2[3]);
float recon_get_stdev(double p[3], float radar);
#endif // !vec_lic
