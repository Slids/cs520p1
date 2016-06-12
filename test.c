#include <stdio.h>
#include <stdlib.h>

int main()
{
	unsigned int x = 0x11;
	unsigned int y = 0x11;
	printf("%x\n", (y << 8) + x);
}
