#ifndef STATIC_FUNCTION_ARGS_H
#define STATIC_FUNCTION_ARGS_H

class ClientManager;
class ConnectionManager;

typedef struct StaticFunctionArgs {
    ClientManager*      clnt_mgr_ptr;
    ConnectionManager*  conn_mgr_ptr;
} SFA;

#endif  // STATIC_FUNCTION_ARGS_H
