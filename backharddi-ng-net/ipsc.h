#ifndef __IPSC_H__
#define __IPSC_H__


struct _network {
	/* Class information */
	char    class;

	/* Network information */
        unsigned int	addr;
        unsigned int    bits;
	unsigned int 	mask;

        /* Subnet information */
	unsigned int    subnet_max;
      	unsigned int    subnet_bits;
	unsigned int	subnet_bits_max;
	unsigned int	subnet_mask;

	/* Host specific information */
        unsigned int	host_bits;	
        unsigned int    host_max;
	unsigned int	host_addr;
	unsigned int	host_id;

	/* Miscellaneous */
	unsigned int 	cisco_wildcard;
	int		rfc1878; /* allow subnets of all ones or zeros */
	char		bitmap[35];
};

typedef struct _network Network;

/* 
 * Function prototypes
 */

Network *ipsc_network_new(void);
void ipsc_network_destroy(Network *n);

/*
 * ipsc_network_init_parse_text() takes a string that can be either an IP
 * address, and IP address with a CIDR offset, or an IP address and a
 * subnet mask.  The IP address and the mask (or offset) are delimited by
 * a forward slash.  Based on that string, the function will parse out the
 * addresses and call the appropriate ipsc_network_init_by_*() function.
 */
int ipsc_network_init_parse_text(Network *n, const char *text);

int ipsc_network_init_by_addr(Network *n, unsigned int addr);
int ipsc_network_init_by_addr_and_mask(Network *n, unsigned int addr, unsigned int mask);
int ipsc_network_init_by_interface(Network *n, const char *if_name);
int ipsc_network_init(Network *n);

int ipsc_network_set_class_info(Network *n, char class, const int reset_host);
int ipsc_network_set_bitmap(Network *n);

void ipsc_network_fprint_general(const Network *n, FILE *f);
void ipsc_network_fprint_subnets(const Network *n, FILE *f);
void ipsc_network_fprint_host(const Network *n, FILE *f);
void ipsc_network_fprint_cidr(const Network *n, FILE *f);
void ipsc_network_fprint_all(const Network *n, FILE *f);

int ipsc_network_get_supernet_max(const Network *n);
int ipsc_network_get_full_mask(const Network *n);
int ipsc_network_get_prefix_bits(const Network *n);
int ipsc_network_get_host_subnet_lbound(const Network *n);
int ipsc_network_get_host_subnet_ubound(const Network *n);
int ipsc_network_get_network_id(const Network *n);
int ipsc_network_get_subnet_id(const Network *n);
int ipsc_network_get_host_subnet_first_host(const Network *n);
int ipsc_network_get_host_subnet_last_host(const Network *n);

int ipsc_host_is_on_subnet(const unsigned int subnet_addr, const Network *n);

/* TODO: delete me? */
int incorrect_subnet_bits(const Network *n);

#endif
