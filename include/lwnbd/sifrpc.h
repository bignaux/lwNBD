#ifndef SERVERS_SIFRPC_SIFRPC_H_
#define SERVERS_SIFRPC_SIFRPC_H_

#include <sifcmd.h>
#include <sifman.h>

// void sceSifAddCmdHandler(int cid, SifCmdHandler_t handler, void *harg);
// struct sifrpc_handler {
//	int cid;
//	SifCmdHandler_t handler;
//	void *harg;
// };

/*
 * typedef void * (*SifRpcFunc_t)(int fno, void *buffer, int length);
 */
struct sifrpc_handler
{
    int sid;
    SifRpcFunc_t sifrpc_handler;
};

#endif /* SERVERS_SIFRPC_SIFRPC_H_ */
