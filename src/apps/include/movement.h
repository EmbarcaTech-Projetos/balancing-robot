#ifndef MOVEMENT_H_ 
#define MOVEMENT_H_

#include "hal.h"
#include <stdlib.h>

typedef struct constants
{
    float kp, kd, ki;
    float complementary_filter_alpha;
    uint16_t deadzone;
} constants;

void imu_data_process(hal_imu_data_t *imu_data, hal_imu_data_t *initial_imu_data, constants *constants_variable);

#endif