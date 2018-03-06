/*
 * stepperConfig.h
 *
 *  Created on: Dec 26, 2015
 *      Author: apollo
 */

#ifndef STEPPERCONFIG_H_
#define STEPPERCONFIG_H_

#include "global.h"

typedef struct
{
    float setpointAngle;
    bool enable;
} PlotterArmSetting;

typedef struct
{
	uint32_t value; // Value in range 0.000 - 100.000% (thousandths of a percent)
    bool enable;
} ServoSetting;

typedef struct
{
	float angleMin;
	float angleMax;
	float angleOffset;
} PlotterArmConfig;

#endif /* STEPPERCONFIG_H_ */
