#include <stdio.h>
#include <stdbool.h>


void KCSencode(FILE *fileIn, FILE *fileOut, int baudMode, int stopBits, int leader, bool sineWave, bool wavHeader);
void KCSdecode(FILE *fileIn, FILE *fileOut, int baudMode, int stopBits);
