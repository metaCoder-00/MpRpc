#include "zookeeperutil.h"
#include <semaphore.h>
#include <iostream>
#include "mprpcapplication.h"
#include "logger.h"

/*
    @param type: callback message type
    @param state: connection state
*/
static void global_watcher(zhandle_t *zh, int type, 
                           int state, const char *path,
                           void *watcherCtx) {
    if (type == ZOO_SESSION_EVENT && state == ZOO_CONNECTED_STATE) {
        // sem_t* sem_ptr = reinterpret_cast<sem_t*>(zoo_get_context(zh));
        sem_t* sem_ptr = (sem_t*)zoo_get_context(zh);
        sem_post(sem_ptr);
    }
}

ZkClient::ZkClient() : zk_handle_(nullptr) {

}

ZkClient::~ZkClient() {
    if (zk_handle_) {
        zookeeper_close(zk_handle_);    // close handle 
    }
}

// connection
void ZkClient::Start() {
    std::string host = MprpcApplication::GetInstance().GetConfig().Load("zookeeper_ip");
    std::string port = MprpcApplication::GetInstance().GetConfig().Load("zookeeper_port");
    std::string connect_str = host + ":" + port;

    /* 
        zookeeper_mt: multi-thread version
        zookeeper API created three thread:
        1. API call thread
        2. network I/O thread (poll)
        3. watcher callback thread
    */
    zk_handle_ = zookeeper_init(connect_str.c_str(), global_watcher, 30000, nullptr, nullptr, 0);
    if (nullptr == zk_handle_) {
        LOG_ERROR("zookeeper_init error!");
        exit(EXIT_FAILURE);
    }

    // handle create success!
    sem_t sem;
    sem_init(&sem, 0, 0);
    zoo_set_context(zk_handle_, &sem);

    sem_wait(&sem);
    // DEGBUG info
    LOG_INFO("zookeeper_init success!");
}

void ZkClient::Create(const char* path, const char* data, int data_len, int state) {
    char path_buf[128];
    int buf_len = sizeof(path_buf);
    int flag = zoo_exists(zk_handle_, path, 0, nullptr);
    // is znode in the path does existed?
    if (ZNONODE == flag) {
        flag = zoo_create(zk_handle_, path, data, data_len, &ZOO_OPEN_ACL_UNSAFE, state, path_buf, buf_len);
        if (ZOK == flag) {
            LOG_INFO("znode create success... path: %s", path);
        } else {
            LOG_ERROR("znode create error... path: %s, flag: %d", path, flag);
            exit(EXIT_FAILURE);
        }
    }
}

std::string ZkClient::GetData(const char* path) {
    char buf[64] = {0};
    int buf_len = sizeof(buf);
    int flag = zoo_get(zk_handle_, path, 0, buf, &buf_len, nullptr);
    if (ZOK != flag) {
        LOG_ERROR("get znode error... path: %s, flag: %d", path, flag);
        return "";
    } else {
        return buf;
    }
}