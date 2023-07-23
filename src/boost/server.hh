#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
#include <fstream>
#include <iostream>
#include <openssl/ssl.h>
#include <utility>
#include <variant>

#include "config.hh"
#include "json.hpp"
#include "session.hh"
#include "vhost/vhost.hh"

using standard_socket = boost::asio::ip::tcp::socket;
using ssl_socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;

class Server
{
public:
    Server(boost::asio::io_service& io_service, unsigned short port,
           const boost::asio::ip::address& ip,
           const std::shared_ptr<VHost>& vhost);

    template <typename SocketType>
    void wait_client_and_accept();
    std::string server_name_;

    void display_connection();

    inline friend std::ostream& operator<<(std::ostream& output,
                                           const Server& server)
    {
        output << "Server (" << (server.vhost_->isCrypted() ? "https" : "http")
               << "): "
               << "Ip: " << server.ip_ << ", Port: " << server.port_ << " ";
        return output;
    }

private:
    boost::asio::io_service& io_service_;
    const boost::asio::ip::address ip_;
    unsigned short port_;
    tcp::acceptor listener_functor_;
    std::shared_ptr<VHost> vhost_;
};