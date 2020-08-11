#include <iostream>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "mprpcchannel.h"

int main(int argc, char** argv) {
    MprpcApplication::Init(argc, argv);     // Init framwork

    // call rpc method Login
    pb::UserServiceRpc_Stub stub(new MprpcChannel());
    pb::LoginRequest login_request;
    login_request.set_name("caller");
    login_request.set_pwd("password");
    pb::LoginResponse login_response;
    stub.Login(nullptr, &login_request, &login_response, nullptr);      // RpcChannel->RpcChannel::CallMethod

    // read response
    if (!login_response.result().error_code()) {
        std::cout << "rpc login response success: " << login_response.success() << std::endl;
    } else {
        std::cout << "rpc login response error: " << login_response.result().error_message() << std::endl;
    }

    // call rpc method Register
    pb::RegisterRequest register_request;
    register_request.set_id(12345);
    register_request.set_name("registor");
    register_request.set_pwd("password");
    pb::RegisterResponse register_response;
    stub.Register(nullptr, &register_request, &register_response, nullptr);

    if (!register_response.result().error_code()) {
        std::cout << "rpc register response success: " << register_response.success() << std::endl;
    } else {
        std::cout << "rpc register response error: " << register_response.result().error_message() << std::endl;
    }

    return 0;
}