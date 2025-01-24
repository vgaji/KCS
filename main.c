#include "kcs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void printHeaderInfo();
void printUsage(int errorCode);
void printProcessing(char *inputFile, char *outputFile);
void printErrorFile(char *str);
void printErrorOption();

int main(int argc, char *argv[])
{
	/*Assigning variables with default KCS 300 baud mode settings*/
	int baudMode = 0;
	int stopBits = 2;
	int leader = 5;
	bool sineWave = false;
	bool wavHeader = false;
	bool encode = false;

	FILE *fileIn, *fileOut;

	int i = 0;


	if(argc <= 1) printUsage(1);

	for(i = 1; i < argc && argv[i][0] == '-'; ++i)
	{
		switch(argv[i][1])
		{
			case 'm':
				encode = true;
				break;

			case 't':
				sineWave = true;
				break;

			case 'w':
				wavHeader = true;
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
                printUsage(1);
				break;
		}
	}

    if(encode)  //ENCODING
	{
		char temp[256] = {0};
		fileIn = fopen(argv[i], "rb");

		if(fileIn == NULL)
		{
			printErrorFile(argv[i]);
			exit(1);
		}

		if(argv[i + 1] == NULL && !wavHeader)
		{
			strcpy(temp, argv[i]);
			strcat(temp, ".raw");

			fileOut = fopen(temp, "wb");
		}
		else if(argv[i + 1] == NULL && wavHeader)
        {
			strcpy(temp, argv[i]);
			strcat(temp, ".wav");

			fileOut = fopen(temp, "wb");
        }
        else
		{
			fileOut = fopen(argv[i + 1], "wb");
		}

		if(temp[0] == 0) printProcessing(argv[i], argv[i + 1]);
		else printProcessing(argv[i], temp);

		KCSencode(fileIn, fileOut, baudMode, stopBits, leader, sineWave, wavHeader);

		fprintf(stdout, "Done!");
	}
	else    //DECODING
	{
		char temp[256] = {0};
		fileIn = fopen(argv[i], "rb");

		if(fileIn == NULL)
		{
			printErrorFile(argv[i]);
			exit(1);
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

		if(temp[0] == 0) printProcessing(argv[i], argv[i + 1]);
		else printProcessing(argv[i], temp);

		KCSdecode(fileIn, fileOut, baudMode, stopBits);
		fprintf(stdout, "Done!");
	}


	fclose(fileIn);
	fclose(fileOut);
	return 0;
}

void printHeaderInfo()
{
	fprintf(stdout, "\nKCS version 1.1\t\tBy Veljko Gajic 24.01.2025\n\n");
}

void printUsage(int errorCode)
{
	printHeaderInfo();
	fprintf(stdout, "Usage: KCS [options] [input file] [output file]\n\n");
	fprintf(stdout, "-m	make a raw wavefile (encode)\n-w\tinclude the wav header\n-t	sine wave\n-ln	leader (sec)\n-bn	baudrate 0=300, 1=600, 2=1200\n-s	1 stop bit\n\n");
	fprintf(stdout, "Default: 300 baud, 1 start bit, 8 data bits,\n	 2 stop bits, square wave\n\n");
    fprintf(stdout, "Conversion utility for Kansas City Standard\ntapes. Input/output in an unsigned 8-bit\nmono PCM wavefile recorded at 22,050\nsamples per second.\n");
    exit(errorCode);
}

void printProcessing(char *inputFile, char *outputFile)
{
	printHeaderInfo();
	fprintf(stdout, "Input file:	%s\nOutput file:	%s\n\nProcessing...\n\n", inputFile, outputFile);
}

void printErrorFile(char *str)
{
	printHeaderInfo();
	fprintf(stdout, "File %s not found!\n", str);
}
