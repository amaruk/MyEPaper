#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#define SYSFS_ADC_DIR_CHMAX	7
#define SYSFS_ADC_DIR_CH0 "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"
#define SYSFS_ADC_DIR_CH1 "/sys/bus/iio/devices/iio:device0/in_voltage1_raw"
#define SYSFS_ADC_DIR_CH2 "/sys/bus/iio/devices/iio:device0/in_voltage2_raw"
#define SYSFS_ADC_DIR_CH3 "/sys/bus/iio/devices/iio:device0/in_voltage3_raw"
#define SYSFS_ADC_DIR_CH4 "/sys/bus/iio/devices/iio:device0/in_voltage4_raw"
#define SYSFS_ADC_DIR_CH5 "/sys/bus/iio/devices/iio:device0/in_voltage5_raw"
#define SYSFS_ADC_DIR_CH6 "/sys/bus/iio/devices/iio:device0/in_voltage6_raw"
#define SYSFS_ADC_DEV "/sys/devices/bone_capemgr.9/slots"
#define MAX_BUF 64

#define STATUS_RELEASE	0
#define STATUS_PRESS	1

#define THREASHOLD_GND		2700	/* ADC value when connected to GND */
#define THREASHOLD_VALID	3700	/* Valid ADC value when connected to VDD */
#define FLT_CNT_PRESS		2		/* Press filter counter */
#define FLT_CNT_RELEASE		2		/* Release filter counter */
#define FLT_CNT_INIT		0		/* Initial value of counters */

/* ADC device file names */
static char *_adcChStrP[SYSFS_ADC_DIR_CHMAX] = { NULL };
/* ADC channel status: PRESS/RELEASE */
char adcStatus[SYSFS_ADC_DIR_CHMAX] =
{ STATUS_RELEASE, STATUS_RELEASE, STATUS_RELEASE,
	STATUS_RELEASE, STATUS_RELEASE, STATUS_RELEASE };
/* ADC channel press filter counter */
char adcPressCnt[SYSFS_ADC_DIR_CHMAX] =
{ FLT_CNT_INIT, FLT_CNT_INIT, FLT_CNT_INIT,
	FLT_CNT_INIT, FLT_CNT_INIT, FLT_CNT_INIT };
/* ADC channel release filter counter */
char adcReleaseCnt[SYSFS_ADC_DIR_CHMAX] =
{ FLT_CNT_INIT, FLT_CNT_INIT, FLT_CNT_INIT,
	FLT_CNT_INIT, FLT_CNT_INIT, FLT_CNT_INIT };

/* Get ADC channel value 0~4095 */
int adcGetChVal(int ch)
{
	char adcChStr[10]; 		/* String ADC value */
	int adcChStrLen = 0; 	/* String ADC value length */
	int adcChInt = 0; 		/* Integer ADC value */
	FILE *fd = NULL; 		/* Point to ADC device` file */
	int idx = 0;

	fd = fopen(_adcChStrP[ch], "r+");
	fscanf(fd, "%s", adcChStr);

	/* Record ADC value string length */
	adcChStrLen = strlen(adcChStr);

	/* Translate character into number */
	for (idx = 0; idx < adcChStrLen; idx++)
	{
		/* Invalid character */
		if (('\0' != adcChStr[idx]) &&
			(('0' > adcChStr[idx]) ||
			('9' < adcChStr[idx])))
		{
			adcChStrLen = 0;
			break;
		}
		/* Character to number */
		else
		{ adcChStr[idx] -= '0'; }
	}

	/* Make up the int value */
	switch (adcChStrLen)
	{
		case 4:
			adcChInt = adcChStr[0] * 1000 + adcChStr[1] * 100
					+ adcChStr[2] * 10 + adcChStr[3];
			break;
		case 3:
			adcChInt = adcChStr[0] * 100 + adcChStr[1] * 10
					+ adcChStr[2];
			break;
		case 2:
			adcChInt = adcChStr[0] * 10 + adcChStr[1];
			break;
		case 1:
			adcChInt = adcChStr[0];
			break;
		default:
			adcChInt = THREASHOLD_VALID + 1; /* Invalid */
			break;
	}

	fclose(fd);

	return adcChInt;
}

/* ADC initialization */
void adcInit(void)
{
	FILE *fd = NULL;

	_adcChStrP[0] = SYSFS_ADC_DIR_CH0;
	_adcChStrP[1] = SYSFS_ADC_DIR_CH1;
	_adcChStrP[2] = SYSFS_ADC_DIR_CH2;
	_adcChStrP[3] = SYSFS_ADC_DIR_CH3;
	_adcChStrP[4] = SYSFS_ADC_DIR_CH4;
	_adcChStrP[5] = SYSFS_ADC_DIR_CH5;
	_adcChStrP[6] = SYSFS_ADC_DIR_CH6;

	fd = fopen(SYSFS_ADC_DIR_CH0, "w");
	if (fd == NULL) /* Load ADC if it is not loaded*/
	{
		fd = fopen(SYSFS_ADC_DEV, "w");
		fwrite("BB-ADC", sizeof(int), 6, fd); /* "BB-ADC" has 6 characters */
		fclose(fd);
	}
}

void adcTest(void)
{
	int adcVal = 0; /* ADC channel value */
	int adcChIdx = 0;

	adcInit();

	for (;;)
	{
		for (adcChIdx = 0; adcChIdx < SYSFS_ADC_DIR_CHMAX; adcChIdx++)
		{
			adcVal = adcGetChVal(adcChIdx);

			/* Release status + Press detected */
			if ((adcVal < THREASHOLD_GND) && (adcStatus[adcChIdx] == STATUS_RELEASE))
			{
				char *musicP = NULL;
				/* Press filter */
				if (FLT_CNT_PRESS > adcPressCnt[adcChIdx])
				{
					adcPressCnt[adcChIdx]++;
				}
				if (FLT_CNT_PRESS == adcPressCnt[adcChIdx])
				{
					adcStatus[adcChIdx] = STATUS_PRESS;
					adcReleaseCnt[adcChIdx] = FLT_CNT_INIT;
					//printf("[%d] ON[%d]\n", adcChIdx, adcVal);
					/* Play the wav file */
					switch (adcChIdx)
					{
					case 0:
						musicP = "./wavPlayer ./Piano/C4.wav &";
						break;
					case 1:
						musicP = "./wavPlayer ./Piano/D4.wav &";
						break;
					case 2:
						musicP = "./wavPlayer ./Piano/E4.wav &";
						break;
					case 3:
						musicP = "./wavPlayer ./Piano/F4.wav &";
						break;
					case 4:
						musicP = "./wavPlayer ./Piano/G4.wav &";
						break;
					case 5:
						musicP = "./wavPlayer ./Piano/A4.wav &";
						break;
					case 6:
						musicP = "./wavPlayer ./Piano/B4.wav &";
						break;
					default:
						printf("Fatal Error!\n");
						break;
					}
					//printf("playing: %s\n", musicP);
					system(musicP);
				}
			}
			/* Press status + Press detected */
			else if ((adcVal < THREASHOLD_GND) && (adcStatus[adcChIdx] == STATUS_PRESS))
			{
				adcReleaseCnt[adcChIdx] = FLT_CNT_INIT;
			}
			/* Press status + Release detected */
			else if ((adcVal >= THREASHOLD_GND) && (adcVal <= THREASHOLD_VALID)
					&& (adcStatus[adcChIdx] == STATUS_PRESS))
			{
				/* Release filter */
				if (FLT_CNT_RELEASE > adcReleaseCnt[adcChIdx])
				{
					adcReleaseCnt[adcChIdx]++;
				}
				if (FLT_CNT_RELEASE == adcReleaseCnt[adcChIdx])
				{
					adcStatus[adcChIdx] = STATUS_RELEASE;
					adcPressCnt[adcChIdx] = FLT_CNT_INIT;
					//printf("[%d] OFF[%d]\n", adcChIdx, adcVal);
				}
			}
			/* Release status + Release detected */
			else if ((adcVal >= THREASHOLD_GND) && (adcVal <= THREASHOLD_VALID)
					&& (adcStatus[adcChIdx] == STATUS_RELEASE))
			{
				adcPressCnt[adcChIdx] = FLT_CNT_INIT;
			}

			usleep(1000);
		}
	}
}

