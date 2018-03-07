/*
 * stepperController.cpp
 *
 *  Created on: Dec 26, 2015
 *      Author: apollo
 */

#include "mechanicController.h"
#include "building_blocks.h"
#include "Timer.h"
#include "global.h"

#include "math_tools.h"
#include <cstdlib>
#include "stepperConfig.h"

#define SET_EACH_MEMBER_TO_NULL(X) 	for (int i =0; i < ARRAY_SIZE((X));i++){(X)[i] = NULL;}


MechanicController::MechanicController()
:bCalibrationFinished(false),
armObjectCount(0),
stepperObjectCount(0),
limStepperObjectCount(0),
servoObjectCount(0)
{
	SET_EACH_MEMBER_TO_NULL(armObjectArray);
	SET_EACH_MEMBER_TO_NULL(stepperObjectArray);
	SET_EACH_MEMBER_TO_NULL(limStepperObjectArray);
	SET_EACH_MEMBER_TO_NULL(servoObjectArray);
};


void MechanicController::registerArms(PlotterArm * pArms)//, int armsCount)
{
	if ((pArms != NULL) and (armObjectCount <= cMaxNumberOfPlotterArms))
	{
		armObjectArray[armObjectCount++] = pArms;
	}
	else
	{
		assert(false);
	}
}

void MechanicController::registerServos(Servo * pServos, int servosCount)
{
	if ((pServos != NULL) and (servosCount <= cMaxNumberOfServos))
	{
		for (int i = 0; i < servosCount; i++)
		{
			servoObjectArray[i] = &(pServos[i]);
		}
		servoObjectCount = servosCount;
	}
	else
	{
		assert(false);
	}
}


void MechanicController::registerSteppers(Stepper * pSteppers, int steppersCount)
{
	if ((pSteppers != NULL) and (steppersCount <= cMaxNumberOfSteppers))
	{
		for (int i = 0; i < steppersCount; i++)
		{
			// TODO  fix this horrible solution - use pointer to pointere
			stepperObjectArray[i] = &(pSteppers[i]);
		}
		stepperObjectCount = steppersCount;
	}
	else
	{
		assert(false);
	}
}


void MechanicController::OnUpdateAll(armCommand command)
{
	// Add better command/setting
	/* struct
	 * {
	 *		angle1, angle2, angle3, .....
	 *		numberOfUsedPlotter arms
	 *		float relativePos1, relativePos2, relativePos3,
	 *		numberOf used steppers with limits
	 *		stepCount, stepCount, stepCount, ....
	 *		number of used servos
	 * }
	*/

	// TODO Unify usage of steppers - use each of them or use an array

	PlotterArmSetting leftArm = {command.angle1, true};
	PlotterArmSetting rightArm = {command.angle2, true};
	ServoSetting servoSetting = {command.relPosZ, (bool)command.extrudeLength };

	if (this->armObjectCount > 0)
	{
		armObjectArray[0]->NewPosition(leftArm); // left
	}

	if (this->armObjectCount > 1)
	{
		armObjectArray[1]->NewPosition(rightArm); // right
	}

	//assert(this->armObjectCount >= 2 and armObjectArray != NULL);
	//armObjectArray[1]->NewPosition(rightArm); // right

	//assert(this->servoObjectCount >= 1 and servoObjectArray != NULL);
	if (this->servoObjectCount > 1)
	{
		servoObjectArray[0]->OnUpdate(&servoSetting);
	}
}

int MechanicController::GetMaxStepperError(void)
{
    int maxError = 0;

	for(int idx = 0; idx < this->armObjectCount; idx++)
	{
	    int error = abs(this->armObjectArray[idx]->getError());
        maxError = (maxError < error)? error: maxError;
	}

	for(int idx = 0; idx < this->stepperObjectCount; idx++)
	{
	    int error = abs(this->stepperObjectArray[idx]->getError());
        maxError = (maxError < error)? error: maxError;
	}

	for(int idx = 0; idx < this->limStepperObjectCount; idx++)
	{
	    int error = abs(this->limStepperObjectArray[idx]->getError());
        maxError = (maxError < error)? error: maxError;
	}
    return maxError;
}

void MechanicController::updateRegulationOnAll(void)
{
	// Update regulation on all object of type stepper or similar
	for(int idx = 0; idx < this->armObjectCount; idx++)
	{
		this->armObjectArray[idx]->OnUpdateRegulation();
	}

	for(int idx = 0; idx < this->stepperObjectCount; idx++)
	{
		this->stepperObjectArray[idx]->OnUpdateRegulation();
	}

	for(int idx = 0; idx < this->limStepperObjectCount; idx++)
	{
		this->limStepperObjectArray[idx]->OnUpdateRegulation();
	}
}


void MechanicController::moveWithEach(void)
{
	// Start step pulse
    for(int idx = 0; idx < this->armObjectCount; idx++)
    {
        this->armObjectArray[idx]->moveStart();
    }

    for(int idx = 0; idx < this->stepperObjectCount; idx++)
    {
        this->stepperObjectArray[idx]->moveStart();
    }

    for(int idx = 0; idx < this->limStepperObjectCount; idx++)
    {
        this->limStepperObjectArray[idx]->moveStart();
    }

    Timer::sleep(2); //Sleep in ms

    // End step pulse
    for(int idx = 0; idx < this->armObjectCount; idx++)
    {
        this->armObjectArray[idx]->moveEnd();
    }

    for(int idx = 0; idx < this->stepperObjectCount; idx++)
    {
        this->stepperObjectArray[idx]->moveEnd();
    }

    for(int idx = 0; idx < this->limStepperObjectCount; idx++)
    {
        this->limStepperObjectArray[idx]->moveEnd();
    }

    //TODO maybe remove delay
    Timer::sleep(10); //Sleep in ms
}

void MechanicController::OnMove(void)
{
    // Update all servo motors
    for(int i = 0; i < this->servoObjectCount; i++)
    {
        this->servoObjectArray[i]->OnMove();
    }
    //{&servo}),
    // Main logic for moving with steppers
    int maxError = this->GetMaxStepperError();

    for (int i = 0; i < maxError; i++)
    {
    	this->updateRegulationOnAll();
    	this->moveWithEach();
    }
}

bool MechanicController::calibrationFinished()
{
	return this->bCalibrationFinished;
}

void MechanicController::calibrate(bool startNewCalibration)
{
	if (startNewCalibration)
	{
		this->bCalibrationFinished = false;
	}
    //bool calibrationFinished = false;

    if (false == this->bCalibrationFinished)
    {
        bool bIsCalibrationDone = true;
        for (int i = 0; i < this->armObjectCount; i++)
        {
            bool result = this->armObjectArray[i]->Calibrate();
            bIsCalibrationDone &= result;
        }

        for (int i = 0; i < this->limStepperObjectCount; i++)
        {
            bool result = this->limStepperObjectArray[i]->Calibrate();
            bIsCalibrationDone &= result;
        }

        this->bCalibrationFinished = bIsCalibrationDone;

    	this->moveWithEach();
/*
        for(int i = 0; i < this->armObjectCount; i++)
        {
            this->armObjectArray[i]->moveStart();
        }

        Timer::sleep(2); //Sleep in ms

        for(int i = 0; i < this->armObjectCount; i++)
        {
            this->armObjectArray[i]->moveEnd();
        }
*/
        //TODO maybe remove delay
        //Timer::sleep(10); //Sleep in ms
    }
}
