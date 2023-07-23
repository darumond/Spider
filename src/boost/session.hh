#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind/bind.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <boost/log/trivial.hpp>
#include <boost/shared_ptr.hpp>
#include <fstream>
#include <iostream>
#include <optional>
#include <utility>
#include <variant>

#include "dispatcher.hh"
#include "json.hpp"
#include "request.hh"
#include "response.hh"
#include "vhost/vhost.hh"

using boost::asio::ip::tcp;
using standard_socket = boost::asio::ip::tcp::socket;
using ssl_socket = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
using std::optional;
template <class socket_type>
class Sessions : public std::enable_shared_from_this<Sessions<socket_type>>
{
public:
    // ============= Constructor =============

    Sessions(boost::asio::io_service& io_service, const std::string& ip_,
             const unsigned short& port_);

    // ============= Client Handle ============

    void read_client_request();
    void read_client_request_body();
    void handle_read(const boost::system::error_code ec, size_t length);
    void handle_read_body(const boost::system::error_code ec, size_t length);
    void send_response();
    void close_connection(boost::system::error_code ec, std::size_t /*length*/);
    std::string get_file_contents(const std::string& file_path);
    void init_response();
    void handshake();
    void setVhost(const std::shared_ptr<VHost>& vhost);
    const std::shared_ptr<VHost>& getVhost() const;
    boost::asio::io_service& io_service;

    std::optional<boost::asio::ssl::context>& getSslContext();
    std::shared_ptr<socket_type>& getClientSocket();
    std::optional<boost::asio::ssl::context> ssl_context_;
    std::optional<boost::asio::ssl::context> ssl_context2_;
    std::shared_ptr<socket_type> client_socket_;
    void handle_connection(boost::system::error_code ec,
                           std::size_t /*length*/);

    void async_write();

    friend std::ostream& operator<<(std::ostream& output,
                                    Sessions<socket_type>& session)
    {
        output << "Sessions "
               << "(" << session.proto_ << ") "
               << "Ip: " << session.ip_ << ", Port: " << session.port_ << " ";
        return output;
    };

    std::string content_to_send_;

private:
    // ============= Client Socket =============
    request::Response client_response_;
    std::string request_body_;
    size_t content_length_ = 0;
    std::vector<char> buffer_;
    std::string ip_;
    unsigned short port_;

public:
    std::string proto_;
    std::string request_;
    request::Request client_request_;
    const std::string& getIp() const;
    unsigned short getPort() const;
    std::string client_ip_;

private:
    // ============= Server Vhost =============
    std::shared_ptr<VHost> vhost_;
    bool handle_dispatch(const std::string& host_header);
    std::string getHostHeader(const std::string& request);
    void handle_request(int body_size);
};

template <>
void Sessions<ssl_socket>::setVhost(const std::shared_ptr<VHost>& vhost);

template <>
Sessions<ssl_socket>::Sessions(boost::asio::io_service& io_service,
                               const std::string& ip_,
                               const unsigned short& port_);

template <>
void Sessions<ssl_socket>::handshake();

template <>
void Sessions<ssl_socket>::close_connection(boost::system::error_code ec,
                                            std::size_t /*length*/);

class ReverseProxy : public std::enable_shared_from_this<ReverseProxy>
{
public:
    ReverseProxy(const std::shared_ptr<VHost>& vhost,
                 boost::asio::io_service& io_service,
                 const std::string& request, const std::string& request_body,
                 const std::string& original_host,
                 std::shared_ptr<Sessions<standard_socket>> session);

    ReverseProxy(const std::shared_ptr<VHost>& vhost,
                 boost::asio::io_service& io_service,
                 const std::string& request, const std::string& request_body,
                 const std::string& original_host,
                 std::shared_ptr<Sessions<ssl_socket>> session);

    std::string
    recreate(const std::string& original_string,
             const optional<std::unordered_map<std::string, std::string>>&
                 proxy_set_header,
             const optional<std::vector<std::string>>& proxy_remove_header);

    std::string recreateRequest(
        std::unordered_map<std::string, std::string> proxy_set_header,
        const std::vector<std::string>& proxy_remove_header);

    void sendRequest(const std::string& content_to_send_);

    void closeConnection(boost::system::error_code ec, std::size_t /*length*/);

    void handle_read_response_headers(boost::system::error_code ec,
                                      std::size_t /*length*/);

    void handle_read_response_content(boost::system::error_code ec,
                                      std::size_t /*length*/,
                                      long content_length);

    void recreateResponse();
    std::string getContentLength(const std::string& request);

    std::string response_header_;
    std::string response_content_;

    void start_reversing();

    friend std::ostream& operator<<(std::ostream& output,
                                    ReverseProxy& /*reverse_proxy*/)
    {
        output << "Reverse_proxy  ";
        return output;
    };

private:
    std::shared_ptr<VHost> vhost_;
    std::string request_;
    const std::string request_body_;
    boost::asio::ip::tcp::socket back_socket_;
    std::vector<char> buffer_;
    boost::asio::streambuf response_;
    std::variant<std::shared_ptr<Sessions<standard_socket>>,
                 std::shared_ptr<Sessions<ssl_socket>>>
        session_;
    std::string proto_;
    std::string client_ip_;
    std::string original_host_;
    bool is_head_ = false;
};

#include "session.hxx"