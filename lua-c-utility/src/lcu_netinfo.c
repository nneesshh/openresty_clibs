#include "lcu_netinfo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lcu_platform.h"

#define NET_INFO_NUM_MAX 4
static netinfo_t s_net_info_cache[NET_INFO_NUM_MAX] = { 0 };

#if defined(LCU_PLATFORM_WIN32) || defined(LCU_PLATFORM_WINRT)
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <IphlpApi.h>
#ifdef _MSC_VER
#pragma comment(lib, "Iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#endif

netinfo_t *
get_netinfo(int *out_num)
{
	int num = 0;
	char mac[32] = { '\0' };

	PIP_ADAPTER_ADDRESSES pAddrs = NULL;
	ULONG buflen = 0;
	GetAdaptersAddresses(AF_INET, 0, NULL, pAddrs, &buflen);
	pAddrs = (PIP_ADAPTER_ADDRESSES)malloc(buflen);
	GetAdaptersAddresses(AF_INET, 0, NULL, pAddrs, &buflen);

	PIP_ADAPTER_INFO pInfos = NULL;
	buflen = 0;
	GetAdaptersInfo(pInfos, &buflen);
	pInfos = (PIP_ADAPTER_INFO)malloc(buflen);
	GetAdaptersInfo(pInfos, &buflen);

	netinfo_t *pNetinfo = s_net_info_cache;
	PIP_ADAPTER_ADDRESSES pAddr = pAddrs;
	while (pAddr) {
		snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
			pAddr->PhysicalAddress[0],
			pAddr->PhysicalAddress[1],
			pAddr->PhysicalAddress[2],
			pAddr->PhysicalAddress[3],
			pAddr->PhysicalAddress[4],
			pAddr->PhysicalAddress[5]);

		memset(pNetinfo, 0, sizeof(netinfo_t));
		strncpy(pNetinfo->name, pAddr->AdapterName, sizeof(pNetinfo->name));
		strncpy(pNetinfo->ip, "0.0.0.0", sizeof(pNetinfo->ip));
		strncpy(pNetinfo->mask, "0.0.0.0", sizeof(pNetinfo->ip));
		strncpy(pNetinfo->mac, mac, sizeof(pNetinfo->mac));

		pAddr = pAddr->Next;

		++pNetinfo;
		++num;

		if (num >= NET_INFO_NUM_MAX)
			break;
	}

	PIP_ADAPTER_INFO pInfo = pInfos;
	int i;
	while (pInfo) {
		for (i =0; i<num; ++i) {
			pNetinfo = &s_net_info_cache[i];
			if (0 == strcmp(pNetinfo->name, pInfo->AdapterName)) {
				// description more friendly
				strncpy(pNetinfo->name, pInfo->Description, sizeof(pNetinfo->name));
				strncpy(pNetinfo->ip, pInfo->IpAddressList.IpAddress.String, sizeof(pNetinfo->ip));
				strncpy(pNetinfo->mask, pInfo->IpAddressList.IpMask.String, sizeof(pNetinfo->mask));
			}
		}

		pInfo = pInfo->Next;
	}

	free(pAddrs);
	free(pInfos);

	// fill loop back
	for (i = 0; i < num; ++i) {
		pNetinfo = &s_net_info_cache[i];
		if (strcmp(pNetinfo->ip, "0.0.0.0") == 0 ||
			strcmp(pNetinfo->ip, "127.0.0.1") == 0 ||
			strcmp(pNetinfo->mac, "00:00:00:00:00:00") == 0) {
			continue;
		}
	}
	*out_num = num;
	return s_net_info_cache;
}
#elif defined(LCU_PLATFORM_LINUX) || defined(LCU_PLATFORM_ANDROID) || defined(LCU_PLATFORM_DARWIN) 
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <linux/if.h>
#include <arpa/inet.h>

netinfo_t *
get_netinfo(int *out_num)
{
	int num = 0;
	char macaddr[32] = { '\0' };

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		return NULL;
	}

	struct ifconf ifc;
	char buf[1024];
	ifc.ifc_len = sizeof(buf);
	ifc.ifc_buf = buf;

	int iRet = ioctl(sock, SIOCGIFCONF, &ifc);
	if (iRet != 0) {
		close(sock);
		return NULL;
	}

	int count = ifc.ifc_len / sizeof(struct ifreq);
	//printf("ifc.size=%d\n", count);
	if (count == 0) {
		close(sock);
		return NULL;
	}

	netinfo_t *pNetinfo;
	struct ifreq ifr;
	int i;
	for (i = 0; i < count; ++i) {
		memset(pNetinfo, 0, sizeof(netinfo_t));

		// name
		strcpy(ifr.ifr_name, ifc.ifc_req[i].ifr_name);
		//printf("name: %s\n", ifr.ifr_name);
		strncpy(pNetinfo->name, ifr.ifr_name, sizeof(pNetinfo->name));

		// flags
		//iRet = ioctl(sock, SIOCGIFFLAGS, &ifr);
		//short flags = ifr.ifr_flags;
		
		// addr
		iRet = ioctl(sock, SIOCGIFADDR, &ifr);
		struct sockaddr_in* addr = (struct sockaddr_in*)&ifr.ifr_addr;
		char* ip = inet_ntoa(addr->sin_addr);
		//printf("ip: %s\n", ip);
		strncpy(pNetinfo->ip, ip, sizeof(pNetinfo->ip));

		// netmask
		iRet = ioctl(sock, SIOCGIFNETMASK, &ifr);
		addr = (struct sockaddr_in*)&ifr.ifr_netmask;
		char* netmask = inet_ntoa(addr->sin_addr);
		//printf("netmask: %s\n", netmask);
		strncpy(pNetinfo->mask, netmask, sizeof(pNetinfo->mask));

		// broadaddr
		//iRet = ioctl(sock, SIOCGIFBRDADDR, &ifr);
		//addr = (struct sockaddr_in*)&ifr.ifr_broadaddr;
		//char* broadaddr = inet_ntoa(addr->sin_addr);
		//printf("broadaddr: %s\n", broadaddr);
		
		// hwaddr
		iRet = ioctl(sock, SIOCGIFHWADDR, &ifr);
		snprintf(macaddr, sizeof(macaddr), "%02x:%02x:%02x:%02x:%02x:%02x",
			(unsigned char)ifr.ifr_hwaddr.sa_data[0],
			(unsigned char)ifr.ifr_hwaddr.sa_data[1],
			(unsigned char)ifr.ifr_hwaddr.sa_data[2],
			(unsigned char)ifr.ifr_hwaddr.sa_data[3],
			(unsigned char)ifr.ifr_hwaddr.sa_data[4],
			(unsigned char)ifr.ifr_hwaddr.sa_data[5]);
		//printf("mac: %s\n", macaddr);
		strncpy(pNetinfo->mac, macaddr, sizeof(pNetinfo->mac));
		//printf("\n");

		if (strcmp(pNetinfo->ip, "0.0.0.0") == 0 ||
			strcmp(pNetinfo->ip, "127.0.0.1") == 0 ||
			strcmp(pNetinfo->mac, "00:00:00:00:00:00") == 0) {
			continue;
		}

		++pNetinfo;
		++num;

		if (num >= NET_INFO_NUM_MAX)
			break;
	}

	close(sock);

	*out_num = num;
	return s_net_info_cache;
}
#else
netinfo_t *
get_netinfo(int *out_num)
{
	// not implemented yet
	*out_num = -1;
	return NULL
}
#endif

int
test()
{
	int num;
	netinfo_t *item = get_netinfo(&num);
	int i;
	for (i = 0; i < num; ++i) {
		printf("%s\nip: %s\nmask: %s\nmac: %s\n\n",
			item->name,
			item->ip,
			item->mask,
			item->mac);
		++item;
	}
	return 0;
}