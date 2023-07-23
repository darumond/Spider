#include "session.hh"

template <>
void Sessions<ssl_socket>::setVhost(const std::shared_ptr<VHost>& vhost)
{
    vhost_ = std::shared_ptr<VHost>(vhost);
}

static int sni_callback(SSL* ssl, int*, void* arg)
{
    auto* session = static_cast<Sessions<ssl_socket>*>(arg);
    try
    {
        std::string hostname =
            SSL_get_servername(ssl, TLSEXT_NAMETYPE_host_name);
        if (hostname.empty())
        {
            BOOST_LOG_TRIVIAL(error)
                << *session << "sni_callback(): sni empty\n";
            return SSL_TLSEXT_ERR_ALERT_FATAL;
        }
        Dispatcher dispatcher =
            Dispatcher(hostname, session->getIp(), session->getPort());
        if (dispatcher.isError())
            session->client_request_.set_status_code(
                BAD_REQUEST, false, "send_response",
                "cannot dispatch correctly");
        else if (dispatcher.need_to_dispatch())
        {
            session->setVhost(std::shared_ptr<VHost>(dispatcher.find_vhost()));
            const char* cert = session->getVhost()->getSslCert().c_str();
            const char* sslKey = session->getVhost()->getSslKey().c_str();
            SSL_use_certificate_file(ssl, cert, SSL_FILETYPE_PEM);
            SSL_use_PrivateKey_file(ssl, sslKey, SSL_FILETYPE_PEM);
            if (SSL_set_tlsext_host_name(ssl, &hostname) != 1)
                BOOST_LOG_TRIVIAL(info)
                    << *session << "sni_callback(): cannot get sni";
        }
        else
        {
            session->client_request_.set_status_code(
                BAD_REQUEST, false, "send_response",
                "cannot dispatch correctly");
        }
    }
    catch (...)
    {
        BOOST_LOG_TRIVIAL(error)
            << *session << "sni_callback(): error during handshake";
    }
    return SSL_TLSEXT_ERR_OK;
}

template <>
Sessions<ssl_socket>::Sessions(boost::asio::io_service& io_service,
                               const std::string& ip,
                               const unsigned short& port)
    : io_service(io_service)
    , ssl_context_(boost::asio::ssl::context(
          boost::asio::ssl::context_base::tlsv13_server))
    , client_socket_(std::make_shared<ssl_socket>(io_service, *ssl_context_))
    , ip_(ip)
    , port_(port)
    , proto_("https")
{}

template <>
void Sessions<ssl_socket>::handshake()
{
    auto self(shared_from_this());
    BOOST_LOG_TRIVIAL(info) << *this << " doing handshake";
    SSL_CTX_set_tlsext_servername_callback(ssl_context_->native_handle(),
                                           sni_callback);
    SSL_CTX_set_tlsext_servername_arg(ssl_context_->native_handle(), this);
    client_socket_->async_handshake(
        boost::asio::ssl::stream_base::server,
        [self](const boost::system::error_code& ec) {
            // if and only if handshake is successful, we read client request
            if (!ec)
                self->read_client_request();
            else
                BOOST_LOG_TRIVIAL(error)
                    << *self << "handshake failed! because: " << ec.message();
        });
}

template <>
void Sessions<ssl_socket>::close_connection(boost::system::error_code ec,
                                            std::size_t /*length*/)
{
    if (!ec)
    {
        BOOST_LOG_TRIVIAL(debug) << this << "closing the connection";
        boost::system::error_code close_ec;
        client_socket_->next_layer().shutdown(tcp::socket::shutdown_both,
                                              close_ec);
        client_socket_->next_layer().close();
        read_client_request();
    }
    else
    {
        BOOST_LOG_TRIVIAL(error)
            << *this << "error during the close connection";
    }
}