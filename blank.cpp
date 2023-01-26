//-----------------------------------------------------------------------------
// Copyright 2015 Thiago Alves
//
// Based on the LDmicro software by Jonathan Westhues
// This file is part of the OpenPLC Software Stack.
//
// OpenPLC is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// OpenPLC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with OpenPLC.  If not, see <http://www.gnu.org/licenses/>.
//------
//
// This file is the hardware layer for the OpenPLC. If you change the platform
// where it is running, you may only need to change this file. All the I/O
// related stuff is here. Basically it provides functions to read and write
// to the OpenPLC internal buffers in order to update I/O state.
// Thiago Alves, Dec 2015
//-----------------------------------------------------------------------------

#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "ladder.h"
#include "custom_layer.h"

#if !defined(ARRAY_SIZE)
    #define ARRAY_SIZE(x) (sizeof((x)) / sizeof((x)[0]))
#endif

//-----------------------------------------------------------------------------
// The digital inputs are the opto-isolated inputs. Four of the digital outputs
// are opto-isolated outputs, and the other four are relays. Both the analog
// inputs and outputs have four each 0-10 V, and four each 4-20 mA.
//-----------------------------------------------------------------------------
#define MAX_INPUT 		4
#define MAX_OUTPUT 		8
#define MAX_ANALOG_IN	8
#define MAX_ANALOG_OUT	8


//-----------------------------------------------------------------------------
// The installed program - path and command
//-----------------------------------------------------------------------------
#define MEGAIO_PATH "/usr/local/bin/megaioind"
#define MEGAIO_COMMAND "megaioind"


//-----------------------------------------------------------------------------
// Digital I/O
//-----------------------------------------------------------------------------
char value_char[4];
int value_int = 0;


//-----------------------------------------------------------------------------
// Set outputs
//-----------------------------------------------------------------------------
//void setOutput(int id, char &output, int output_channel, int output_value);



//-----------------------------------------------------------------------------
// This function is called by the main OpenPLC routine when it is initializing.
// Hardware initialization procedures should be here.
//-----------------------------------------------------------------------------
void initializeHardware()
{
}

//-----------------------------------------------------------------------------
// This function is called by the main OpenPLC routine when it is finalizing.
// Resource clearing procedures should be here.
//-----------------------------------------------------------------------------
void finalizeHardware()
{
}

//-----------------------------------------------------------------------------
// This function is called by the OpenPLC in a loop. Here the internal buffers
// must be updated to reflect the actual Input state. The mutex bufferLock
// must be used to protect access to the buffers on a threaded environment.
//-----------------------------------------------------------------------------
void updateBuffersIn()
{
	pthread_mutex_lock(&bufferLock); //lock mutex

	/*********READING AND WRITING TO I/O**************

	*bool_input[0][0] = read_digital_input(0);
	write_digital_output(0, *bool_output[0][0]);

	*int_input[0] = read_analog_input(0);
	write_analog_output(0, *int_output[0]);

	**************************************************/

	pthread_mutex_unlock(&bufferLock); //unlock mutex
}

//-----------------------------------------------------------------------------
// This function is called by the OpenPLC in a loop. Here the internal buffers
// must be updated to reflect the actual Output state. The mutex bufferLock
// must be used to protect access to the buffers on a threaded environment.
//-----------------------------------------------------------------------------
void updateBuffersOut()
{
	pthread_mutex_lock(&bufferLock); //lock mutex

	// if(*bool_output[0][0] == 0)
	// {
	// 	strcpy(state, "off");
	// }
  //
	// if(*bool_output[0][0] == 1)
	// {
	// 	strcpy(state, "on");
  //
	// }

  value_int = *bool_output[0][0];
  snprintf(value_char, sizeof(value_char), "%d", value_int);

	if (fork() == 0)
	{
		execl(MEGAIO_PATH, MEGAIO_COMMAND, "0", "wrelay", "1", value_char, NULL);
	}

	wait(NULL);


	/*********READING AND WRITING TO I/O**************

	*bool_input[0][0] = read_digital_input(0);
	write_digital_output(0, *bool_output[0][0]);

	*int_input[0] = read_analog_input(0);
	write_analog_output(0, *int_output[0]);

	**************************************************/

	pthread_mutex_unlock(&bufferLock); //unlock mutex
}