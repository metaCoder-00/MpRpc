#ifndef MPRPC_SRC_INCLUDE_MPRPCPROVIDER_H_
#define MPRPC_SRC_INCLUDE_MPRPCPROVIDER_H_

#include <string>
#include <unordered_map>
#include <functional>
#include "google/protobuf/descriptor.h"
#include "google/protobuf/service.h"
#include "muduo/net/TcpServer.h"
#include "muduo/net/EventLoop.h"
#include "muduo/net/InetAddress.h"
#include "muduo/net/TcpConnection.h"

// rpc service distribution class
class RpcProvider {
public:
    // rpc methmod distribution API
    void NotifyService(google::protobuf::Service* service);

    // start rpc service node
    void Run();
private:
    // event loop
    muduo::net::EventLoop event_loop_;

    struct ServiceInfo {
        google::protobuf::Service* service_ptr_;    // service object ptr
        // service method map
        std::unordered_map<std::string, const google::protobuf::MethodDescriptor*> method_map_;
    };
    // notified service map
    std::unordered_map<std::string, ServiceInfo> service_map_;

    // new socket connection callback
    void OnConnection(const muduo::net::TcpConnectionPtr& conn);
    // connected users' r/w callback
    void OnMessage(const muduo::net::TcpConnectionPtr& conn, muduo::net::Buffer* buf, muduo::Timestamp t);
    // Closure callback fun, parse rpc response
    void SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, const google::protobuf::Message* response);
};

#endif // MPRPC_SRC_INCLUDE_MPRPCPROVIDER_H_