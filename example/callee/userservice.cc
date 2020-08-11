#include <iostream>
#include <string>
#include "user.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"

class UserService : public pb::UserServiceRpc {
private:
    bool Login(const std::string& name, const std::string& pwd) {
        std::cout << "local service: Login" << std::endl;
        std::cout << "name: " << name << " password: " << pwd << std::endl;
        return true;
    }

    bool Register(const uint32_t id, const std::string& name, const std::string& pwd) {
        std::cout << "local service: Register" << std::endl;
        std::cout << "user id: " << id << std::endl;
        std::cout << "name: " << name << " password: " << pwd << std::endl;
        return true;
    }
public:
    void Login(::google::protobuf::RpcController* controller,
                       const ::pb::LoginRequest* request,
                       ::pb::LoginResponse* response,
                       ::google::protobuf::Closure* done) {
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool login_success = Login(name, pwd);
        response->set_success(login_success);
        pb::ResultCode* result = response->mutable_result();
        if (login_success) {
            result->set_error_code(0);
            result->set_error_message("");
        } else {
            result->set_error_code(1);
            result->set_error_message("login error");
        }

        
        done->Run();
    }

    void Register(::google::protobuf::RpcController* controller,
                       const ::pb::RegisterRequest* request,
                       ::pb::RegisterResponse* response,
                       ::google::protobuf::Closure* done) {
        uint32_t id = request->id();
        std::string name = request->name();
        std::string pwd = request->pwd();

        bool register_success = Register(id, name, pwd);
        response->set_success(register_success);
        pb::ResultCode* result = response->mutable_result();
        if (register_success) {
            result->set_error_code(0);
            result->set_error_message("");
        } else {
            result->set_error_code(1);
            result->set_error_message("register error!");
        }

        done->Run();
    }
};

int main(int argc, char** argv) {
    // framework init
    MprpcApplication::Init(argc, argv);

    // distribute the Uservice object to the rpc node
    RpcProvider provider;
    provider.NotifyService(new UserService());

    provider.Run();

    return 0;
}