/*
 * This file is part of the µOS++ distribution.
 *   (https://github.com/micro-os-plus)
 * Copyright (c) 2014 Liviu Ionescu.
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom
 * the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

// ----------------------------------------------------------------------------

#include "diag/Trace.h"

#include "Timer.h"
#include "BlinkLed.h"

#include "Timer.h"
#include "hwGpio.h"
#include "hwStepperPins.h"

#include "reader_main.h"
#include "parser_main.h"
#include "movementControl_main.h"
#include "stepperControl_main.h"


#include "stepperControl_main.h"
#include "building_blocks.h"
#include <cstring> // memset
#include "Serial.h"

#include "global.h"

// ----------------------------------------------------------------------------
//
// Standalone STM32F4 led blink sample (trace via NONE).
//
// In debug configurations, demonstrate how to print a greeting message
// on the trace device. In release configurations the message is
// simply discarded.
//
// Then demonstrates how to blink a led with 1 Hz, using a
// continuous loop and SysTick delays.
//
// Trace support is enabled by adding the TRACE macro definition.
// By default the trace messages are forwarded to the NONE output,
// but can be rerouted to any device or completely suppressed, by
// changing the definitions required in system/src/diag/trace_impl.c
// (currently OS_USE_TRACE_ITM, OS_USE_TRACE_SEMIHOSTING_DEBUG/_STDOUT).
//

// Definitions visible only within this translation unit.
namespace
{
  // ----- Timing definitions -------------------------------------------------

  // Keep the LED on for 2/3 of a second.
  constexpr Timer::ticks_t BLINK_ON_TICKS = Timer::FREQUENCY_HZ * 3 / 4;
  constexpr Timer::ticks_t BLINK_OFF_TICKS = Timer::FREQUENCY_HZ
      - BLINK_ON_TICKS;
}

// ----- LED definitions ------------------------------------------------------

#if defined(STM32F401xE)

#warning "Assume a NUCLEO-F401RE board, PA5, active high."

// PA5
#define BLINK_PORT_NUMBER         (0)
#define BLINK_PIN_NUMBER          (5)
#define BLINK_ACTIVE_LOW          (false)

BlinkLed blinkLeds[1] =
  {
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER, BLINK_ACTIVE_LOW },
  };

#elif defined(STM32F407xx)

#warning "Assume a STM32F4-Discovery board, PD12-PD15, active high."

#define BLINK_PORT_NUMBER         (3)
#define BLINK_PIN_NUMBER_GREEN    (12)
#define BLINK_PIN_NUMBER_ORANGE   (13)
#define BLINK_PIN_NUMBER_RED      (14)
#define BLINK_PIN_NUMBER_BLUE     (15)
#define BLINK_ACTIVE_LOW          (false)

BlinkLed blinkLeds[4] =
  {
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER_GREEN, BLINK_ACTIVE_LOW },
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER_ORANGE, BLINK_ACTIVE_LOW },
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER_RED, BLINK_ACTIVE_LOW },
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER_BLUE, BLINK_ACTIVE_LOW },
  };

#elif defined(STM32F411xE)

#warning "Assume a NUCLEO-F411RE board, PA5, active high."

#define BLINK_PORT_NUMBER         (0)
#define BLINK_PIN_NUMBER          (5)
#define BLINK_ACTIVE_LOW          (false)

BlinkLed blinkLeds[1] =
  {
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER, BLINK_ACTIVE_LOW },
  };

#elif defined(STM32F429xx)

#warning "Assume a STM32F429I-Discovery board, PG13-PG14, active high."

#define BLINK_PORT_NUMBER         (6)
#define BLINK_PIN_NUMBER_GREEN    (13)
#define BLINK_PIN_NUMBER_RED      (14)
#define BLINK_ACTIVE_LOW          (false)

BlinkLed blinkLeds[2] =
  {
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER_GREEN, BLINK_ACTIVE_LOW },
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER_RED, BLINK_ACTIVE_LOW },
  };

#else

#warning "Unknown board, assume PA5, active high."

#define BLINK_PORT_NUMBER         (0)
#define BLINK_PIN_NUMBER          (5)
#define BLINK_ACTIVE_LOW          (false)

BlinkLed blinkLeds[1] =
  {
    { BLINK_PORT_NUMBER, BLINK_PIN_NUMBER, BLINK_ACTIVE_LOW },
  };

#endif

// ----- main() ---------------------------------------------------------------

Timer timer;

void heartbeat(void)
{
	blinkLeds[0].toggle();
}

void heartbeat_mainApp(void)
{
	blinkLeds[1].toggle();
}



void atLimitSwitch(int i)
{
	if (i == 0)
	{
		blinkLeds[2].turnOn();
	}
	else
	{
		blinkLeds[3].turnOn();
	}
}


void LEDs_init(void)
{
	// Perform all necessary initialisations for the LEDs.
	for (size_t i = 0; i < (sizeof(blinkLeds) / sizeof(blinkLeds[0])); ++i)
	{
		blinkLeds[i].powerUp ();
	}
}



/*
 * List of tasks:
 *
 * 	1. reader - reads line of data from serial (or from file) - add data to parser
 * 	2. parser - parses data - create commands from them (move, line, etc)
 *  3. movement_control - reads commands and compute movements
 *  4. motor_control - controls steppers according to commands (get them to required position)
 *  5. (optional) GUI (plot actual state)
 *
 */

void initialize_hardware(void)
{
  // Initialise the HAL Library; it must be the first function
  // to be executed before the call of any HAL function.
  HAL_Init();

  // Enable HSE Oscillator and activate PLL with HSE as source
  //SystemClock_Config();

  // Call the CSMSIS system clock routine to store the clock frequency
  // in the SystemCoreClock global RAM location.
  SystemCoreClockUpdate();
}

void LED_loop(void)
{
    timer.start ();
    LEDs_init();

	// Button init
	Gpio userButton({eGPIO_PORT_A, 0, false, eGPIO_Mode_Input, eGPIO_Pull_No });
	userButton.powerUp();

	while(1)
	{
		if (userButton.isOn())
		{
			static int idx = 0;
			blinkLeds[idx].turnOff();
			idx = (idx + 1) % 4;
			blinkLeds[idx].turnOn();

			Timer::sleep(100);
		}
	}

}

void systemInit()
{
	initialize_hardware();

	LOG("System clock: %u Hz\n" <<  SystemCoreClock);

    timer.start ();

    // Heartbeat LED initialization
    LEDs_init();
}


void appInit()
{
	// Order of initialization is important!
	timeDelayInit();
	//stepperControl_init();
	//movementControl_init();
	//parser_init();
	//reader_init();
}

bool lineComplete = false;
char rxBuffer[512] = {0};
int rxIndex = 0;

Serial serial;

void readInput()
{
	char rx = serial.read();
	if (rx == ';')
	{
		lineComplete = true;
	}
	if (rxIndex < sizeof(rxBuffer))
	{
		rxBuffer[rxIndex++] = rx;
	}
}

const int cMaxNumberOfCmdParts = 10;

typedef struct
{
	char * parts[cMaxNumberOfCmdParts];
	int32_t length;
	bool used;
} command;


bool processInputData(char * pData, int dataLen, command * pCmdData)
{
	//char dataBuffer[512];
	command cmdData;
	//memset(dataBuffer, 0, sizeof(dataBuffer));
	memset(cmdData.parts, 0, sizeof(cmdData.parts));

	// Create a local copy of input data
	//size_t sizeToCopy = MIN(dataLen, sizeof(dataBuffer));
	//memcpy(dataBuffer, pData, sizeToCopy);

	const char delimiters[] = ",";
	char *token;

	/* get the first token */
	token = strtok(pData, delimiters);

	int i = 0;
	if (token != NULL)
	{
		cmdData.parts[i++] = token;
	}

	/* walk through other tokens */
	while( token != NULL )
	{
		token = strtok(NULL, delimiters);
		if (token != NULL)
		{
			cmdData.parts[i++] = token;
		}
	}

	cmdData.length = i;
	cmdData.used = (i) ? true : false;


	if (cmdData.length == 2)
	{
		*pCmdData = cmdData;
		return true;
	}
	else
	{
		LOG("Invalid input data: ");
		LOG(pData);
		return false;
	}
}



int main(int argc, char** argv)
{
	/* System init */
	systemInit();

	// User button init
	Gpio userButton({eGPIO_PORT_A, 0, false, eGPIO_Mode_Input, eGPIO_Pull_No });
	userButton.powerUp();

	// Wait for user button press
	while(false == userButton.isOn())
	{
		Timer::sleep(100);
		heartbeat_mainApp();
	}

	appInit();

	Stepper stepper1;
	Stepper stepper2;

	stepper1.registerGPIOs(getStepperGPIOs(0));
	stepper2.registerGPIOs(getStepperGPIOs(1));

	while(1)
	{
		if (lineComplete)
		{
			command cmdData;

			if (processInputData(rxBuffer, rxIndex, &cmdData))
			{
				stepper1.forceMove(atoi(cmdData.parts[0]));
				stepper2.forceMove(atoi(cmdData.parts[1]));
			}

			// Clear input data
			memset(rxBuffer, 0, sizeof(rxBuffer));
			lineComplete = false;
			rxIndex = 0;
		}

		readInput();
	}

	while(1)
	{
		LOG("PROGRAM END");
	}
	return 0;
}


// ----------------------------------------------------------------------------
