#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <optional>
#include <utility>

class Upstream
{
public:
    Upstream();

    void setMethod(const std::string& method);
    void setHost(const std::unordered_map<std::string, std::string>& host);
    void setHosts(
        const std::vector<std::unordered_map<std::string, std::string>>& hosts);
    const std::string& getMethod() const;
    const std::unordered_map<std::string, std::string>& getHost() const;
    const std::vector<std::unordered_map<std::string, std::string>>&
    getHosts() const;

    std::string method_;
    std::unordered_map<std::string, std::string> host_;
    std::vector<std::unordered_map<std::string, std::string>> hosts_;
};