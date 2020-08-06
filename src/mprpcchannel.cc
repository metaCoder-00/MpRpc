#include "mprpcchannel.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <error.h>
#include <string>
#include "rpcheader.pb.h"
#include "mprpcapplication.h"

// header_size(4 bytes) + service_name method_name args_size + args
void MprpcChannel::CallMethod(const google::protobuf::MethodDescriptor* method,
                              google::protobuf::RpcController* controller, 
                              const google::protobuf::Message* request,
                              google::protobuf::Message* response, 
                              google::protobuf::Closure* done) {
    const google::protobuf::ServiceDescriptor* service_ptr = method->service();
    std::string service_name = service_ptr->name();
    std::string method_name = method->name();

    // get args_size
    std::string args_str;
    uint32_t args_size = 0;
    if (request->SerializeToString(&args_str)) {
        args_size = args_str.size();
    } else {
        controller->SetFailed("serialize request error!");
        return;
    }

    // rpc request header
    mprpc::RpcHeader rpc_header;
    uint32_t header_size = 0;
    rpc_header.set_service_name(service_name);
    rpc_header.set_method_name(method_name);
    rpc_header.set_args_size(args_size);

    std::string rpc_header_str;
    if (rpc_header.SerializeToString(&rpc_header_str)) {
        header_size = rpc_header_str.size();
    } else {
        controller->SetFailed("serialize rpc header error!");
        return;
    }

    // generate rpc request string
    std::string send_rpc_str;
    send_rpc_str.insert(0, std::string(reinterpret_cast<char*>(&header_size), 4));  // header size
    send_rpc_str += rpc_header_str;     // header
    send_rpc_str += args_str;   // args

    // Debug log
    std::cout << "===============DEBUG Info===================" << std::endl;
    std::cout << "header_size: " << header_size << std::endl;
    std::cout << "rpc_header_str: " << rpc_header_str << std::endl;
    std::cout << "service_name: " << service_name << std::endl;
    std::cout << "method_name: " << method_name << std::endl;
    std::cout << "args_size: " << args_size << std::endl;
    std::cout << "args_str: " << args_str << std::endl;
    std::cout << "============================================" << std::endl;

    // TCP socket 
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);   // client file descriptor
    if (-1 == client_fd) {
        std::string error_text = "create socket error! errno: ";
        error_text += std::to_string(errno);
        controller->SetFailed(error_text);
        return;
    }

    // read rpc server config
    std::string ip = MprpcApplication::GetInstance().GetConfig().Load("rpcserver_ip");
    uint16_t port = atoi(MprpcApplication::GetInstance().GetConfig().Load("rpcserver_port").c_str());

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());

    // build a new conncection
    if (-1 == connect(client_fd, reinterpret_cast<sockaddr*>(&server_addr), sizeof(server_addr))) {
        close(client_fd);
        std::string error_text = "connect error! errno: ";
        error_text += std::to_string(errno);
        controller->SetFailed(error_text);
        return;
    }

    // send rpc request
    if (-1 == send(client_fd, send_rpc_str.c_str(), send_rpc_str.size(), 0)) {
        close(client_fd);
        std::string error_text = "send error! errno: ";
        error_text += std::to_string(errno);
        controller->SetFailed(error_text);
        return;
    }

    // receive rpc response
    char recv_buf[1024] = {0};
    int recv_size = 0;
    if (-1 == (recv_size = recv(client_fd, recv_buf, 1024, 0))) {
        close(client_fd);
        std::string error_text = "receive error! errno: ";
        error_text += std::to_string(errno);
        controller->SetFailed(error_text);
        return;
    }

    // parse rpc response
    if (!response->ParseFromArray(recv_buf, recv_size)) {
        close(client_fd);
        std::string error_text = "parse error! response_str: ";
        error_text += std::to_string(errno);
        controller->SetFailed(error_text);
        return;
    }

    close(client_fd);
}