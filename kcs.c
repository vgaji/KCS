#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "binary.h"

#define SECOND_22050HZ 1225
#define CYCLES_LENGTH 18

void KCSencode(FILE *fileIn, FILE *fileOut, int baudMode, int stopBits, int leader, bool sineWave, bool wavHeader);
void KCSdecode(FILE *fileIn, FILE *fileOut, int baudMode, int stopBits);
void KCSwavHeaderWrite(FILE *file, unsigned int sampleCount);


void KCSencode(FILE *fileIn, FILE *fileOut, int baudMode, int stopBits, int leader, bool sineWave, bool wavHeader)
{
	/*
	baudMode
	0 - 300
	1 - 600
	2 - 1200

	sineWave
	FALSE - square wave
	TRUE - sine wave

	wavHeader
    FALSE - do not generate wav header
	TRUE - generate wav header
	*/
	size_t fileSize;

	unsigned char BIT0[CYCLES_LENGTH] = {0x96, 0xC0, 0xE2, 0xF8, 0xFF, 0xF8, 0xE2, 0xC0, 0x96, 0x6A, 0x40, 0x1E, 0x08, 0x00, 0x08, 0x1E, 0x40, 0x6A};/*1 cycle of a 1200 sine wave (0)*/
	unsigned char BIT1[CYCLES_LENGTH] = {0xD2, 0xFE, 0xEF, 0xAC, 0x54, 0x11, 0x02, 0x2E, 0x80, 0xD2, 0xFE, 0xEF, 0xAC, 0x54, 0x11, 0x02, 0x2E, 0x80};/*2 cycles of a 2400 sine wave (1)*/


	if(!sineWave) for(int i = 0; i < 18; ++i) //If sineWave is false, we convert waves to square by clipping each at 128.
		{
			if(BIT0[i] > 128) BIT0[i] = 255;
			else BIT0[i] = 0;

			if(BIT1[i] > 128) BIT1[i] = 255;
			else BIT1[i] = 0;
		}

    if(wavHeader) fseek(fileOut, 44, SEEK_SET);  //Skipping 44 bytes to write a wav header to the file later.


    /*
    Here we convert desired baud rate to number of wave cycles;
	0 = 4 - 300 baud / 4 cycles of 1200HZ and 8 cycles of 2400HZ tone.
	1 = 2 - 600 baud / 2 cycles of 1200HZ and 4 cycles of 2400HZ tone.
	2 = 1 - 1200 baud / 1 cycles of 1200HZ and 2 cycles of 2400HZ tone.
	*/
	switch(baudMode)
	{
		case 0:
			baudMode = 4;
			break;

		case 1:
			baudMode = 2;
			break;

		case 2:
			baudMode = 1;
			break;
	}


	fseek(fileIn, 0L, SEEK_END);
	fileSize = ftell(fileIn);
	fseek(fileIn, 0L, SEEK_SET);


	//This writes a lead in 2400hz tone of a specified duration.
	for(size_t i = 0; i < SECOND_22050HZ * leader; ++i)
	{
		fwrite(BIT1, CYCLES_LENGTH, sizeof(unsigned char), fileOut);
	}

	for(size_t i = 0; i < fileSize; ++i) // This is the main loop that writes the data of a file to a file stream.
	{
		unsigned char byteDec;
		bool *byteStr;
		fread(&byteDec, sizeof(unsigned char), 1, fileIn); // Reading a byte from a file.
		byteStr = (bool*)decToBin(byteDec); // Converting the byte we jut reat to a binary string.

		for(int j = 0; j < baudMode; ++j)
		{
			fwrite(BIT0, 18, 1, fileOut);
		}

		for(int j = 7; j >= 0; --j) // j is 7 as we go from the most significant bit I guess.
		{
			for(int k = 0; k < baudMode; ++k)
			{
				if(*(byteStr + j) == 1) fwrite(BIT1, 18, 1, fileOut);
				else fwrite(BIT0, 18, sizeof(unsigned char), fileOut);
			}
		}

		for(int j = 0; j < baudMode * stopBits; ++j)
		{
			fwrite(BIT1, 18, 1, fileOut);
		}

		free(byteStr);
	}
	//This writes a lead out 2400hz tone of a specified duration.
	for(int i = 0; i < SECOND_22050HZ * leader; ++i)
	{
		fwrite(BIT1, CYCLES_LENGTH, sizeof(unsigned char), fileOut);
	}

    if(wavHeader)
    {
        fileSize = ftell(fileOut);
        fseek(fileOut, 0L, SEEK_SET);               // If wavHeader == true, getting the filesize, seeking the file to the beginning and writing the wav header in the first 44 bytes.
        KCSwavHeaderWrite(fileOut, fileSize - 44);
    }

	fflush(fileOut);
}

void KCSdecode(FILE *fileIn, FILE *fileOut, int baudMode, int stopBits)
{
	size_t filesize, i;

	unsigned char thisSample = 0;
	unsigned char lastSample = 1;
	unsigned char numberOf1200hzHalfPeriods = 0;
	unsigned char periods1200 = 4;
	unsigned char numberOf2400hzHalfPeriods = 0;
	unsigned char periods2400 = 8;
	unsigned char numberOfSamplesBeforeSwitch = 0;
	unsigned char binaryPlaceCount = 0;
	bool startBit = false;
	unsigned char binary[11] = {0};


	for(int i = 0; i < 4; ++i)
    {
        unsigned char temp;

        fread(&temp, sizeof(unsigned char), 1, fileIn);             // If we detect the wav header magic number at the beginning we skip 44 bytes (the wav header), and we get to the data.

        if("RIFF"[i] == temp && i == 3) fseek(fileIn, SEEK_SET, 44);
    }


	fseek(fileIn, 0L, SEEK_END);
	filesize = ftell(fileIn);       // Getting the file size.
	fseek(fileIn, 0L, SEEK_SET);


    /*
    Here we convert desired baud rate to number of wave cycles;
	0 = 4 - 300 baud / 4 cycles of 1200HZ and 8 cycles of 2400HZ tone.
	1 = 2 - 600 baud / 2 cycles of 1200HZ and 4 cycles of 2400HZ tone.
	2 = 1 - 1200 baud / 1 cycles of 1200HZ and 2 cycles of 2400HZ tone.
	*/
	switch(baudMode)
	{
		case 0:
			periods1200 = 8;
			periods2400 = 16;
			break;

		case 1:
			periods1200 = 4;
			periods2400 = 8;
			break;

		case 2:
			periods1200 = 2;
			periods2400 = 4;
			break;
	}




	for(i = 0; i < filesize; ++i)
	{
		/*get next sample*/
    	fread(&thisSample, 1, 1, fileIn);

		/*check if sign has switched*/
    	if(thisSample == 128 || (thisSample < 128 && lastSample > 128) || (thisSample > 128 && lastSample < 128))
		{
			/*
			* number of samples between 1 and 7, is one		(should be 3 and 7, but this works, so I won't touch it)
			* number of samples between 7 and 17, is zero	(should be 7 and 17, but this works, so I won't touch it)
			* a perfect 1 is 4.2 samples,
			* a perfect 0 is 8.5 samples
			*/
    		if(numberOfSamplesBeforeSwitch > 1 && numberOfSamplesBeforeSwitch < 7)
			{
				numberOf2400hzHalfPeriods++;
				/*reset sample count*/
				numberOfSamplesBeforeSwitch = 0;
			}
			else if(numberOfSamplesBeforeSwitch >= 7 && numberOfSamplesBeforeSwitch <= 17)
			{
				numberOf1200hzHalfPeriods++;
				/*reset sample count*/
				numberOfSamplesBeforeSwitch = 0;
    		}
    	}



		if(numberOf2400hzHalfPeriods == periods2400 && startBit)
		{
    		/*add 1 bit*/

			/*reset metrics*/
    		numberOf1200hzHalfPeriods = 0;
    		numberOf2400hzHalfPeriods = 0;
			/*add 1 to current number place*/
    		binary[binaryPlaceCount] = 1;
			/*iterate to next number place*/
    		binaryPlaceCount++;
		}

		if(numberOf1200hzHalfPeriods == periods1200)
		{
			/*add 0 bit*/

			/*toggle start*/
			startBit = true;
			/*reset metrics*/
    		numberOf1200hzHalfPeriods = 0;
    		numberOf2400hzHalfPeriods = 0;
    		/*add 0 to current number place*/
    		binary[binaryPlaceCount] = 0;
			/*iterate to next number place*/
    		binaryPlaceCount++;
    	}


    	if(binaryPlaceCount == 8 + stopBits + 1 && startBit)
		{
			/*write binary to file stream*/
			bool temp[8] = {0};
			unsigned char byteDec;
			int k = 0;
    		for(int j = 8; j >= 1; --j)
    		{
    			temp[k] = binary[j];
    			++k;
			}
			byteDec = binToDec(temp);
    		fwrite(&byteDec, 1, 1, fileOut);
			/*reset number place*/
    		binaryPlaceCount = 0;
    	}

		numberOfSamplesBeforeSwitch++;

		lastSample = thisSample;

		if(binary[0] == 1)break;
	}

    /*write output*/
	fflush(fileOut);
}

void KCSwavHeaderWrite(FILE *file, unsigned int sampleCount)
{
    //RIFF header
    fwrite("RIFF", 4, 1, file);
    unsigned int filesize = sampleCount + 12 + 16 + 8 - 8;
    fwrite(&filesize, 4, 1, file);
    fwrite("WAVE", 4, 1, file);

    //format chunk
    fwrite("fmt ", 4, 1, file);
    unsigned int fmtlength = 16;
    fwrite(&fmtlength, 4, 1, file);
    unsigned short int format = 1; //PCM
    fwrite(&format, 2, 1, file);
    unsigned short int channels = 1;
    fwrite(&channels, 2, 1, file);
    unsigned int sampleRate = 22050;
    fwrite(&sampleRate, 4, 1, file);
    fwrite(&sampleRate, 4, 1, file); // bytes/second
    unsigned short int blockAlign = 1;
    fwrite(&blockAlign, 2, 1, file);
    unsigned short int bitsPerSample = 8;
    fwrite(&bitsPerSample, 2, 1, file);

    //data chunk
    fwrite("data", 4, 1, file);
    fwrite(&sampleCount, 4, 1, file);

    fflush(file);
}
