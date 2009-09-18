#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>

#ifdef __sun
#include <sys/sockio.h>
#endif

int get_addr(int ioctrl, struct ifreq *ifr);

void *ifc_get_ifc_buf(int *buf_len)
{
	int sockfd, len, last_len, ifreq_size = sizeof(struct ifreq);
	struct ifconf ifc;	
	void *buf;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}

	/* Algorithm adapted from Stevens UNP. 
	 * Since we are not sure how many interfaces the system has, 
	 * we don't know what size buffer to allocate.  The ioctl() 
	 * will not return an error if the buffer is too small, it will
	 * just give us everything that will fit in the buffer.  So, we
	 * have to get the interface information at least twice. 
	 * 
	 * The first time we get the interface info with a buffer of the
	 * legnth of one hundred ifreq structures. A guess.  Each time
	 * after, we increase the buffer size to fit ten more ifreq
	 * structures.  If we get a larger length buffer the last buffer
	 * wasn't large enough and we go through another iteration,
	 * otherwise, we were right the last time around and we stop.
	 */
	last_len = 0;
	len = 100 * ifreq_size;
	while(1)  
	{
		if(!(buf = malloc(len)))
		{
			perror("malloc");
			exit(1);
		}
	
		ifc.ifc_buf = buf;
		ifc.ifc_len = len;
		if( ioctl(sockfd, SIOCGIFCONF, &ifc) < 0)
		{
			if(errno == EINVAL || last_len == 0)
			{
				perror("ioctl");
				exit(1);
			}
		}
		else
		{
			if(ifc.ifc_len == last_len)
				break;
			last_len = ifc.ifc_len;
		}
		len += 10 * ifreq_size;
		free(buf);
	}
	
	*buf_len = ifc.ifc_len;
	return ifc.ifc_buf;
}

int ifc_get_addr(struct ifreq *ifr)
{
	return get_addr(SIOCGIFADDR, ifr); 
}

int ifc_get_mask(struct ifreq *ifr)
{
	return get_addr(SIOCGIFNETMASK, ifr);
}

int get_addr(int ioctrl, struct ifreq *ifr)
{
	struct ifreq ifrtmp; 
	struct sockaddr_in *sin;
	int sockfd;

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("socket");
		exit(1);
	}

	ifrtmp = *ifr;
	if( ioctl(sockfd, ioctrl, &ifrtmp))
	{
		perror("ioctl");
		exit(1);
	}

	close(sockfd);
	
	sin = (struct sockaddr_in *) &ifrtmp.ifr_addr;
	return ntohl(sin->sin_addr.s_addr);
}

char **ifc_get_names(int *if_count)
{
	int len, i = 0;
	int ifreq_size = sizeof(struct ifreq);
	struct ifreq *ifr;
	void *buf, *ptr;
	char **if_list = NULL;

	buf = ifc_get_ifc_buf(&len);

	ptr = buf;
	if_list = (char **) malloc(sizeof(char *) * (len/ifreq_size +1));
	while(ptr < buf + len)
	{
		ifr = (struct ifreq *) ptr;	
		ptr += ifreq_size;	

		if((if_list[i] = (char *) malloc(sizeof(char) 
			* strlen(ifr->ifr_name))) == NULL)
		{
			fprintf(stderr, 
				"get_interface_info(): couldn't malloc\n");
			return(NULL);
		}
		strcpy(if_list[i], ifr->ifr_name);
		i++;
	}
	
	if_list[i] = NULL;
	*if_count = i;
	
	free(buf);
	return(if_list);
}
