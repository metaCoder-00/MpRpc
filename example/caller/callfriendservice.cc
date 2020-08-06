#include <iostream>
#include "friend.pb.h"
#include "mprpcapplication.h"

int main(int argc, char** argv) {
    MprpcApplication::Init(argc, argv);     // Init framwork

    // call rpc method Login
    pb::FriendServiceRpc_Stub stub(new MprpcChannel());
    pb::GetFriendsListRequest request;
    request.set_user_id(12345);
    pb::GetFriendsListResponse response;
    MprpcController controller;
    stub.GetFriendsList(&controller, &request, &response, nullptr);     // RpcChannel->RpcChannel::CallMethod

    // read response
    if (controller.Failed()) {
        std::cout << controller.ErrorText() << std::endl;
    } else {
        if (!response.result().error_code()) {
            std::cout << "rpc GetFriendsListResponse response success!" << std::endl;
            int size = response.friends_size();
            for (int i = 0; i < size; ++i) {
                std::cout << "index: " << i + 1 << " name:" << response.friends(i) << std::endl;
            }
        } else {
            std::cout << "rpc GetFriendsListResponse response error: " << response.result().error_message() << std::endl;
        }
    }

    return 0;
}