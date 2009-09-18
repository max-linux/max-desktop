#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>

#include "ipsc.h"
#include "ifc.h"
#include "prips.h"

#define DOT(x) (8*(x))+((x)-1)

int ipsc_network_set_bitmap(Network *n)
{
	int i, dots = 0;
	char bitcode = 'n';

	n->bitmap[DOT(1)] = '.';
	n->bitmap[DOT(2)] = '.';
	n->bitmap[DOT(3)] = '.';

	for(i = 0; i < 35; i++)
	{
		if(i == DOT(1) || i == DOT(2) || i == DOT(3))
		{
			dots++;
			continue;
		}
		
		if(i == (n->bits + dots))
			bitcode = 's';
		
		if(i == (n->bits + n->subnet_bits + dots))
			bitcode = 'h';
	
		n->bitmap[i] = bitcode;
	}
	return(0);
}

/* 
 * Address size - network bits will get the maximum number 
 * of bits that can be allocated for subnets.  If the subnet
 * bits specified are greater than that amount we have a 
 * problem.
 */
int incorrect_subnet_bits(const Network *n)
{
	return(n->subnet_bits > (32 - n->bits) ? 1 : 0);
}

void ipsc_network_fprint_general(const Network *n, FILE *f)
{
	int full_mask = ipsc_network_get_full_mask(n);

        /* Network information */
        fprintf(f, "%-26s%-10c\n", "Network class:", n->class);
        fprintf(f, "%-26s%-10s\n", "Network mask:", denumberize(n->mask));
        fprintf(f, "%-26s%-10X\n", "Network mask (hex):", n->mask);
        fprintf(f, "%-26s%-10s\n", "Network address:",
		denumberize( ipsc_network_get_full_mask(n) & n->host_addr));

        /* Subnet information */
        fprintf(f, "%-26s%-10d\n", "Subnet bits:", n->subnet_bits);
        fprintf(f, "%-26s%-10d\n", "Max subnets:", n->subnet_max);
        fprintf(f, "%-26s%-10s\n", "Full subnet mask:", denumberize(full_mask));
        fprintf(f, "%-26s%-10X\n", "Full subnet mask (hex):", full_mask);

        /* Host information */
        fprintf(f, "%-26s%-10d\n", "Host bits:", n->host_bits);
        fprintf(f, "%-26s%-10d\n", "Addresses per subnet:", n->host_max);

        /* Bit mask */
        fprintf(f, "%-26s%-10s\n", "Bit map:", n->bitmap);

	fprintf(f, "\n");
}

void ipsc_network_fprint_subnets(const Network *n, FILE *f)
{
        unsigned int i;
        int subnet_addr = n->addr;
	char subnet_buf[25];

        for(i = 0; i < n->subnet_max; i++)
        {
		snprintf(subnet_buf, 25, "Subnet %d:", i + 1);
		fprintf(f, "%-26s", subnet_buf);
                fprintf(f, "%-17s", denumberize(subnet_addr));
                fprintf(f, "%-17s", denumberize((subnet_addr + n->host_max) - 1));
                
		if(ipsc_host_is_on_subnet(subnet_addr, n))
                        fprintf(f, "*");
                
		fprintf(f, "\n");
		subnet_addr += n->host_max;
        }

	fprintf(f, "\n");
}

int ipsc_host_is_on_subnet(const unsigned int subnet_addr, const Network *n)
{
	return (ipsc_network_get_host_subnet_lbound(n) == subnet_addr);
}

void ipsc_network_fprint_subnets_from_supernet(const Network *n, FILE *f)
{
	int i;

	for(i = 0; i < ipsc_network_get_supernet_max(n); i++)
	{

	}
}

void ipsc_network_fprint_host(const Network *n, FILE *f)
{
	fprintf(f, "%-26s%-10s\n", "IP address:", denumberize(n->host_addr));
	fprintf(f, "%-26s%-10X\n", "Hexadecimal IP address:", n->host_addr);
	
	fprintf(f, "%-26s%-10s", "Host allocation range:", denumberize(ipsc_network_get_host_subnet_first_host(n)));
	fprintf(f, " - %s\n", denumberize(ipsc_network_get_host_subnet_last_host(n)));
        
	fprintf(f, "%-26s%-10s\n", "Full subnet mask:", 
		denumberize( ipsc_network_get_full_mask(n)));
	fprintf(f, "%-26s%-10s\n", "Subnet mask:", denumberize(n->subnet_mask));
	fprintf(f, "%-26s%-10s\n", "Subnet ID:", 
		denumberize( ipsc_network_get_subnet_id(n)));
	fprintf(f, "%-26s%-10s\n", "Network ID:", 
		denumberize( ipsc_network_get_network_id(n)));
	fprintf(f, "%-26s%-10s\n", "Host ID:", denumberize(n->host_id));
	fprintf(f, "\n");
}

void ipsc_network_fprint_cidr(const Network *n, FILE *f)
{
	fprintf(f, "%-26s%-10s/%d\n", "CIDR notation:", denumberize(n->addr),
			ipsc_network_get_prefix_bits(n));
	fprintf(f, "%-26s%-10d\n", "Supernet max:", ipsc_network_get_supernet_max(n));
	fprintf(f, "%-26s%-10s\n", "Cisco wildcard:", denumberize(n->cisco_wildcard));
	fprintf(f, "%-26s%-10s/%d\n", "Classful network:", denumberize(n->addr), n->bits);
	fprintf(f, "%-26s%-10s / ", "Route/Mask:", denumberize(n->addr));
	fprintf(f, "%s\n", denumberize( ipsc_network_get_full_mask(n)));
	fprintf(f, "%-26s%-10X / %X\n", "Hexadecimal route/mask:", n->addr, 
			ipsc_network_get_full_mask(n));
	fprintf(f, "\n");
}

void ipsc_network_fprint_all(const Network *n, FILE *f)
{
	ipsc_network_fprint_general(n, f);
	ipsc_network_fprint_subnets(n, f);
	ipsc_network_fprint_host(n, f);
	ipsc_network_fprint_cidr(n, f);
}

int ipsc_network_init_by_interface(Network *n, const char *if_name)
{
	int len, found_flag = 0;
	int ifreq_size = sizeof(struct ifreq);
	struct ifreq *ifr;
	void *buf, *ptr;

	if(!if_name)
		return -1;

	buf = ifc_get_ifc_buf(&len);

	/*
	 * For each network interface, if the network interface
	 * matches the interface specified in the if_name arg then
	 * we get the interface address, the subnet mask, and we do
	 * some calculations.  We need to get the subnet mask to
	 * find the subnet bits.
	 */
	ptr = buf;
	while(ptr < buf + len)
	{
		ifr = (struct ifreq *) ptr;	
		if( strcmp(ifr->ifr_name, if_name) == 0)
		{
			unsigned int addr = ifc_get_addr(ifr);
			unsigned int mask = ifc_get_mask(ifr);
			
			ipsc_network_init_by_addr_and_mask(n, addr, mask);
		
			found_flag = 1;
			break;
		}
		ptr += ifreq_size;	
	}		
	
	free(buf);
	return found_flag ? 0 : -1;
}

/*
 * Set the basic information for the given network class.
 * We also assign a default value for the network address.
 * The default value of the network address is the first
 * network in the given class.
 */
int ipsc_network_set_class_info(Network *n, char class, const int set_defaults)
{
	switch(tolower(class))
	{
	case 'a':
      	n->class = 'A';
		n->bits = 8;
		n->addr = 0x01000000;
		n->mask = 0xff000000;
		break;
	case 'b':
		n->class = 'B';
		n->bits = 16;
		n->addr = 0x80000000;
		n->mask = 0xffff0000;
		break;
	case 'c':
		n->class = 'C';
		n->bits = 24;
		n->addr = 0xc0000000;
		n->mask = 0xffffff00;
		break;
	default:
		return -1;
	}

	if(set_defaults) 
		n->host_addr = n->addr +1;
	return 0;
}

Network *ipsc_network_new(void)
{
	Network *n = malloc( sizeof(Network) );
	memset(n, 0, sizeof(n));
	return n;
}

int ipsc_network_init_parse_text(Network *n, const char *text)
{
	char *addr_str = NULL;
	char *mask_str = NULL;
	unsigned int addr, mask;

	if((addr_str = strdup(text)) == NULL)
		return -1;

	if( strchr(addr_str, '/'))
	{
		 /* IP and mask or ip and offset */
		addr_str = strtok(addr_str, "/");
		mask_str = strtok(NULL, "/");
		
		if(!mask_str)
			return -1;
		
		if((addr = numberize(addr_str)) == -1)
			return -1;
	
		if( strchr(mask_str, '.'))
		{
			/* subnet mask */
			if((mask = numberize(mask_str)) == -1)
				return -1;
		}
		else
		{
			/* CIDR */
			int offset = atoi(mask_str);

			if(offset < 1 || offset > 32)
			{
				/* bad CIDR offset */
				return -1;
			}
		
			mask = set_bits_from_right(offset) 
				<< (32 - offset);
		}

		ipsc_network_init_by_addr_and_mask(n, addr, mask);
	}
	else
	{
		/* regular IP address */
		if((addr = numberize(addr_str)) == -1)
			return -1;
		ipsc_network_init_by_addr(n, addr);
	}

	free(addr_str);
	return 0;
}

int ipsc_network_init_by_addr(Network *n, unsigned int addr)
{
	char class = get_class(addr);
	
	/* 
	 * Make an assumption about subnet bits.  If
	 * we only have address, we assume there are
	 * no subnets to start
	 */
	n->subnet_bits = 0;

	n->host_addr = addr;
	ipsc_network_set_class_info(n, class, 0);
	ipsc_network_init(n);

	return 0;
}

int ipsc_network_init_by_addr_and_mask(Network *n, unsigned int addr, unsigned int mask)
{
	char class = get_class(addr);

	/* Host */
        n->host_addr = addr;
	
	/* Network */
	ipsc_network_set_class_info(n, class, 0);
	
	if(n->mask > mask) /* supernets */
	{
		n->mask = mask;
		n->bits = count_on_bits(mask);
		n->subnet_bits = 0;
	}
	else /* old school subnets or CIDR that looks it */
	{
		n->subnet_mask = n->mask^mask; 
		n->subnet_bits = count_on_bits(n->subnet_mask);
	}
	
	ipsc_network_init(n);
	return 0;
}

/*
 * Now that we have the basic information we need in n, we can 
 * make the rest of the calculations.  Note that this function 
 * is usually called after a call to one of the ipsc_network_set_*() 
 * functions.  Those functions set the network class information
 * and subnet information.
 */
int ipsc_network_init(Network *n)
{
	n->addr = n->mask & n->host_addr;

        n->host_bits = 32 - (n->bits + n->subnet_bits);
        n->host_max = pow(2, n->host_bits);
        n->host_id = n->host_addr - n->addr;

        n->subnet_mask = ~(n->mask | set_bits_from_right(n->host_bits));
        n->subnet_max = pow(2, n->subnet_bits);
        n->subnet_bits_max = 32 - n->bits;

        n->cisco_wildcard = ~(n->mask | n->subnet_mask);

        ipsc_network_set_bitmap(n);
	return 0;
}

int ipsc_network_get_supernet_max(const Network *n)
{
	Network *net = ipsc_network_new();
	int supernet_max;

	ipsc_network_set_class_info(net, n->class, 0);
	supernet_max = net->bits - n->bits;
	ipsc_network_destroy(net);
	return supernet_max ? pow(2, supernet_max) : 0;
}

int ipsc_network_get_full_mask(const Network *n)
{
	return(n->mask | n->subnet_mask);
}

int ipsc_network_get_prefix_bits(const Network *n)
{
	return(n->bits + n->subnet_bits);
}

void ipsc_network_destroy(Network *n)
{
	if(n) free(n);
}

int ipsc_network_get_host_subnet_lbound(const Network *n)
{
	unsigned int mask = ipsc_network_get_full_mask(n);
	return (mask & n->host_addr);
}

int ipsc_network_get_host_subnet_ubound(const Network *n)
{
	unsigned int mask = ipsc_network_get_full_mask(n);
        return (mask & n->host_addr) + n->host_max;
}

int ipsc_network_get_network_id(const Network *n)
{
	return n->mask & n->host_addr;
}

int ipsc_network_get_subnet_id(const Network *n)
{
	return n->subnet_mask & n->host_addr;
}

int ipsc_network_get_host_subnet_last_host(const Network *n)
{
	return ipsc_network_get_host_subnet_ubound(n) -2;
}

int ipsc_network_get_host_subnet_first_host(const Network *n)
{
	return ipsc_network_get_host_subnet_lbound(n) + 1;
}
