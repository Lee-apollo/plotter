/*
 * embedded_cout.cpp
 *
 *  Created on: 15 Mar 2018
 *      Author: pi
 */
#include <cstdio>
#include <string>
#include <string.h>
#include <stdlib.h>

#include "embedded_cout.h"

#include "Serial.h"

EmbeddedCout dbgCout;

Serial serial1;

void EmbeddedCout::_putChar(char c)
{

	serial1.write(c);
	//printf("%c", c);
}

size_t EmbeddedCout::_getFreeSpaceSize()
{
	return cEmbeddedCoutBufferSize - charIndex;
}

void EmbeddedCout::flush()
{
	for (int i = 0; i < charIndex; i++)
	{
		_putChar(buffer[i]);
	}
	charIndex = 0;
}

void EmbeddedCout::_add(const char * string, const size_t stringLen)
{
	size_t charCount = stringLen;
	while(charCount > 0)
	{
		size_t dataSize = (charCount < _getFreeSpaceSize()) ? charCount : _getFreeSpaceSize();
		memcpy(&buffer[charIndex], &string[stringLen - charCount], dataSize);
		charIndex += dataSize;
		charCount -= dataSize;

		if (_getFreeSpaceSize() == 0)
		{
			flush();
			continue;
		}
	}
}


EmbeddedCout::EmbeddedCout()
{
	memset(buffer, 0, sizeof(buffer));
	charIndex = 0;
}


/*
EmbeddedCout& EmbeddedCout::operator<<(std::string data)
{
	_add(data.c_str(), data.length());
	return *this;
}
*/

EmbeddedCout& EmbeddedCout::operator<<(const char * c_string)
{
	_add(c_string, strlen(c_string));
	return *this;
}

EmbeddedCout& EmbeddedCout::operator<<(eStreamControl command)
{
	switch(command)
	{
		case (dbgEndl):
			_add("\n", 1);
			flush();
			break;
		default:
			break;
	}
	return *this;
}

EmbeddedCout& EmbeddedCout::operator<<(const uint8_t data)
{
	this->operator<<((uint32_t)data);
	return *this;
}

EmbeddedCout& EmbeddedCout::operator<<(const uint16_t data)
{
	this->operator<<((uint32_t)data);
	return *this;
}

EmbeddedCout& EmbeddedCout::operator<<(const uint32_t data)
{
	char itoaBuffer[100];
	snprintf(itoaBuffer, sizeof(itoaBuffer), "%u", data);
	_add(itoaBuffer, strlen(itoaBuffer));
	return *this;
}

EmbeddedCout& EmbeddedCout::operator<<(const int8_t data)
{
	this->operator<<((int32_t)data);
	return *this;
}

EmbeddedCout& EmbeddedCout::operator<<(const int16_t data)
{
	this->operator<<((int32_t)data);
	return *this;
}

EmbeddedCout& EmbeddedCout::operator<<(const int32_t data)
{
	char itoaBuffer[100];
	snprintf(itoaBuffer, sizeof(itoaBuffer), "%d", data);
	_add(itoaBuffer, strlen(itoaBuffer));
	return *this;
}


EmbeddedCout& EmbeddedCout::operator<<(const float fData)
{
	//int data = (int)fData;
	char itoaBuffer[100];
	snprintf(itoaBuffer, sizeof(itoaBuffer), "%d*10^(-6)", (int)(fData * 1000000));
	_add(itoaBuffer, strlen(itoaBuffer));
	return *this;
}



