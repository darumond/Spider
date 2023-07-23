#include "server.hh"

#include <boost/log/trivial.hpp>

// ====================== SSL SOCKET ======================
template <>
void Server::wait_client_and_accept<ssl_socket>()
{
    BOOST_LOG_TRIVIAL(info)
        << "Server(https): "
        << "Ip: " << vhost_->getIp() << ", Port: " << vhost_->getPort()
        << " waiting for a client..";
    // wait for a client socket
    auto session = std::make_shared<Sessions<ssl_socket>>(
        io_service_, ip_.to_string(), port_);
    listener_functor_.listen();
    listener_functor_.async_accept(
        session->getClientSocket()->next_layer(),
        [session, this](boost::system::error_code ec) {
            if (!ec)
            {
                // Start a session for a client
                BOOST_LOG_TRIVIAL(info) << *this << "has received a client";
                session->client_ip_ = session->getClientSocket()
                                          ->next_layer()
                                          .remote_endpoint()
                                          .address()
                                          .to_string();
                session->handshake();
                wait_client_and_accept<ssl_socket>();
            }
            else
                BOOST_LOG_TRIVIAL(error) << ec.message();
        });
}

// ====================== STANDARD SOCKET ======================
template <>
void Server::wait_client_and_accept<standard_socket>()
{
    BOOST_LOG_TRIVIAL(info)
        << "Server(http): "
        << "Ip: " << vhost_->getIp() << ", Port: " << vhost_->getPort()
        << " waiting for a client..";

    auto session = std::make_shared<Sessions<standard_socket>>(
        io_service_, ip_.to_string(), port_);
    listener_functor_.listen();
    listener_functor_.async_accept(
        *session->getClientSocket(),
        [session, this](boost::system::error_code ec) {
            if (!ec)
            {
                // Start a session for a client
                BOOST_LOG_TRIVIAL(info) << *this << "has received a client";
                session->client_ip_ = session->getClientSocket()
                                          ->remote_endpoint()
                                          .address()
                                          .to_string();
                session->read_client_request();
                wait_client_and_accept<standard_socket>();
            }
            else
                BOOST_LOG_TRIVIAL(error) << ec.message();
        });
}

// ====================== Constructor ======================
Server::Server(boost::asio::io_service& io_service, unsigned short port,
               const boost::asio::ip::address& ip,
               const std::shared_ptr<VHost>& vhost)

    : io_service_(io_service)
    , ip_(ip)
    , port_(port)
    , listener_functor_(io_service)
    , vhost_(std::shared_ptr<VHost>(vhost))
{
    boost::asio::ip::tcp::endpoint endpoint(ip, port);
    listener_functor_.open(endpoint.protocol());
    int one = 1;
    setsockopt(listener_functor_.native_handle(), SOL_SOCKET,
               SO_REUSEADDR | SO_REUSEPORT, &one, sizeof(one));
    listener_functor_.bind(endpoint);

    BOOST_LOG_TRIVIAL(info)
        << "Server(https): "
        << "Ip: " << vhost_->getIp() << ", Port: " << vhost_->getPort()
        << " has been created";

    if (vhost->isCrypted())
        wait_client_and_accept<ssl_socket>();
    else
        wait_client_and_accept<standard_socket>();
}