/*
 * mainApp.cpp
 *
 *  Created on: Dec 27, 2015
 *      Author: apollo
 */

#include "mechanicController.h"
#include "global.h"

#include "hwStepperPins.h"
#include "servo.h"
#include "Timer.h"
#include "config.h"

#include "stepperControl_main.h"
#include "math_tools.h"

MechanicController mechanicController;

PlotterArm plotterArms[] =
{
		{{MIN_ANGLE, MAX_ANGLE, LEFT_ARM_OFFSET}},
		{{MIN_ANGLE, MAX_ANGLE, RIGHT_ARM_OFFSET}}
};

Servo servos[] =
{
		{1}
};

bool isSystemReady()
{
	return mechanicController.calibrationFinished();
}

void fakeInit(void)
{
	while(plotterArms[0].Calibrate() != true)
	{
		plotterArms[0].moveStart();
		plotterArms[0].moveEnd();
		/*
		LOG("-------");
		LOG("calibration state:" << plotterArms[0].calibrationState);
		LOG("summError:" << plotterArms[0].sumError);
		LOG("maxAngle:" << plotterArms[0].maxAngle);
		LOG("maxStepperValue:" << plotterArms[0].maxStepperValue);
		LOG("currenctStepCount:" << plotterArms[0].currentStepCount);
		LOG("setpointStepperValue:" << plotterArms[0].setpointStepperValue);
		*/


	}
	while(1)
	{
	for (int i = 0; i < 11; i++)
	{
		plotterArms[0].OnUpdate((float)i/10, true);
		while(plotterArms[0].getError() != 0)
		{
			plotterArms[0].OnUpdateRegulation();
			plotterArms[0].moveStart();
			plotterArms[0].moveEnd();
		}
	}
	}
}

void stepperControl_init(void)
{
	plotterArms[0].registerGPIOs(getStepperGPIOs(0));
	plotterArms[0].registerLimitSwitchGPIOs(getLimitSwitchGPIOs(0));

	plotterArms[1].registerGPIOs(getStepperGPIOs(1));
	plotterArms[1].registerLimitSwitchGPIOs(getLimitSwitchGPIOs(1));

	mechanicController.registerArms(&plotterArms[0]);
	mechanicController.registerArms(&plotterArms[1]);

	//mechanicController.registerArms(plotterArms, ARRAY_SIZE(plotterArms));
    mechanicController.registerServos(servos, ARRAY_SIZE(servos));

    mechanicController.calibrate(true);

    while(!mechanicController.calibrationFinished())
    {
    		mechanicController.calibrate(false);
    }

    LOG("Calibration finished!");
}


/* Creates control command for motor
 *
 * Finds required intersections and sets arms to required positions C
 *
 * @param[in]  C           required position
 * @param[out] outputCmd   result
 * @return     bool        True if intersection was found, False if not
 * out - arm command for motor
 *
 */
bool calculateArmPosition(position finalPosition, float extrudeLength, armCommand& outputCmd)
//bool createArmCommand(position C, armCommand& outputCmd)
{
	/*
	 *           C
	 *           /\
	 *          /  \
	 *         /    \
	 *        /      \
	 *       A        B
	 *        \      /
	 *         \    /
	 *          \  /
	 *          S1 S2
	 *
	 */

	// C = Required position

    position A1,A2;
    position B1,B2;

    bool result = true;

    result = result and getIntersection(finalPosition, armLength_AC, pos_S1, armLength_AS1, A1, A2);
    result = result and getIntersection(finalPosition, armLength_BC, pos_S2, armLength_BS2, B1, B2);

    if (result)
    {
        position A = (A1.x < A2.x) ? A2 : A1;
    	position B = (B1.x < B2.x) ? B1 : B2;

    	float angle1 = getAngle(pos_S1, {pos_S1.x+100, pos_S1.y, pos_S1.z}, A);
    	float angle2 = getAngle(pos_S2, {pos_S2.x+100, pos_S2.y, pos_S2.z}, B);

    	// Fill only arm positions
    	outputCmd.angle1 = angle1;
    	outputCmd.angle2 = angle2;
        outputCmd.relPosZ = zAxeToRelative(finalPosition.z);
        outputCmd.extrudeLength = extrudeLength;
    	return true;
    }

    return false;
}

// TODO add speed
void stepperControl_goToThisPosition(position newPosition,float extrudeLength)
{
	armCommand armCmd;

	if (calculateArmPosition(newPosition, extrudeLength, armCmd))
	{
	    mechanicController.OnUpdateAll(armCmd);
	    mechanicController.OnMove();
	}
	else
	{
		LOG("Unable to navigate to this position!");
	}
}

