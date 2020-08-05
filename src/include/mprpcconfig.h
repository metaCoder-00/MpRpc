#ifndef MPRPC_SRC_INCLUDE_MPRPCCONFIG_H_
#define MPRPC_SRC_INCLUDE_MPRPCCONFIG_H_

#include <string>
#include <unordered_map>

// read config file class
// key: rpcserver_ip rpcserver_port zookeeper_ip zookeeper_port
class MprpcConfig {
public:
    // analyze and load config file
    void LoadConfigFile(const char* config_file);
    // find config info.
    std::string Load(const std::string& key);
private:
    std::unordered_map<std::string, std::string> config_map_;
};

#endif // MPRPC_SRC_INCLUDE_MPRPCCONFIG_H_