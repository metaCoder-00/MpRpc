#include "mprpcprovider.h"
#include "mprpcheader.pb.h"
#include "mprpcapplication.h"
#include "zookeeperutil.h"
#include "logger.h"

void RpcProvider::NotifyService(google::protobuf::Service* service) {
    // get service object descriptor
    auto service_descriptor_ptr = service->GetDescriptor();
    // get service name
    const std::string service_name = service_descriptor_ptr->name();
    int method_num = service_descriptor_ptr->method_count();

    LOG_INFO("service name: %s", service_name.c_str());

    ServiceInfo service_info;
    service_info.service_ptr_ = service;
    for (int i = 0; i < method_num; ++i) {
        auto method_descriptor_ptr = service_descriptor_ptr->method(i);
        const std::string method_name = method_descriptor_ptr->name();
        service_info.method_map_.insert({method_name, method_descriptor_ptr});

        LOG_INFO("method name: %s", method_name.c_str());
    }
    service_map_.insert({service_name, service_info});
}

void RpcProvider::Run() {
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserver_ip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserver_port").c_str());
    muduo::net::InetAddress address(ip, port);

    // create TCP server object
    muduo::net::TcpServer server(&event_loop_, address, "RpcProvider");
    // bind connection callback and message r/w method
    server.setConnectionCallback(std::bind(&RpcProvider::OnConnection, this, std::placeholders::_1));
    server.setMessageCallback(std::bind(&RpcProvider::OnMessage, this, std::placeholders::_1,
                              std::placeholders::_2, std::placeholders::_3));

    // set muduo thread nums
    server.setThreadNum(2);

    // register rpc server to the zookeeper
    ZkClient zk_client;
    zk_client.Start();
    // service_name is a permanent node, method_name is a temporary node
    for (auto& service_pair : service_map_) {
        // /service_name
        std::string service_path = "/" + service_pair.first;
        zk_client.Create(service_path.c_str(), nullptr, 0);
        for (auto& method_pair : service_pair.second.method_map_) {
            // /service_name/method_name
            std::string method_path = service_path + "/" + method_pair.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            // ZOO_EPHEMERAL: temporary node
            zk_client.Create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    // Debug info.
    LOG_INFO("RPC server ip: %s", ip.c_str());
    LOG_INFO("RPC server port: %d", port);

    // start service
    server.start();
    event_loop_.loop();
}

// socket connection callback fun.
void RpcProvider::OnConnection(const muduo::net::TcpConnectionPtr& conn) {
    if (!conn->connected()) {
        // rpc disconnection
        conn->shutdown(); 
    }
}
/*
    request header:
        header_size(uint32) + header_str + args_str
*/
//connected user's r/w callback fun
void RpcProvider::OnMessage(const muduo::net::TcpConnectionPtr& conn, 
                            muduo::net::Buffer* buf, 
                            muduo::Timestamp t) {
    // recived remote rpc request byte flow
    std::string recive_buf = buf->retrieveAllAsString();

    // read infront 4 bytes
    uint32_t header_size = 0;
    recive_buf.copy(reinterpret_cast<char*>(&header_size), 4, 0);
    // read data header origin string by header size
    std::string rpc_header_str = recive_buf.substr(4, header_size);
    // parse rpc header
    mprpc::RpcHeader rpc_header;
    std::string service_name, method_name;
    uint32_t args_size = 0;
    if (rpc_header.ParseFromString(rpc_header_str)) {
        // parse successed
        service_name = rpc_header.service_name();
        method_name = rpc_header.method_name();
        args_size = rpc_header.args_size();
    } else {
        // parse failed
        LOG_ERROR("rpc_header_str: %s parse error!", rpc_header_str.c_str());
        return;
    }

    // read args string 
    std::string args_str = recive_buf.substr(4 + header_size, args_size);

    // Debug log
    LOG_INFO("rpc_header_str: %s", rpc_header_str.c_str());
    LOG_INFO("header_size: %d", header_size);
    LOG_INFO("service_name: %s", service_name.c_str());
    LOG_INFO("method_name: %s", method_name.c_str());
    LOG_INFO("args_size: %d", args_size);
    LOG_INFO("args_str: %s", args_str.c_str());

    // get service object and method object
    auto service_map_itr = service_map_.find(service_name);
    if (service_map_itr == service_map_.end()) {
        LOG_ERROR("%s not found!", service_name.c_str());
        return;
    }

    auto method_map_itr = service_map_itr->second.method_map_.find(method_name);
    if (method_map_itr == service_map_itr->second.method_map_.end()) {
        LOG_ERROR("%s: %s not found!", service_name.c_str(), method_name.c_str());
        return;
    }

    google::protobuf::Service* service_ptr = service_map_itr->second.service_ptr_;
    const google::protobuf::MethodDescriptor* method_ptr = method_map_itr->second;

    // generate rpc method call request and response args
    google::protobuf::Message* request = service_ptr->GetRequestPrototype(method_ptr).New();
    if (!request->ParseFromString(args_str)) {
        LOG_ERROR("request parse error!, content: %s", args_str.c_str());
        return;
    }
    google::protobuf::Message* response = service_ptr->GetResponsePrototype(method_ptr).New();

    // bind closure callback fun. to parse and send response message
    auto done = google::protobuf::NewCallback<RpcProvider, 
                                              const muduo::net::TcpConnectionPtr&,
                                              const google::protobuf::Message*>(this, 
                                                                                &RpcProvider::SendRpcResponse, 
                                                                                conn, response);

    service_ptr->CallMethod(method_ptr, nullptr, request, response, done);
}

// closure callback fun. parse and send response
void RpcProvider::SendRpcResponse(const muduo::net::TcpConnectionPtr& conn, 
                                  const google::protobuf::Message* response) {
    std::string response_str;
    if (response->SerializeToString(&response_str)) {
        // serialized successful
        conn->send(response_str);
    } else {
        LOG_ERROR("serialize response_str error!");
    }
    conn->shutdown();   // disconnect http
}