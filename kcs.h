#include <stdio.h>


void KCSencode(FILE *fileIn, FILE *fileOut, unsigned char baudMode, unsigned char stopBits, unsigned char leader, unsigned char BOOLsineWave);
void KCSdecode(FILE *fileIn, FILE *fileOut, unsigned char baudMode, unsigned char stopBits);
