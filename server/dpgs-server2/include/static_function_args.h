#ifndef __STATIC_FUNCTION_ARGS_H__
#define __STATIC_FUNCTION_ARGS_H__

//#include "vp_engine.h"

class ClientManager;
class ConnectionManager;

typedef struct StaticFunctionArgs {
    ClientManager*      clnt_mgr_ptr;
    ConnectionManager*  conn_mgr_ptr;
//    VPEngine*           vp_engine_ptr;
    pthread_t           tid;
} SFA;

#endif  // __STATIC_FUNCTION_ARGS_H__
