#ifndef __prips_h__
#define __prips_h__

#if defined(__FreeBSD__) || defined(__NetBSD__)
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <arpa/inet.h>

#define BUF_SIZE 128

#define IPQUAD(addr) \
        ((unsigned char *)&addr)[3], \
        ((unsigned char *)&addr)[2], \
        ((unsigned char *)&addr)[1], \
        ((unsigned char *)&addr)[0]

unsigned long numberize(const char *addr);
const char *denumberize(unsigned long addr);
const char *cidrize(unsigned long start, unsigned long end);
unsigned long add_offset(const char *addr, int offset);
unsigned long denboize(struct sockaddr_in *s);
unsigned long set_bits_from_right(int bits);
int count_on_bits(unsigned long number);
char get_class(unsigned long address);

#endif /* __prips_h__ */
