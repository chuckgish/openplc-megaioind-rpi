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
#include <math.h>
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
// Global vars
//-----------------------------------------------------------------------------
char id_char[2];
int id_int = 0;

char output_name[20];
char input_name[20];

char channel_char[2];
int channel_int = 0;

char value_char[20];
int value_int = 0;
float value_float = 0.0;


//-----------------------------------------------------------------------------
// Get a digital input
//-----------------------------------------------------------------------------
int getDigitalInput(int id, char *input, int channel);


//-----------------------------------------------------------------------------
// Set a digital output
//-----------------------------------------------------------------------------
void setDigitalOutput(int id, char *output, int channel, int value);


//-----------------------------------------------------------------------------
// Get an analog input
//-----------------------------------------------------------------------------
float getAnalogInput(int id, char *input, int channel);


//-----------------------------------------------------------------------------
// Set an analog output
//-----------------------------------------------------------------------------
void setAnalogOutput(int id, char *output, int channel, float value);


//-----------------------------------------------------------------------------
// Scale word value (0-65535) from PLC to some range
//-----------------------------------------------------------------------------
float scaleFromWord(float source_value, float target_min, float target_max);

//-----------------------------------------------------------------------------
// Scale from analog value to word value (0-65535)
//-----------------------------------------------------------------------------
int scaleToWord(float measure_value, float measure_min, float measure_max);


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


  //DIGITAL INPUT
  strcpy(input_name, "ropto");

	for (int i = 0; i < MAX_INPUT; i++)
	{
	    if (pinNotPresent(ignored_bool_inputs, ARRAY_SIZE(ignored_bool_inputs), i))
    		if (bool_input[i/8][i%8] != NULL)
        {
          channel_int = i+1;
          *bool_input[i/8][i%8] = getDigitalInput(id_int, input_name, channel_int);;
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
// This function is called by the OpenPLC in a loop. Here the internal buffers
// must be updated to reflect the actual Output state. The mutex bufferLock
// must be used to protect access to the buffers on a threaded environment.
//-----------------------------------------------------------------------------
void updateBuffersOut()
{
	pthread_mutex_lock(&bufferLock); //lock mutex


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
      		setDigitalOutput(id_int, output_name, channel_int, value_int);
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
          setDigitalOutput(id_int, output_name, channel_int, value_int);
        }

        wait(NULL);
      }
    }
  }


  // ANALOG OUT
  for (int i = 0; i < MAX_ANALOG_OUT; i++)
  {
    if (i < 4) //0-10 Volt outputs - 0 to 3
    {
      if (pinNotPresent(ignored_int_outputs, ARRAY_SIZE(ignored_int_outputs), i))
    		if (int_output[i] != NULL)
      {
        strcpy(output_name, "wuout");
        channel_int = i+1;
        value_float = scaleFromWord(*int_output[i], 0, 10);

        if (fork() == 0)
      	{
      		setAnalogOutput(id_int, output_name, channel_int, value_float);
      	}

      	wait(NULL);
      }

    }
    else // 4-20 mA outputs - 4-7
    {
      if (pinNotPresent(ignored_int_outputs, ARRAY_SIZE(ignored_int_outputs), i))
    		if (int_output[i] != NULL)
      {
        strcpy(output_name, "wiout");
        channel_int = i-3;
        value_float = scaleFromWord(*int_output[i], 4, 20);

        if (fork() == 0)
      	{
      		setAnalogOutput(id_int, output_name, channel_int, value_float);
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
// getDigitalInput
//-----------------------------------------------------------------------------
int getDigitalInput(int id, char *input, int channel)
{

  int ret_int = 0;

  snprintf(id_char, sizeof(id_char), "%d", id);

  snprintf(channel_char, sizeof(channel_char), "%d", channel);

  //----------------------------------------------------------------------------
  // This is based on code found at:
  // https://stackoverflow.com/questions/7292642/grabbing-output-from-exec
  //----------------------------------------------------------------------------
  #define die(e) do { fprintf(stderr, "%s\n", e); exit(EXIT_FAILURE); } while (0);

  int link[2];
  pid_t pid;
  char execl_response[10];

  if (pipe(link)==-1)
    die("pipe");

  if ((pid = fork()) == -1)
    die("fork");

  if(pid == 0) {

    dup2 (link[1], STDOUT_FILENO);
    close(link[0]);
    close(link[1]);
    execl(MEGAIO_PATH, MEGAIO_COMMAND, id_char, input, channel_char, NULL);
    die("execl");

  } else {

    close(link[1]);
    int nbytes = read(link[0], execl_response, sizeof(execl_response));

    //ret_int = execl_response[0] - '0';

    char ret_char[2] = {execl_response[0], '\0'};
    ret_int = atoi(ret_char);

    wait(NULL);

  }
  //------------------------------------------------------------------------------

return ret_int;

}


//-----------------------------------------------------------------------------
// setDigitalOutput
//-----------------------------------------------------------------------------
void setDigitalOutput(int id, char *output, int channel, int value)
{

  snprintf(id_char, sizeof(id_char), "%d", id);

  snprintf(channel_char, sizeof(channel_char), "%d", channel);
  snprintf(value_char, sizeof(value_char), "%d", value);

  execl(MEGAIO_PATH, MEGAIO_COMMAND, id_char, output, channel_char, value_char, NULL);
}


//-----------------------------------------------------------------------------
// getAnalogInput
//-----------------------------------------------------------------------------
float getAnalogInput(int id, char *input, int channel)
{
  float ret_float = 0.0;

  snprintf(id_char, sizeof(id_char), "%d", id);

  snprintf(channel_char, sizeof(channel_char), "%d", channel);

  //----------------------------------------------------------------------------
  // This is based on code found at:
  // https://stackoverflow.com/questions/7292642/grabbing-output-from-exec
  //----------------------------------------------------------------------------
  #define die(e) do { fprintf(stderr, "%s\n", e); exit(EXIT_FAILURE); } while (0);

  int link[2];
  pid_t pid;
  char execl_response[10];

  if (pipe(link)==-1)
    die("pipe");

  if ((pid = fork()) == -1)
    die("fork");

  if(pid == 0) {

    dup2 (link[1], STDOUT_FILENO);
    close(link[0]);
    close(link[1]);
    execl(MEGAIO_PATH, MEGAIO_COMMAND, id_char, input, channel_char, NULL);
    die("execl");

  } else {

    close(link[1]);
    int nbytes = read(link[0], execl_response, sizeof(execl_response));

    char ret_char[6] = {
                        execl_response[0],
                        execl_response[1],
                        execl_response[2],
                        execl_response[3],
                        execl_response[4],
                        '\0'
                        };

    ret_float = atof(ret_char);

    wait(NULL);

  }
  //------------------------------------------------------------------------------

return ret_float;

}


//-----------------------------------------------------------------------------
// setAnalogOutput
//-----------------------------------------------------------------------------
void setAnalogOutput(int id, char *output, int channel, float value)
{

  snprintf(id_char, sizeof(id_char), "%d", id);

  snprintf(channel_char, sizeof(channel_char), "%d", channel);
  snprintf(value_char, sizeof(value_char), "%2f", value);

  execl(MEGAIO_PATH, MEGAIO_COMMAND, id_char, output, channel_char, value_char, NULL);
}


//-----------------------------------------------------------------------------
// scaleFromWord
//-----------------------------------------------------------------------------
float scaleFromWord(float word_value, float target_min, float target_max)
{
  float target_value = (word_value/65535)*(target_max - target_min) + target_min;

  return target_value;
}


//-----------------------------------------------------------------------------
// scaleToWord
//-----------------------------------------------------------------------------
int scaleToWord(float measure_value, float measure_min, float measure_max)
{
  float target_value = ((measure_value - measure_min)/(measure_max - measure_min))*65535;

  return target_value;
}
