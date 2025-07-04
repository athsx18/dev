#ifndef UTILS_H
#define UTILS_H

// ---------- Interpolation Functions ----------
float interpolate(float x0, float x1, float y0, float y1, float x);

// ---------- Compensation Factor ----------
float getCompensationFactor(float T, float H, float raw);

// ---------- Grids and Table ----------
extern const float T_grid[3];
extern const float H_grid[2];
extern const float raw_grid[5];
extern const float correction_table[3][2][5];

#endif // UTILS_H
