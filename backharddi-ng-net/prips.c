#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <arpa/inet.h>
#include "prips.h"

#if !defined(INET_ADDRSTRLEN)
#define INET_ADDRSTRLEN 16
#endif

/**********************************************/
/* Turn an IP address in dotted decimal into  */ 
/* a number.  This function also works  for   */
/* partial addresses.                         */
/**********************************************/
unsigned long numberize(const char *addr)
{
	unsigned long sin_addr;
	int retval;

	retval = inet_pton(AF_INET, addr, &sin_addr);

	/* invalid address or error in inet_pton() */
	if(retval == 0 || retval == -1)
		return -1; 

	return ntohl(sin_addr);	
}

/**********************************************/
/* Converts an IP address into dotted decimal */
/* format.  Note that this function cannot be */
/* used twice in one instruction (e.g. printf */
/* ("%s%s",denumberize(x),denumberize(y)));   */
/* because the return value is static.        */
/**********************************************/
const char *denumberize(unsigned long addr)
{
	static char buffer[INET_ADDRSTRLEN];
	unsigned long addr_nl = htonl(addr);
	
	if(!inet_ntop(AF_INET, &addr_nl, buffer, sizeof(buffer)))
		return NULL;
	return buffer;
}

/**********************************************/
/* Given start and end IP addresses, we try   */
/* to find the offset used to print the range */
/* in CIDR notation.                          */
/**********************************************/
const char *cidrize(unsigned long start, unsigned long end)
{
	unsigned long base;
	int offset = 0;
	static char *buffer[BUF_SIZE];

	/* find the mask (offset) by finding the 
	 * highest bit set differently in the start
	 * and end addresses. 
	 */
	unsigned long diff = start ^ end;
	
	/* find the highest bit set in diff */
	int i; 
	for(i=1; i <= 32; i++) 
	{
		if (diff == 0) 
		{
			offset = i - 1;
			break;
		}
		diff = diff >> 1;
	}

	/* clear out the bits below the mask */
	base = (start >> offset) << offset;

	snprintf((char *) buffer, BUF_SIZE, "%s/%d",
		denumberize(base), 32 - offset);

	return((char *) buffer);
}

/***********************************************/
/* Takes offset (number of bits from the left  */ 
/* of addr) and subtracts it from the number   */
/* of possible bits.  The number of possible   */
/* bits is the number of bits left for hosts.  */
/* We then return last host address.           */
/***********************************************/
unsigned long add_offset(const char *addr, int offset)
{
	unsigned long naddr;

	if(offset > 32 || offset < 0)
	{
		fprintf(stderr, "CIDR offsets are between 0 and 32\n");
		exit(1);
	}

	naddr = numberize(addr);
	if((naddr << offset) != 0) 
	{
	  fprintf(stderr, 
		"CIDR base address didn't start at subnet boundary\n");
	  exit(1);
	}

	return (int) pow(2, 32 - offset) + naddr -1;
}

unsigned long set_bits_from_right(int bits)
{
        register int i;
        unsigned long number = 0;

        for(i = 0; i < bits; i++)
                number += (int) pow(2, i);

        return number;
}

int count_on_bits(unsigned long number)
{
	unsigned long mask = 1;
	int i, on_bits = 0;

	mask <<= 31;
	for(i = 0; i < 32; i++)
	{
		if(mask & number)
			on_bits++;
		number <<= 1;
	}

	return on_bits; 
}
 
char get_class(unsigned long address)
{
	unsigned long addr;
        char class;

        addr = (address & 0xff000000) >> 24;

        if (addr < 127)
                class = 'a';
        else if (addr >= 127 && addr < 192)
                class = 'b';
        else if (addr >= 192 && addr < 224)
                class = 'c';
        else
                class = 'd';

	return class;
}
