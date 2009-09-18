/* IP Subnet Calculator */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "ipsc.h"

const char *MAINTAINER = "dan@machlin.net";
const char *VERSION =
	"ipsc 0.4.3\n"
	"This program comes with NO WARRANTY, to the extent permitted\n" 
	"by law.  You may redistribute copies under the terms of the\n" 
	"GNU General Public License.\n";

Network n;
void usage(const char *prog);

int main(int argc, char *argv[])
{
	int ch;
	char *argstr = "C:B:r:i:agshcv";
	char *interface = "";
	
	/* flags */
	int print_all_flag = 0;
	int print_general_flag = 0;
	int print_subnet_flag = 0;
	int print_host_flag = 0;
	int print_cidr_flag = 0;
	int interface_flag = 0;
	int class_flag = 0;
	int bits_flag = 0;

	opterr = 0;
        while ((ch = getopt(argc, argv, argstr)) != EOF) {
                switch (ch) {
                case 'C':
			if( ipsc_network_set_class_info(&n, optarg[0], 1) < 0)
			{
				usage(argv[0]);
				exit(1);
			}
			class_flag = 1;
			break;
		case 'B':
			n.subnet_bits = atoi(optarg);
			bits_flag = 1;
			break;
		case 'i':
			interface_flag = 1;
			interface = optarg;
			break;
		case 'a':
			print_all_flag = 1;
			break;
		case 'g':
			print_general_flag = 1;
			break;
		case 's':
			print_subnet_flag = 1;
			break;
		case 'h':
			print_host_flag = 1;
			break;
		case 'c':
			print_cidr_flag = 1;
			break;
		case 'v':
			printf("%s", VERSION);
			exit(0);
		case '?':
			usage(argv[0]);
			exit(1);
		}
	}

	if((optind - argc) == 0) 
	{
		if(interface_flag || bits_flag || class_flag)
		{
			if(interface && ipsc_network_init_by_interface(&n, interface) < 0) 
			{
				fprintf(stderr, 
					"%s: could not find interface \"%s\"\n",
					argv[0], interface);
				exit(1);
			}
				
			if((bits_flag || class_flag) && !(bits_flag && class_flag))
			{
				fprintf(stderr, "%s: -C and -B must be used together\n",
					argv[0]);
				exit(1);
			}
		}
		else
		{
			usage(argv[0]);
			exit(1);
		}
	}
	else
	{
		if(interface_flag)
		{
			fprintf(stderr, "%s: too many interfaces\n", argv[0]);
			exit(1);
		}
		ipsc_network_init_parse_text(&n, argv[optind]);
	}
	
	ipsc_network_init(&n);
	if(incorrect_subnet_bits(&n))
	{
		fprintf(stderr, 
			"%s: too many subnet bits for network class\n", 
			argv[0]);
		exit(1);
	} 

	if(print_all_flag)
		ipsc_network_fprint_all(&n, stdout);

	if(print_general_flag)
		ipsc_network_fprint_general(&n, stdout);

	if(print_subnet_flag)
		ipsc_network_fprint_subnets(&n, stdout);
	
	if(print_host_flag)
		ipsc_network_fprint_host(&n, stdout);

	if(print_cidr_flag)
		ipsc_network_fprint_cidr(&n, stdout);

	if(!print_general_flag && !print_subnet_flag &&
	   !print_host_flag && !print_cidr_flag && !print_all_flag)
	{
		ipsc_network_fprint_general(&n, stdout);
	}

	return 0;
}

void usage(const char *prog)
{
        fprintf(stderr, "usage: %s [options] <addr/mask | addr/offset | addr>\n\
        -C <class>      Network class (a, b, or c).  Must be used with -B\n\
	-B <bits>	Subnet bits (must be used with -C)\n\
        -i <if>		Reverse engineer an interface (e.g. eth0)\n\
	-a		Print all information available\n\
	-g		Print general information\n\
        -s 		Print all possible subnets\n\
	-h		Print host information\n\
	-c		Print CIDR information\n\
        -v		Print the program version\n\
        -?		Print this help message\n\
        \n\
        \rReport bugs to %s\n",
                        prog, MAINTAINER);
}
