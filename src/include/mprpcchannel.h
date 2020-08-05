#ifndef MPRPC_SRC_INCLUDE_MPRPCCHANNEL_H_
#define MPRPC_SRC_INCLUDE_MPRPCCHANNEL_H_

#include "google/protobuf/service.h"

class MprpcChannel : public google::protobuf::RpcChannel {
public:
    // rpc method args serialization and send
    void CallMethod(const google::protobuf::MethodDescriptor* method,
                    google::protobuf::RpcController* controller, 
                    const google::protobuf::Message* request,
                    google::protobuf::Message* response, 
                    google::protobuf::Closure* done);
};

#endif  // MPRPC_SRC_INCLUDE_MPRPCCHANNEL_H_