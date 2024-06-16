#include <stdio.h>
#include <stdlib.h>
#include "binary.h"


void KCSencode(FILE *fileIn, FILE *fileOut, unsigned char baudMode, unsigned char stopBits, unsigned char leader, unsigned char BOOLsineWave)
{
	/*
	baudMode		0 = 300, 1 = 600, 2 = 1200
	BOOLsineWave	0 = square wave, 1 = sine wave
	*/
	
	size_t i, fileSize;
	int j, k;
	
	unsigned char BIT0[18] = {0x96, 0xC0, 0xE2, 0xF8, 0xFF, 0xF8, 0xE2, 0xC0, 0x96, 0x6A, 0x40, 0x1E, 0x08, 0x00, 0x08, 0x1E, 0x40, 0x6A};/*1 cycle of a 1200 sine wave (0)*/
	unsigned char BIT1[18] = {0xD2, 0xFE, 0xEF, 0xAC, 0x54, 0x11, 0x02, 0x2E, 0x80, 0xD2, 0xFE, 0xEF, 0xAC, 0x54, 0x11, 0x02, 0x2E, 0x80};/*2 cycles of a 2400 sine wave (1)*/
	
	/*If BOOLsineWave is == 0, we convert waves to square*/
	if(BOOLsineWave == 0)
	{
		for(i = 0; i < 18; ++i)
		{
			if(BIT0[i] > 128) BIT0[i] = 255;
			else BIT0[i] = 0;
			
			if(BIT1[i] > 128) BIT1[i] = 255;
			else BIT1[i] = 0;
		}
	}
	
	/*
	Here we convert desired baud rate to number of wave cycles;
	baud_mode
	0 = 4 - 300 baud,
	1 = 2 - 600 baud,
	2 = 1 -1200 baud
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
	
	
	for(i = 0; i < 1225 * leader; ++i)
	{
		fwrite(BIT1, 18, 1, fileOut);
	}

	for(i = 0; i < fileSize; ++i)
	{
		unsigned char byteDec;
		unsigned char *byteStr;
		fread(&byteDec, 1, 1, fileIn);
		byteStr = decToBin(byteDec);
				
		for(j = 0; j < baudMode; ++j)
		{
			fwrite(BIT0, 18, 1, fileOut);
		}
		
		for(j = 7; j >= 0; --j)
		{
			for(k = 0; k < baudMode; ++k)
			{
				if(*(byteStr + j) == 1) fwrite(BIT1, 18, 1, fileOut);
				else fwrite(BIT0, 18, sizeof(unsigned char), fileOut);
			}
		}
		
		for(j = 0; j < baudMode * stopBits; ++j)
		{
			fwrite(BIT1, 18, 1, fileOut);
		}
		
		free(byteStr);
	}
	
	for(i = 0; i < 1225 * leader; ++i)
	{
		fwrite(BIT1, 18, 1, fileOut);
	}
	
	fflush(fileOut);
}

void KCSdecode(FILE *fileIn, FILE *fileOut, unsigned char baudMode, unsigned char stopBits)
{
	size_t filesize, i;
	
	unsigned char thisSample					= 0;
	unsigned char lastSample					= 1;
	unsigned char numberOf1200hzHalfPeriods		= 0;
	unsigned char periods1200					= 4;
	unsigned char numberOf2400hzHalfPeriods		= 0;
	unsigned char periods2400					= 8;
	unsigned char numberOfSamplesBeforeSwitch	= 0;
	unsigned char binaryPlaceCount				= 0;
	unsigned char BOOLstartBit					= 0;
	unsigned char binary[11] = {0};
	
	fseek(fileIn, 0L, SEEK_END);
	filesize = ftell(fileIn);
	fseek(fileIn, 0L, SEEK_SET);
	
	
	/*
	Here we convert desired baud rate to number of wave cycles;
	baudMode
	0 = 4 - 300 baud,
	1 = 2 - 600 baud,
	2 = 1 -1200 baud
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
		
    	
    	
		if(numberOf2400hzHalfPeriods == periods2400 && BOOLstartBit)
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
			BOOLstartBit = 1;
			/*reset metrics*/
    		numberOf1200hzHalfPeriods = 0; 
    		numberOf2400hzHalfPeriods = 0;
    		/*add 0 to current number place*/
    		binary[binaryPlaceCount] = 0;
			/*iterate to next number place*/
    		binaryPlaceCount++;
    	}
	
		
    	if(binaryPlaceCount == 8 + stopBits + 1 && BOOLstartBit)
		{
			/*write binary to file stream*/
			unsigned char temp[8] = {0};
			unsigned char *byteBin;
			unsigned char byteDec;
			int j, k = 0;
    		for(j = 8; j >= 1; --j)
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
