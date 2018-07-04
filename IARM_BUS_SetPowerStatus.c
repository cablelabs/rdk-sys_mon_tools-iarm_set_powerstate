/*
 * If not stated otherwise in this file or this component's Licenses.txt file the
 * following copyright and licenses apply:
 *
 * Copyright 2016 RDK Management
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
*/
#include <stdio.h>
#include <string.h>
#include "libIBus.h"
#include "pwrMgr.h"

const char *pPowerON = "ON";
const char *pPowerOFF = "OFF";
const char *pPowerStandby = "STANDBY";
const char *pPowerLigtSleep = "LIGHTSLEEP";
const char *pPowerDeepSleep = "DEEPSLEEP";
const char *pPowerSavingModeS1 = "S1";
const char *pPowerSavingModeS2 = "S2";

void setDeepSleepTimer(unsigned int seconds);

void usage ()
{
    printf ("\nUsage: 'iARM_SetPowerStatus [ON | STANDBY | LIGHTSLEEP | DEEPSLEEP | OFF ]'\n");
    printf ("\nUsage: Optionally, specify S1 or S2 as the power saving mode with LIGHTSLEEP or DEEPSLEEP.\n");
    printf ("\t\t ON         -> Set to Active Mode\n");
    printf ("\t\t STANDBY    -> Set to Standby Mode\n");
    printf ("\t\t LIGHTSLEEP -> Set to LIGHT Sleep Standby mode\n");
    printf ("\t\t DEEPSLEEP  -> Set to DEEP Sleep Standby mode\n");
    printf ("\t\t DEEPSLEEP <Timeout value in seconds> -> Set to DEEP Sleep Standby mode with timeout specified\n");
    printf ("\t\t OFF        -> Set to OFF\n");
}

void setPowerMode();

static IARM_Bus_PWRMgr_PowerSavingMode_t getPowerSavingsModeFromArgs(const char * input)
{
	IARM_Bus_PWRMgr_PowerSavingMode_t mode;
	if(0 == strncasecmp(input, pPowerSavingModeS1, strlen(pPowerSavingModeS1)))
	{
		mode = POWER_SAVING_MODE_S1;
	}
	else if(0 == strncasecmp(input, pPowerSavingModeS2, strlen(pPowerSavingModeS2)))
	{
		mode = POWER_SAVING_MODE_S2;
	}
	else
	{
		mode = POWER_SAVING_MODE_NONE;
	}
	return mode;
}
/**
 * Test application to check whether the box is in standby or not.
 * This has been developed to resolve, XONE-4598
 */
int main(int argc, char *argv[])
{
	IARM_Bus_PWRMgr_SetPowerState_Param_t param;
	unsigned int timeout;

	IARM_Bus_Init("iARMSetPower_tool");
	IARM_Bus_Connect();

	param.newState = IARM_BUS_PWRMGR_POWERSTATE_ON;
	param.powerSavingMode = POWER_SAVING_MODE_NONE;

	if (argc < 2)
	{
		usage();
	}
	else if (strncasecmp(pPowerON, argv[1], strlen (pPowerON)) == 0)
	{
		printf ("ON Request...\n");
		param.newState = IARM_BUS_PWRMGR_POWERSTATE_ON;
	}
	else if (strncasecmp(pPowerStandby, argv[1], strlen (pPowerStandby)) == 0)
	{
		param.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY;
		printf ("STANDBY Request...\n");
		if(3 <= argc)
		{
			param.powerSavingMode = getPowerSavingsModeFromArgs(argv[2]);
		}
	}
	else if (strncasecmp(pPowerLigtSleep, argv[1], strlen (pPowerLigtSleep)) == 0)
	{
		param.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP;
		if(3 <= argc)
		{
			param.powerSavingMode = getPowerSavingsModeFromArgs(argv[2]);
		}
		printf ("Light Sleep Request...\n");
	}
	else if (strncasecmp(pPowerDeepSleep, argv[1], strlen (pPowerDeepSleep)) == 0)
	{
		param.newState = IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP;
		param.powerSavingMode = POWER_SAVING_MODE_S2; 
		printf ("Deep Sleep Request...\n");
		if (3 == argc)
		{
			if(('S' == argv[2][0]) || ('s' == argv[2][0]))
			{
				param.powerSavingMode = getPowerSavingsModeFromArgs(argv[2]);
			}
			else
			{
				timeout = atoi(argv[2]);
				setDeepSleepTimer(timeout);
			}
		}
		else if(3 < argc)
		{
			timeout = atoi(argv[2]);
			setDeepSleepTimer(timeout);
			param.powerSavingMode = getPowerSavingsModeFromArgs(argv[3]);
		}

	}
	else if (strncasecmp(pPowerOFF, argv[1], strlen (pPowerOFF)) == 0)
	{
		printf ("OFF Request...\t Not processed\n");
	}
	else
	{
		usage();
	}


	if ((param.newState == IARM_BUS_PWRMGR_POWERSTATE_ON) || 
			(param.newState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY) ||
			(param.newState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY_LIGHT_SLEEP) ||
			(param.newState == IARM_BUS_PWRMGR_POWERSTATE_STANDBY_DEEP_SLEEP))
	{
		/** Query current Power state  */
		if (IARM_RESULT_SUCCESS == IARM_Bus_Call(IARM_BUS_PWRMGR_NAME,
					IARM_BUS_PWRMGR_API_SetPowerState,
					(void *)&param,
					sizeof(param)))
		{
			printf ("SetPowerState :: Success \n");
		}
		else
		{
			printf ("SetPowerState :: Failed \n");
		}
	}

	IARM_Bus_Disconnect();
	IARM_Bus_Term();
	return 0;
}

void setDeepSleepTimer(unsigned int seconds)
{
    IARM_Bus_PWRMgr_SetDeepSleepTimeOut_Param_t param;
    param.timeout = seconds;
    IARM_Result_t res = IARM_Bus_Call(IARM_BUS_PWRMGR_NAME, IARM_BUS_PWRMGR_API_SetDeepSleepTimeOut,
                      (void *)&param, sizeof(param));
    if(res == IARM_RESULT_SUCCESS)
    {
        printf ("SetDeepSleepTimeOut :: Success \n");
    }
    else
    {
        printf ("SetDeepSleepTimeOut :: Failed \n");
    }
    return;
}

