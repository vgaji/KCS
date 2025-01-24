#include <stdlib.h>
#include <stdbool.h>


bool *decToBin(unsigned char dec)
{
	bool *bin = (bool*)malloc(sizeof(bool) * 8);
	
	for (int i = 7; i >= 0; --i)
		bin[7 - i] = (dec >> i) & 1;
	
	return bin;
}

unsigned char binToDec(bool *bin)
{
	unsigned char dec = 0;
	
	for (int i = 0; i < 8; ++i)
		if (bin[i])
			dec |= (1 << (7 - i));
	
	return dec;
}
