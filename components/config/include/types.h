#ifndef TYPES_H
#define TYPES_H

// ---------- Sensor Data Structure ----------
struct SensorData {
    float rawPPM = 0;
    float correctedPPM = 0;
    float compFactor = 1.0;
};

#endif // TYPES_H
