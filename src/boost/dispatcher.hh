#pragma once

#include <string>

#include "config/config.hh"
#include "utils/utils.hh"
#include "vhost/vhost.hh"

class Dispatcher
{
public:
    // ======================== Constructor ========================
    explicit Dispatcher(const std::string& host_header, const std::string& ip,
                        const unsigned short& port);

    // ======================== Handle Dispatch ========================
    bool need_to_dispatch();
    bool isError() const;
    std::shared_ptr<VHost> find_vhost();
    const std::string& getServerName() const;
    std::string server_name_;

private:
    std::string ip_;
    int port_;

private:
    std::shared_ptr<VHost> vhost_ = nullptr;
    bool error_ = false;
};
