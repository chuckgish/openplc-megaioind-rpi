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
// are open drain outputs, and the other four are relays. Both the analog
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
// Global vars - general
//-----------------------------------------------------------------------------
char id_char[2];
int id_int = 0;

char output_name[20];

char channel_char[2];
int channel_int = 0;

char value_char[20];
int value_int = 0;


//-----------------------------------------------------------------------------
// Set outputs
//-----------------------------------------------------------------------------
void setOutput(int id, char *output, int channel, int value);



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


  // This is for future versions, in which stacked cards may be used
  id_int = 0;

  // DIGITAL OUT
  for (int i = 0; i < MAX_OUTPUT; i++)
  {
    if (i < 4) //Open-collector outputs - 0 to 3
    {
      if (pinNotPresent(ignored_bool_outputs, ARRAY_SIZE(ignored_bool_outputs), i))
      if (bool_output[i/8][i%8] != NULL)
      {
        strcpy(output_name, "woc");
        channel_int = i+1;
        value_int = *bool_output[i/8][i%8];

        if (fork() == 0)
      	{
      		setOutput(id_int, output_name, channel_int, value_int);
      	}

      	wait(NULL);
      }

    }
    else // Relay outputs - 4-7
    {
      if (pinNotPresent(ignored_bool_outputs, ARRAY_SIZE(ignored_bool_outputs), i))
      if (bool_output[i/8][i%8] != NULL)
      {
        strcpy(output_name, "wrelay");
        channel_int = i-3;
        value_int = *bool_output[i/8][i%8];

        if (fork() == 0)
        {
          setOutput(id_int, output_name, channel_int, value_int);
        }

        wait(NULL);
      }
    }
  }



	/*********READING AND WRITING TO I/O**************

	*bool_input[0][0] = read_digital_input(0);
	write_digital_output(0, *bool_output[0][0]);

	*int_input[0] = read_analog_input(0);
	write_analog_output(0, *int_output[0]);

	**************************************************/

	pthread_mutex_unlock(&bufferLock); //unlock mutex
}


//-----------------------------------------------------------------------------
// Definition of setOutput
//-----------------------------------------------------------------------------
void setOutput(int id, char *output, int channel, int value)
{

  snprintf(id_char, sizeof(id_char), "%d", id);

  snprintf(channel_char, sizeof(channel_char), "%d", channel);
  snprintf(value_char, sizeof(value_char), "%d", value);

  execl(MEGAIO_PATH, MEGAIO_COMMAND, id_char, output, channel_char, value_char, NULL);
}
