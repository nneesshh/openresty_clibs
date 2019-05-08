#ifndef __LCU_NETINFO_H__
#define __LCU_NETINFO_H__

#ifdef __cplusplus
extern "C" {
#endif

typedef struct netinfo_s {
	char name[160];
	char ip[32];
	char mask[32];
	char mac[32];
} netinfo_t;

netinfo_t * get_netinfo(int *out_num);

#ifdef __cplusplus
}
#endif


#endif // __LCU_NETINFO_H__