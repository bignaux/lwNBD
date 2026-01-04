#include <lwnbd/ee_lib.h>
#include <netman.h>
#include <ps2ips.h> // RPC

// for IP config
#define IP_ADDR "192.168.1.18"
#define NETMASK "255.255.255.0"
#define GATEWAY "192.168.1.254"

////////////////////////////////////////////////////////////////////////
#define IPCONF_MAX_LEN 64 // Don't reduce even more this value

// Make sure the "cached config" is in the data section
// To prevent it from being "zeroed" on a restart of ps2link
char if_conf[IPCONF_MAX_LEN] __attribute__((section(".data"))) = "";
int if_conf_len __attribute__((section(".data"))) = 0;

////////////////////////////////////////////////////////////////////////
// void set_ipconfig(void)
//{
//     memset(if_conf, 0, IPCONF_MAX_LEN);
//     if_conf_len = 0;
//
//     strncpy(&if_conf[if_conf_len], IP_ADDR, 15);
//     if_conf_len += strlen(IP_ADDR) + 1;
//     strncpy(&if_conf[if_conf_len], NETMASK, 15);
//     if_conf_len += strlen(NETMASK) + 1;
//     strncpy(&if_conf[if_conf_len], GATEWAY, 15);
//     if_conf_len += strlen(GATEWAY) + 1;
// }

int set_ipconfig()
{
    t_ip_info ip_info;
    int result;

    // Initialize NETMAN
    NetManInit();

    //    memset(&ip_info, 0, sizeof(t_ip_info));
    if ((result = ps2ip_getconfig("sm0", &ip_info)) >= 0) {
        ip_info.dhcp_enabled = 1;
        return ps2ip_setconfig(&ip_info);
    }
    return -1;
}
