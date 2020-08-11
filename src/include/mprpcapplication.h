#ifndef MPRPC_SRC_INCLUDE_MPRPCAPPLICATION_H_
#define MPRPC_SRC_INCLUDE_MPRPCAPPLICATION_H_

#include "mprpcconfig.h"
#include "mprpcchannel.h"
#include "mprpccontroller.h"

// mprpc init class
class MprpcApplication {
public:
    static void Init(int argc, char **argv);
    static MprpcApplication& GetInstance();
    static MprpcConfig& GetConfig();
private:
    static MprpcConfig config_;

    MprpcApplication() {}
    MprpcApplication(const MprpcApplication&) = delete;
    MprpcApplication(MprpcApplication&&) = delete;
};

#endif // MPRPC_SRC_INCLUDE_MPRPCAPPLICATION_H_