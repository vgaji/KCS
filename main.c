#include "kcs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VER0 0
#define VER1 5


void verInfo();
void help();
void processing(unsigned char fileName0[], unsigned char fileName1[]);
void errorFile(unsigned char str[]);
void errorOption();

int main(int argc, char *argv[])
{
	/*Assigning variables with default KCS 300 baud mode settings*/
	unsigned char baudMode = 0;
	unsigned char stopBits = 2;
	unsigned char leader = 5;
	unsigned char BOOLsineWave = 0;
	
	FILE *fileIn, *fileOut;
	
	int i;
	unsigned char BOOLencode = 0;
	
	
	if(argc == 1)
	{
		help();
		return 0;
	}
	
	for(i = 1; i < argc && argv[i][0] == '-'; ++i)
	{
		switch(argv[i][1])
		{
			case 'm':
				BOOLencode = 1;
				break;
				
			case 't':
				BOOLsineWave = 1;
				break;
				
			case 'l':
				leader = atoi(&argv[i][2]);
				break;
				
			case 'b':
				baudMode = atoi(&argv[i][2]);
				break;
				
			case 's':
				stopBits = 1;
				break;
				
			default:
				errorOption();
				exit(0);
				break;
		}
	}
	
	
	if(argv[i] == NULL)
	{
		help();
		return 0;
	}
	
	
	else if(BOOLencode == 1)
	{
		unsigned char temp[256] = {0};
		fileIn = fopen(argv[i], "rb");
		
		if(fileIn == NULL)
		{
			errorFile(argv[i]);
			return 0;
		}
		
		if(argv[i + 1] == NULL)
		{
			strcpy(temp, argv[i]);
			strcat(temp, ".raw");
			
			fileOut = fopen(temp, "wb");
		}
		else
		{
			fileOut = fopen(argv[i + 1], "wb");
		}
		
		if(temp[0] == 0) processing(argv[i], argv[i + 1]);
		else processing(argv[i], temp);
		KCSencode(fileIn, fileOut, baudMode, stopBits, leader, BOOLsineWave);
		fprintf(stderr, "Done!");
	}
	else
	{
		unsigned char temp[256] = {0};
		fileIn = fopen(argv[i], "rb");
		
		if(fileIn == NULL)
		{
			errorFile(argv[i]);
			return 0;
		}
		
		if(argv[i + 1] == NULL)
		{
			strcpy(temp, argv[i]);
			strcat(temp, ".prg");
			
			fileOut = fopen(temp, "wb");
		}
		else
		{
			fileOut = fopen(argv[i + 1], "wb");
		}
		
		if(temp[0] == 0) processing(argv[i], argv[i + 1]);
		else processing(argv[i], temp);
		KCSdecode(fileIn, fileOut, baudMode, stopBits);
		fprintf(stderr, "Done!");
	}
	
	
	fclose(fileIn);
	fclose(fileOut);
	return 0;
}

void verInfo()
{
	fprintf(stderr, "\nKCS version %d.%d\t\tBy Veljko Gajic 18.07.2022.\n\n", VER0, VER1);
}

void help()
{
	verInfo();
	fprintf(stderr, "Usage: KCS [options] [input file] [output file]\n\n-M	make wavefile\n-T	sine wave\n-Ln	leader (sec)\n-Bn	1=600, 2=1200\n-S	1 stop bit\n\nDefault: 300 baud, 1 start bit, 8 data bits,\n	 2 stop bits, square wave\n\nConversion utility for Kansas City Standard\ntapes. Input/output in an unsigned 8-bit\nmono PCM raw wavefile recorded at 22,050\nsamples per second.\n");
}

void processing(unsigned char fileName0[], unsigned char fileName1[])
{
	verInfo();
	fprintf(stderr, "Input file:	%s\nOutput file:	%s\n\nProcessing...\n\n", fileName0, fileName1);
}

void errorFile(unsigned char str[])
{
	verInfo();
	fprintf(stderr, "File %s not found!\n", str);
}

void errorOption()
{
	verInfo();
	fprintf(stderr, "Invalid option!\n");
}
