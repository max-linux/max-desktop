#ifndef __IFC_H__
#define __IFC_H__

#include <sys/socket.h>
#include <net/if.h>

char **ifc_get_names(int *if_count);
void *ifc_get_ifc_buf(int *buf_len);

int ifc_get_addr(struct ifreq *ifr);
int ifc_get_mask(struct ifreq *ifr);

#endif
