#ifndef _SYS_SOCKET_H_
#define _SYS_SOCKET_H_

#include <ps2ip.h>

#define close lwip_close

/*
 * we redefine with standard prototype, as on modern lwip
 */
#undef send
static inline int send(int s, const void *dataptr, size_t size, int flags)
{
    return lwip_send(s, (void *)dataptr, (int)size, (unsigned int)flags);
}


/*
 *
 */
//#include "lwip/netif.h"

// extern struct netif gnetif;
//
// local_SubNet = ip4_addr_get_u32(netif_ip4_netmask(&gnetif));
//
// local_Gateway = ip4_addr_get_u32(netif_ip4_gw(&gnetif));
// printf("IP address: %s\n", ip4addr_ntoa(netif_ip4_addr(&gnetif)));


#endif /* _SYS_SOCKET_H_ */
