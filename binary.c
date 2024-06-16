#include <stdlib.h>
#include <math.h>


unsigned char *decToBin(unsigned char dec)
{
	int i, j, temp;
	unsigned char *bin;
	
	temp = 0;
	bin = (char*)malloc(8);
	
	for (i = 7; i >= 0; --i)
	{
		j = dec >> i;
		
		if (j & 1)
		  *(bin + temp) = 1;
		else
		  *(bin + temp) = 0;
		  
		++temp;
	}
	
	return bin;
}

unsigned char binToDec(unsigned char *bin)
{
	int i, dec;
	
	dec = 0;
	
	for(i = 7; i >= 0; --i)
	{
		if(*(bin + i) == 1)
		dec += pow(2, 8 - i - 1);
	}
	
	return dec;
}
