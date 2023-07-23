#include <regex>

#include "session.hh"
ReverseProxy::ReverseProxy(const std::shared_ptr<VHost>& vhost,
                           boost::asio::io_service& io_service,
                           const std::string& request,
                           const std::string& request_body,
                           const std::string& original_host,
                           std::shared_ptr<Sessions<standard_socket>> session)
    : vhost_(vhost)
    , request_(request)
    , request_body_(request_body)
    , back_socket_(io_service)
    , session_(session)
    , proto_(session->proto_)
    , client_ip_(session->client_ip_)
    , original_host_(original_host){};

ReverseProxy::ReverseProxy(const std::shared_ptr<VHost>& vhost,
                           boost::asio::io_service& io_service,
                           const std::string& request,
                           const std::string& request_body,
                           const std::string& original_host,
                           std::shared_ptr<Sessions<ssl_socket>> session)
    : vhost_(vhost)
    , request_(request)
    , request_body_(request_body)
    , back_socket_(io_service)
    , session_(session)
    , proto_(session->proto_)
    , client_ip_(session->client_ip_)
    , original_host_(original_host){};

std::string nodeModifier(const std::string& input)
{
    auto res_input = '"' + input + '"';
    std::regex ipv6Regex(R"(\b([0-9A-Fa-f]{1,4}(:[0-9A-Fa-f]{1,4}){7})\b)");
    std::smatch match;

    if (std::regex_search(res_input, match, ipv6Regex))
    {
        // IPv6 address found in the input string
        std::string ipAddress = match[0].str();
        std::string enclosedAddress = "[" + ipAddress + "]";

        // Replace the IPv6 address in the input string with the enclosed
        // address
        std::string result =
            std::regex_replace(res_input, ipv6Regex, enclosedAddress);
        return result;
    }
    else
    {
        // No IPv6 address found, return the input as is
        return res_input;
    }
}

std::string ReverseProxy::getContentLength(const std::string& request)
{
    std::string ContentLength;

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
            if (headerName == "content-length")
            {
                // Store the Host header value
                ContentLength = headerValue;
                break;
            }
        }

        // Move to the next line
        startPos = endPos + 2; // Skip \r\n
        endPos = request.find("\r\n", startPos);
    }

    return ContentLength;
}

void ReverseProxy::start_reversing()
{
    BOOST_LOG_TRIVIAL(info) << *this << " start_reversing";

    try
    {
        boost::asio::ip::tcp::endpoint endpoint(
            (vhost_->getProxyPass()->ip_proxy_),
            vhost_->getProxyPass()->port_proxy_.value());

        back_socket_.connect(endpoint);
        BOOST_LOG_TRIVIAL(info) << *this << " connected";
        const std::string& content_to_send = recreateRequest(
            vhost_->getProxyPass()->proxy_set_header_.value(),
            vhost_->getProxyPass()->proxy_remove_header_.value());
        BOOST_LOG_TRIVIAL(info)
            << *this << " request recreated: " << content_to_send;

        sendRequest(content_to_send);
    }
    catch (...)
    {
        try
        {
            auto s =
                std::get<std::shared_ptr<Sessions<standard_socket>>>(session_);
            s->client_request_.set_status_code(
                BAD_REQUEST, false, "send_response", "the host is unreachable");
            return s->send_response();
        }
        catch (std::bad_variant_access const& ex)
        {
            auto s = std::get<std::shared_ptr<Sessions<ssl_socket>>>(session_);
            s->client_request_.set_status_code(
                BAD_REQUEST, false, "send_response", "the host is unreachable");
            return s->send_response();
        }
    }
}

std::string ReverseProxy::recreate(
    const std::string& original_string,
    const optional<std::unordered_map<std::string, std::string>>&
        proxy_set_header,
    const optional<std::vector<std::string>>& proxy_remove_header)
{
    if (original_string.find("HEAD") != std::string::npos)
        this->is_head_ = true;
    // Split the request into lines
    BOOST_LOG_TRIVIAL(debug) << *this << original_string;
    std::size_t pos = original_string.find("\r\n\r\n");
    if (pos == std::string::npos)
    {
        return original_string;
    }
    auto headers = original_string.substr(0, pos);
    std::regex regex("\r\n");
    std::vector<std::string> lines(
        std::sregex_token_iterator(headers.begin(), headers.end(), regex, -1),
        std::sregex_token_iterator());

    // Convert the existing headers into a map
    std::unordered_map<std::string, std::string> existingHeaders;
    for (auto it = lines.begin() + 1; it != lines.end(); ++it)
    {
        // Split the line into key and value
        size_t colonPos = it->find(':');
        if (colonPos != std::string::npos)
        {
            std::string key = it->substr(0, colonPos);
            std::string value = it->substr(colonPos + 1);

            // Remove leading whitespace from the value
            value.erase(0, value.find_first_not_of(" \t"));

            existingHeaders[key] = value;
        }
    }

    // Remove headers specified in proxy_remove_header
    if (proxy_remove_header.has_value())
    {
        for (auto it = existingHeaders.begin(); it != existingHeaders.end();)
        {
            std::string key = it->first;
            if (std::find(proxy_remove_header.value().begin(),
                          proxy_remove_header.value().end(), key)
                != proxy_remove_header.value().end())
            {
                it = existingHeaders.erase(it);
                continue;
            }

            ++it;
        }
    }

    if (proxy_set_header.has_value())
    {
        for (const auto& [key, value] : proxy_set_header.value())
        {
            auto existingHeaderIt = existingHeaders.find(key);
            if (existingHeaderIt != existingHeaders.end())
            {
                existingHeaderIt->second = value;
            }
            else
            {
                existingHeaders.insert(std::make_pair(key, value));
            }
        }
    }

    std::string recreatedString;
    recreatedString.append(lines[0]).append("\r\n");
    for (const auto& [key, value] : existingHeaders)
    {
        recreatedString.append(key).append(": ").append(value).append("\r\n");
    }
    return recreatedString;
}

void addTextToForwardedHeader(std::string& headers,
                              const std::string& textToAdd)
{
    std::string forwardedHeaderStart = "Forwarded:";
    size_t forwardedHeaderPos = headers.find(forwardedHeaderStart);
    if (forwardedHeaderPos != std::string::npos)
    {
        forwardedHeaderPos += forwardedHeaderStart.length();

        size_t lineStartPos = headers.find("for=", forwardedHeaderPos);
        if (lineStartPos != std::string::npos)
        {
            size_t lineEndPos = headers.find('\r', lineStartPos);
            if (lineEndPos == std::string::npos)
            {
                lineEndPos = headers.length();
            }
            headers.insert(lineEndPos, textToAdd);
        }
    }
}

std::string ReverseProxy::recreateRequest(
    std::unordered_map<std::string, std::string> proxy_set_header,
    const std::vector<std::string>& proxy_remove_header)
{
    if (proxy_set_header.find("Host") == proxy_set_header.end())
    {
        proxy_set_header.insert(std::make_pair(
            "Host",
            vhost_->getProxyPass().value().ip_proxy_.to_string().c_str()));
    }

    auto recreatedRequest =
        recreate(request_, proxy_set_header, proxy_remove_header);
    if (recreatedRequest.find("Forwarded") != std::string::npos)
    {
        std::string for_ip = ",for=" + nodeModifier(client_ip_);
        addTextToForwardedHeader(recreatedRequest, for_ip);
    }
    else
    {
        recreatedRequest += "Forwarded: for=" + nodeModifier(client_ip_)
            + ";host=" + nodeModifier(original_host_) + ";proto=" + proto_
            + "\r\n";
    }
    recreatedRequest += "\r\n";
    return recreatedRequest;
}

void ReverseProxy::sendRequest(const std::string& content_to_send_)
{
    BOOST_LOG_TRIVIAL(info) << *this << "Start sending the request";
    auto self = shared_from_this();
    boost::asio::async_write(
        back_socket_,
        boost::asio::buffer(content_to_send_, content_to_send_.size()),
        boost::bind(&ReverseProxy::handle_read_response_headers, self,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred()));
}

void ReverseProxy::handle_read_response_headers(boost::system::error_code ec,
                                                std::size_t /*length*/)
{
    if (ec)
        BOOST_LOG_TRIVIAL(error)
            << "Reverse Proxy error during reading client response headers";
    else
    {
        BOOST_LOG_TRIVIAL(info)
            << "Reverse Proxy reading client response headers";
        auto self = this->shared_from_this();
        boost::asio::async_read_until(
            back_socket_, boost::asio::dynamic_buffer(buffer_), "\r\n\r\n",
            [self](const boost::system::error_code& ec, std::size_t length) {
                if (ec || length == 0)
                {
                    BOOST_LOG_TRIVIAL(error)
                        << "Reverse Proxy error during "
                           "reading client response headers";
                    self->closeConnection(ec, length);
                    return;
                }

                std::string client_request(self->buffer_.data(),
                                           self->buffer_.size());
                self->response_header_ = self->recreate(
                    client_request,
                    self->vhost_->getProxyPass()->set_header_.value(),
                    self->vhost_->getProxyPass()->remove_header_.value());
                self->response_header_ += "\r\n";
                std::size_t delimiter_pos = client_request.find(CRLF CRLF);
                std::string body_data =
                    client_request.substr(delimiter_pos + 4);
                self->response_content_ += body_data;
                auto content_length =
                    std::stoi(self->getContentLength(self->response_header_))
                    - self->response_content_.size();
                if (content_length == 0 || self->is_head_)
                    self->closeConnection(ec, length);
                else
                    self->handle_read_response_content(ec, length,
                                                       content_length);
            });
    }
}

void ReverseProxy::handle_read_response_content(boost::system::error_code ec,
                                                std::size_t length,
                                                long content_length)
{
    BOOST_LOG_TRIVIAL(info) << "Reverse Proxy reading client response content";
    auto self = this->shared_from_this();
    if (!ec)
    {
        BOOST_LOG_TRIVIAL(info)
            << "Reverse Proxy reading client response content";
        buffer_.clear();
        boost::asio::async_read(
            back_socket_, boost::asio::dynamic_buffer(buffer_),
            boost::asio::transfer_at_least(content_length),
            [self](const boost::system::error_code& ec, std::size_t length) {
                if (ec)
                {
                    BOOST_LOG_TRIVIAL(error)
                        << "Reverse Proxy error during "
                           "reading client response content";
                    self->closeConnection(ec, length);
                }
                else
                {
                    std::string client_response_body(self->buffer_.data(),
                                                     self->buffer_.size());
                    self->response_content_ += client_response_body;
                    self->closeConnection(ec, length);
                }
            });
    }
    else if (ec != boost::asio::error::eof)
    {
        std::cout << "Reverse Proxy Error: " << ec << "\n";
    }
    else
        closeConnection(ec, length);
}

void ReverseProxy::closeConnection(boost::system::error_code ec,
                                   std::size_t /*length*/)
{
    if (!ec)
    {
        BOOST_LOG_TRIVIAL(debug) << " Reverse proxy closing the connection";
        BOOST_LOG_TRIVIAL(debug)
            << "Reverse Proxy response header " << response_header_;
        BOOST_LOG_TRIVIAL(debug)
            << "Reverse Proxy response body " << response_content_;
        boost::system::error_code close_ec;
        back_socket_.shutdown(tcp::socket::shutdown_both, close_ec);
        back_socket_.close();
        try
        {
            auto s =
                std::get<std::shared_ptr<Sessions<standard_socket>>>(session_);

            s->content_to_send_ = response_header_ + response_content_;
            s->async_write();
        }
        catch (std::bad_variant_access const& ex)
        {
            auto s = std::get<std::shared_ptr<Sessions<ssl_socket>>>(session_);
            s->content_to_send_ = response_header_ + response_content_;
            BOOST_LOG_TRIVIAL(debug)
                << " Reverse proxy content_to_send: " << s->content_to_send_;
            BOOST_LOG_TRIVIAL(debug) << " Reverse proxy sending response";
            s->async_write();
        }
    }
    // Handling bad gateway
    else
    {
        BOOST_LOG_TRIVIAL(debug) << " Reverse proxy error during closing the "
                                    "connection, sending bad gateway";
        try
        {
            auto s =
                std::get<std::shared_ptr<Sessions<standard_socket>>>(session_);

            s->client_request_.set_status_code(BAD_GATEWAY, false,
                                               "send_response", "bad gateway");
            s->send_response();
        }
        catch (std::bad_variant_access const& ex)
        {
            auto s = std::get<std::shared_ptr<Sessions<ssl_socket>>>(session_);
            s->client_request_.set_status_code(BAD_GATEWAY, false,
                                               "send_response", "bad gateway");
            s->send_response();
        }
    }
}
