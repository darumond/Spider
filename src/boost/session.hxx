#pragma once
#include "server.hh"
#include "session.hh"

template <typename socket_type>
const std::shared_ptr<VHost>& Sessions<socket_type>::getVhost() const
{
    return vhost_;
}

template <typename socket_type>
void Sessions<socket_type>::setVhost(const std::shared_ptr<VHost>& vhost)
{
    vhost_ = std::shared_ptr<VHost>(vhost);
}

template <typename socket_type>
void Sessions<socket_type>::read_client_request()
{
    BOOST_LOG_TRIVIAL(info) << *this << " reading client request";
    auto self = this->shared_from_this();
    boost::asio::async_read_until(
        *client_socket_, boost::asio::dynamic_buffer(buffer_), CRLF CRLF,
        boost::bind(&Sessions::handle_read, self,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred()));
}

template <typename socket_type>
void Sessions<socket_type>::read_client_request_body()
{
    auto self = this->shared_from_this();
    boost::asio::async_read(
        *client_socket_, boost::asio::dynamic_buffer(buffer_),
        boost::asio::transfer_at_least(content_length_),
        boost::bind(&Sessions::handle_read_body, self,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred()));
}
template <typename socket_type>
bool Sessions<socket_type>::handle_dispatch(const std::string& host_header)
{
    Dispatcher dispatcher = Dispatcher(host_header, ip_, port_);
    if (vhost_ != nullptr && vhost_->isCrypted())
        dispatcher.server_name_ = vhost_->getServerName();
    if (dispatcher.isError())
        return false;
    else if (dispatcher.need_to_dispatch())
    {
        auto vhost = std::shared_ptr<VHost>(dispatcher.find_vhost());
        if (vhost_ != nullptr)
        {
            if (vhost_->getServerName() != vhost->getServerName())
            {
                BOOST_LOG_TRIVIAL(debug)
                    << *this
                    << "Actual host does not correspond to previous one";
                return false;
            }
        }
        else
        {
            setVhost(vhost);
        }
    }
    else
        return false;
    return true;
}

template <typename socket_type>
std::string Sessions<socket_type>::getHostHeader(const std::string& request)
{
    std::string hostHeader;

    // Split the request by lines
    std::size_t startPos = 0;
    std::size_t endPos = request.find("\r\n");
    while (endPos != std::string::npos)
    {
        // Get the current line
        std::string line = request.substr(startPos, endPos - startPos);

        // Find the delimiter between header name and value
        std::size_t delimiterPos = line.find(": ");
        if (delimiterPos != std::string::npos)
        {
            // Extract the header name and value
            std::string headerName = line.substr(0, delimiterPos);
            std::string headerValue =
                line.substr(delimiterPos + 2); // Skip the delimiter

            // Check if the header is Host
            to_lower_case(headerName);
            if (headerName == "host")
            {
                // Store the Host header value
                hostHeader = headerValue;
                break;
            }
        }

        // Move to the next line
        startPos = endPos + 2; // Skip \r\n
        endPos = request.find("\r\n", startPos);
    }

    return hostHeader;
}

template <typename socket_type>
void Sessions<socket_type>::handle_request(int body_size)
{
    BOOST_LOG_TRIVIAL(debug) << *this << " handling normal request";
    content_length_ = client_request_.getContentLength() - body_size;
    if (client_request_.getMethod() == PUT && content_length_ > 0)
        read_client_request_body();
    else
        send_response();
}

template <typename socket_type>
void Sessions<socket_type>::handle_read(const boost::system::error_code ec,
                                        size_t length)
{
    if (!ec)
    {
        BOOST_LOG_TRIVIAL(debug) << *this << "handling the request";
        if (length != 0)
        {
            std::string client_request(buffer_.data(), buffer_.size());
            request_ = client_request;
            std::size_t delimiter_pos = client_request.find(CRLF CRLF);
            std::string body_data = client_request.substr(delimiter_pos + 4);
            request_body_ += body_data;
            std::cout << *this << request_;
            std::string host_header = getHostHeader(request_);
            if (host_header.empty())
            {
                BOOST_LOG_TRIVIAL(error) << *this << " host header is empty";
                client_request_.set_status_code(BAD_REQUEST, false,
                                                "send_response",
                                                "cannot dispatch correctly");
                return send_response();
            }
            else
            {
                bool good_dispatch = handle_dispatch(host_header);
                if (!good_dispatch)
                {
                    BOOST_LOG_TRIVIAL(error)
                        << *this << " error during the dispatching";
                    client_request_.set_status_code(
                        BAD_REQUEST, false, "send_response",
                        "cannot dispatch correctly");
                    return send_response();
                }
                if (!vhost_->getProxyPass().has_value())
                {
                    if (!vhost_->getProxyPass().has_value())
                        client_request_ = request::Request(request_);
                    if (client_request_.isErrorPreDispatch())
                        return send_response();
                    client_request_.check_after_dispatch(vhost_);
                }
                if (!vhost_->getProxyPass().has_value())
                    return handle_request(body_data.size());
                else
                {
                    BOOST_LOG_TRIVIAL(debug)
                        << *this << " handling vhost proxy passs";
                    auto self = this->shared_from_this();
                    auto reverse_proxy = std::make_shared<ReverseProxy>(
                        vhost_, io_service, request_, request_body_,
                        host_header, self);
                    reverse_proxy->start_reversing();
                }
            }
        }
    }
    else
    {
        BOOST_LOG_TRIVIAL(error)
            << *this << " error during reading the client request";
    }
}

template <typename socket_type>
void Sessions<socket_type>::handle_read_body(const boost::system::error_code ec,
                                             size_t length)
{
    if (!ec)
    {
        BOOST_LOG_TRIVIAL(debug) << *this << "handling the request body";
        if (content_length_ <= length)
            content_length_ = 0;
        else
            content_length_ -= length;

        std::string received_data(buffer_.data(), buffer_.size());

        std::size_t delimiter_pos = received_data.find(CRLF CRLF);

        std::string body_data = received_data.substr(delimiter_pos + 4);
        request_body_ += body_data;

        std::clog << "send_response(): " << vhost_->getServerName()
                  << " has send a response to the client\n";
        return send_response();
    }
    else
    {
        BOOST_LOG_TRIVIAL(error)
            << *this << " error during reading the client request body";
    }
}
template <typename socket_type>
void Sessions<socket_type>::init_response()
{
    BOOST_LOG_TRIVIAL(debug) << *this << "init the response";
    if (!client_request_.is_status_correct())
        client_request_.keep_alive_ = false;

    // ==================== Create Response ====================ssf
    client_response_ = request::Response(
        client_request_.getStatusCode(), client_request_.getMethod(),
        client_request_.getResourcePath(), request_body_,
        client_request_.isKeepAlive());

    // ======================== PUT Method ========================
    if (client_request_.getMethod() == PUT
        && (client_request_.getStatusCode() == OK
            || client_request_.getStatusCode() == CREATED))
    {
        std::string filename = client_request_.getResourcePath();
        std::ofstream file(filename);
        if (file.is_open())
        {
            file << request_body_;
            file.close();
        }
    }
}

template <typename socket_type>
void Sessions<socket_type>::async_write()
{
    auto self = this->shared_from_this();
    BOOST_LOG_TRIVIAL(debug) << *this << "response: " << content_to_send_;
    boost::asio::async_write(
        *client_socket_,
        boost::asio::buffer(content_to_send_, content_to_send_.size()),
        boost::bind(&Sessions::handle_connection, self,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred()));
}

template <typename socket_type>
void Sessions<socket_type>::send_response()
{
    init_response();

    // ======================== GET Method ========================
    std::string body;
    if (client_request_.getMethod() == GET
        && client_request_.getStatusCode() == OK)
        body = get_file_contents(client_request_.getResourcePath());

    content_to_send_ = client_response_.response() + body;
    async_write();
}

template <typename socket_type>
void Sessions<socket_type>::handle_connection(boost::system::error_code ec,
                                              std::size_t length)
{
    if (!ec
        && (client_request_.getStatusCode() == 200
            || client_request_.getStatusCode() == 201))
    {
        BOOST_LOG_TRIVIAL(debug) << *this << "sent the response";
        if (!client_request_.isKeepAlive())
            close_connection(ec, length);
        else
        {
            buffer_.clear();
            request_ = "";
            content_to_send_ = "";
            content_length_ = 0;
            request_body_ = "";
            read_client_request();
        }
    }
    else
        close_connection(ec, length);
}

template <typename socket_type>
std::string
Sessions<socket_type>::get_file_contents(const std::string& file_path)
{
    std::ifstream file(file_path);
    if (file.is_open())
    {
        std::stringstream buffer;
        buffer << file.rdbuf();

        std::string file_content = buffer.str();
        file.close();
        return file_content;
    }
    else
    {
        BOOST_LOG_TRIVIAL(error)
            << *this << "get_file_contents(): cannot read the file "
            << file_path;
    }
    return "";
}

template <typename socket_type>
void Sessions<socket_type>::close_connection(boost::system::error_code ec,
                                             std::size_t /*length*/)
{
    if (!ec)
    {
        BOOST_LOG_TRIVIAL(debug) << *this << "closing the connection";
        boost::system::error_code close_ec;
        client_socket_->shutdown(tcp::socket::shutdown_both, close_ec);
        client_socket_->close();
        read_client_request();
    }
    else
    {
        BOOST_LOG_TRIVIAL(error)
            << *this << "error during the close connection";
    }
}

template <typename socket_type>
Sessions<socket_type>::Sessions(boost::asio::io_service& io_service,
                                const std::string& ip,
                                const unsigned short& port)
    : io_service(io_service)
    , client_socket_(std::make_shared<socket_type>(io_service))
    , ip_(ip)
    , port_(port)
    , proto_("http")
{}

template <typename socket_type>
inline std::shared_ptr<socket_type>& Sessions<socket_type>::getClientSocket()
{
    return client_socket_;
}

template <typename socket_type>
inline std::optional<boost::asio::ssl::context>&
Sessions<socket_type>::getSslContext()
{
    return ssl_context_;
}

template <class socket_type>
inline const std::string& Sessions<socket_type>::getIp() const
{
    return ip_;
}

template <class socket_type>
inline unsigned short Sessions<socket_type>::getPort() const
{
    return port_;
}
