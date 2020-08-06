#include "rpcprovider.h"
#include "rpcheader.pb.h"
#include "mprpcapplication.h"
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

    // Debug info.
    std::cout << "RPC server ip: " << ip << std::endl;
    std::cout << "RPC server port: " << port << std::endl;

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
        std::cout << "rpc_header_str: " << rpc_header_str << " parse error!" << std::endl;
        return;
    }

    // read args string 
    std::string args_str = recive_buf.substr(4 + header_size, args_size);

    // Debug log
    std::cout << "===============DEBUG Info===================" << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_size: " << args_size << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "============================================" << std::endl;

    // get service object and method object
    auto service_map_itr = service_map_.find(service_name);
    if (service_map_itr == service_map_.end()) {
        std::cout << service_name << " not found!" << std::endl;
        return;
    }

    auto method_map_itr = service_map_itr->second.method_map_.find(method_name);
    if (method_map_itr == service_map_itr->second.method_map_.end()) {
        std::cout << service_name << ": " << method_name << "not found!" << std::endl;
        return;
    }

    google::protobuf::Service* service_ptr = service_map_itr->second.service_ptr_;
    const google::protobuf::MethodDescriptor* method_ptr = method_map_itr->second;

    // generate rpc method call request and response args
    google::protobuf::Message* request = service_ptr->GetRequestPrototype(method_ptr).New();
    if (!request->ParseFromString(args_str)) {
        std::cout << "request parse error!, content: " << args_str << std::endl;
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
        std::cout << "serialize response_str error!" << std::endl;
    }
    conn->shutdown();   // disconnect http
}