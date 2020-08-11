#include "mprpcconfig.h"
#include <iostream>
#include <string>

static void Trim(std::string& str) {
    // erase head space
    size_t idx = str.find_first_not_of(' ');
    if (std::string::npos != idx) {
        str = str.substr(idx, str.length() - idx);
    }
    // erase tail space
    idx = str.find_last_not_of(' ');
    if (std::string::npos != idx) {
        str = str.substr(0, idx + 1);
    }
}

// analyze and load config file
void MprpcConfig::LoadConfigFile(const char* config_file) {
    FILE* fp = fopen(config_file, "r");
    if (nullptr == fp) {
        std::cout << config_file << " not exist!" << std::endl;
        exit(EXIT_FAILURE);
    }
    
    // comments, correct config term, erase space
    while (!feof(fp)) {
        char buf[512] = {0};
        fgets(buf, 512, fp);

        // erase head space
        std::string str_buf(buf);
        Trim(str_buf);

        // comments #
        if (str_buf[0] == '#' || str_buf.empty()) {
            continue;
        }

        // config terms
        size_t idx = str_buf.find('=');
        if (std::string::npos == idx) {
            // illegal
            continue;
        }
        std::string key, value;
        key = str_buf.substr(0, idx);
        value = str_buf.substr(idx + 1, str_buf.length() - idx - 1);
        Trim(key);
        // 127.0.0.1\n
        idx = value.find_last_of('\n');
        if (std::string::npos != idx) {
            value.erase(idx);
        }
        Trim(value);
        config_map_[key] = value;
    }
}

// find config info.
std::string MprpcConfig::Load(const std::string& key) {
    if (!config_map_.count(key)) {
        return "";
    }
    
    return config_map_[key];
}