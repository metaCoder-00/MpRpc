#ifndef MPRPC_SRC_INCLUDE_ZOOKEEPERUTIL_H_
#define MPRPC_SRC_INCLUDE_ZOOKEEPERUTIL_H_

#include <string>
#include "zookeeper/zookeeper.h"

// zk clinet class
class ZkClient {
public:
    ZkClient();
    ~ZkClient();
    // start a connection
    void Start();
    // create a znode by path
    void Create(const char* path, const char* data, int data_len, int state=0);
    // get znode value by path
    std::string GetData(const char* path);
private:
    // zk client handle
    zhandle_t* zk_handle_;
};

#endif // MPRPC_SRC_INCLUDE_ZOOKEEPERUTIL_H_