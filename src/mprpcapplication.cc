#include "mprpcapplication.h"
#include <unistd.h>
#include <iostream>
#include <string>

MprpcConfig MprpcApplication::config_;

static void ShowArgsHelp() {
    std::cout << "format: command -i <configfile>" << std::endl;
}

void MprpcApplication::Init(int argc, char **argv) {
    if (argc < 2) {
        ShowArgsHelp();
        exit(EXIT_FAILURE);
    }

    int opt = 0;
    std::string config_file;
    while ((opt = getopt(argc, argv, "i:")) != -1) {
        switch (opt) {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        case ':':
            ShowArgsHelp();
            exit(EXIT_FAILURE);
        default:
            break;
        }
    }

    // load config file 
    // rpc server ip, port, zookeeper ip, port
    config_.LoadConfigFile(config_file.c_str());

    // DEBUG codes
    // std::cout << "rpcserver ip: " << m_config.Load("rpcserver_ip") << std::endl;
    // std::cout << "rpcserver port: " << m_config.Load("rpcserver_port") << std::endl;
    // std::cout << "zookeeper ip: " << m_config.Load("zookeeper_ip") << std::endl;
    // std::cout << "zookeeper port: " << m_config.Load("zookeeper_port") << std::endl;
}

MprpcApplication& MprpcApplication::GetInstance() {
    static MprpcApplication app;
    return app;
}

MprpcConfig& MprpcApplication::GetConfig() {
    return config_;
}