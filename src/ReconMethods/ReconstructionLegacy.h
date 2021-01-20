#ifndef vec_rec_leg
#define vec_rec_leg

int interpolate_streamline_adaptive_weight_sum(float xx, float yy, float& vx, float& vy, float radar);
int interpolate_streamline_simple_weighted_sum(float xx, float yy, float& vx, float& vy, float radar);
int interpolate_streamline_raycast(float xx, float yy, float& vx, float& vy, float radar);
#endif // !vec_lic
