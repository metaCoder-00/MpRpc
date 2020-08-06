#include <iostream>
#include <string>
#include <vector>
#include "friend.pb.h"
#include "mprpcapplication.h"
#include "rpcprovider.h"
#include "logger.h"

class FriendService : public pb::FriendServiceRpc {
private:
    std::vector<std::string> GetFriendsList(uint32_t user_id) {
        std::cout << "local service: GetFriendsList" << std::endl;
        std::cout << "user id: " << user_id << std::endl;
        std::vector<std::string> friends_list = {"trump", "hilary"};
        return friends_list;
    }
public:
    void GetFriendsList(::google::protobuf::RpcController* controller,
                        const ::pb::GetFriendsListRequest* request,
                        ::pb::GetFriendsListResponse* response,
                        ::google::protobuf::Closure* done) {
        uint32_t user_id = request->user_id();

        std::vector<std::string> friends_list = GetFriendsList(user_id);
        response->mutable_result()->set_error_code(0);
        response->mutable_result()->set_error_message("");
        for (std::string& name : friends_list) {
            std::string* str_ptr = response->add_friends();
            *str_ptr = name;
        }

        done->Run();
    }
};

int main(int argc, char** argv) {
    // framework init
    MprpcApplication::Init(argc, argv);

    // distribute the Uservice object to the rpc node
    RpcProvider provider;
    provider.NotifyService(new FriendService());

    provider.Run();

    return 0;
}