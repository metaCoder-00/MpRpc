#ifndef MPRPC_SRC_INCLUDE_MPRPCCONTROLLER_H_
#define MPRPC_SRC_INCLUDE_MPRPCCONTROLLER_H_

#include <string>
#include "google/protobuf/service.h"

class MprpcController : public google::protobuf::RpcController {
public:
    // implemented methods
    inline MprpcController() : failed_(false), error_text_("") {}
    inline void Reset() {
        failed_ = false;
        error_text_ = "";
    }
    inline bool Failed() const {return failed_;}
    inline std::string ErrorText() const {return error_text_;};
    inline void SetFailed(const std::string& reason) {
        failed_ = true;
        error_text_ = reason;
    };

    // just overrided
    void StartCancel() {}
    bool IsCanceled() const {return false;}
    void NotifyOnCancel(google::protobuf::Closure* callback) {}
private:
    bool failed_;
    std::string error_text_;
};

#endif // MPRPC_SRC_INCLUDE_MPRPCCONTROLLER_H_