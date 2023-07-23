#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <optional>
#include <utility>

#include "proxy_pass.hh"
#include "upstream.hh"

class VHost
{
public:
    VHost() = delete;
    VHost(boost::asio::ip::address ip, int port, std::string serverName,
          std::string root, bool cudFiles, std::string defaultFile,
          bool is_default_vhost, std::optional<Proxy_pass> proxyPass);

    VHost(boost::asio::ip::address ip, int port, std::string serverName,
          std::string root, bool cudFiles, std::string defaultFile,
          bool is_default_vhost, std::string ssl_cert, std::string ssl_key,
          std::shared_ptr<boost::asio::ssl::context>& ssl_context_,
          std::optional<Proxy_pass> proxyPass);

    VHost(boost::asio::ip::address ip, int port, std::string serverName,
          std::string root, bool cudFiles, std::string defaultFile,
          bool is_default_vhost, std::string ssl_cert, std::string ssl_key,
          std::optional<Proxy_pass> proxyPass);

    // =============== Logger ===============
    void print_vhost();
    void display_connection();

    // =============== Getter ===============
    const boost::asio::ip::address& getIp() const;
    const std::string& getServerName() const;
    const std::string& getRoot() const;
    bool isCudFiles() const;
    const std::string& getDefaultFile() const;
    unsigned short getPort() const;

    bool isDefaultVhost() const;
    const std::string& getSslCert() const;
    const std::string& getSslKey() const;
    bool isCrypted() const;

    // proxy_pass

private:
    // ========= Step 1: =========

    // mandatory
    boost::asio::ip::address ip_;
    unsigned short port_{};
    std::string server_name_;
    std::string root_;
    // optional
    bool cud_files_ = false;
    std::string default_file_ = std::string("index.html");

    // ========= Step 2: =========

    // optional
    bool is_default_vhost_{};
    // mandatory if one of them is present
    std::string ssl_cert_;
    std::string ssl_key_;

    // ========= Step 3: =========
    bool is_crypted_;
    std::optional<Proxy_pass> proxy_pass_;

public:
    const std::optional<Proxy_pass>& getProxyPass() const;

public:
    std::shared_ptr<boost::asio::ssl::context> ssl_context_;
};

#include "vhost.hxx"