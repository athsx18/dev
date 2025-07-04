#include "utils.h"

// Temperature (°C), Humidity (%), Raw PPM levels
const float T_grid[3] = {20, 30, 40};
const float H_grid[2] = {40, 60};
const float raw_grid[5] = {100, 200, 300, 400, 500};

// Example 3D compensation table
const float correction_table[3][2][5] = {
    {
        {1.00, 0.98, 0.96, 0.95, 0.93},
        {0.97, 0.95, 0.93, 0.91, 0.90}
    },
    {
        {1.05, 1.02, 1.00, 0.98, 0.97},
        {1.00, 0.97, 0.95, 0.93, 0.91}
    },
    {
        {1.08, 1.05, 1.02, 1.00, 0.98},
        {1.02, 1.00, 0.98, 0.95, 0.93}
    }
};

float interpolate(float x0, float x1, float y0, float y1, float x) {
    if (x1 == x0) return y0;
    return y0 + ((x - x0) * (y1 - y0)) / (x1 - x0);
}

float getCompensationFactor(float T, float H, float raw) {
    int t_idx = 0, h_idx = 0, r_idx = 0;
    for (int i = 0; i < 2; ++i) if (T >= T_grid[i]) t_idx = i;
    for (int i = 0; i < 1; ++i) if (H >= H_grid[i]) h_idx = i;
    for (int i = 0; i < 4; ++i) if (raw >= raw_grid[i]) r_idx = i;

    float c000 = correction_table[t_idx][h_idx][r_idx];
    float c001 = correction_table[t_idx][h_idx][r_idx+1];
    float c010 = correction_table[t_idx][h_idx+1][r_idx];
    float c011 = correction_table[t_idx][h_idx+1][r_idx+1];
    float c100 = correction_table[t_idx+1][h_idx][r_idx];
    float c101 = correction_table[t_idx+1][h_idx][r_idx+1];
    float c110 = correction_table[t_idx+1][h_idx+1][r_idx];
    float c111 = correction_table[t_idx+1][h_idx+1][r_idx+1];

    float t0 = T_grid[t_idx], t1 = T_grid[t_idx+1];
    float h0 = H_grid[h_idx], h1 = H_grid[h_idx+1];
    float r0 = raw_grid[r_idx], r1 = raw_grid[r_idx+1];

    float c00 = interpolate(r0, r1, c000, c001, raw);
    float c01 = interpolate(r0, r1, c010, c011, raw);
    float c10 = interpolate(r0, r1, c100, c101, raw);
    float c11 = interpolate(r0, r1, c110, c111, raw);

    float c0 = interpolate(h0, h1, c00, c01, H);
    float c1 = interpolate(h0, h1, c10, c11, H);

    return interpolate(t0, t1, c0, c1, T);
}
