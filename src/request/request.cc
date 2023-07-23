#include "request.hh"

namespace request
{
    Request::Request(std::string& request)
        : keep_alive_(true)
    {
        error_pre_dispatch = !parse_request(request);
    }

    void Request::update_keep_alive()
    {
        if (!is_status_correct())
        {
            keep_alive_ = false;
            return;
        }
        auto connection = header_.find("Connection");
        if (connection == header_.end())
            return;
        if (connection->second.find("close") != std::string::npos)
        {
            keep_alive_ = false;
        }
    }

    bool Request::parse_request(std::string& request)
    {
        size_t cur_pos = 0;
        // Method SP Request-URI SP HTTP-Version CRLF
        std::string request_line = get_next_token(request, &cur_pos, CRLF);
        if (!parse_request_line(request_line))
            return false;

        // Zero or more header (General | Request | Entity) field followed by
        // CRLF
        if (!parse_header_line(request, &cur_pos))
            return false;

        // host is in the request_uri when we have absolute form
        if (!host_.empty() && request_form_ == ABSOLUTE_FORM)
            header_["Host"] = host_;

        // Host header is mandatory
        if (!is_mandatory_headers_present_and_valid())
            return false;

        // check if empty line is present
        if (cur_pos + 2 > request.length() || request[cur_pos] != '\r'
            || request[cur_pos + 1] != '\n')
        {
            status_code_ = BAD_REQUEST;
            std::clog << "parse_request(): no \r\n after request line\n";
            return false;
        }
        update_keep_alive();
        return true;
    }

    // ========================= UTILS =========================

    bool Request::check_host_format(std::string& host)
    {
        /*
         * check if host have the following format:  uri-host [ ":" port ]
         *  uri-host == server_name or ip et port == port
         */

        // Yaka: ipv6 can be "[" ip "]"
        std::string host_processed = remove_bracket(host);
        to_lower_case(host_processed);

        auto server_name = std::string(current_vhost_->getServerName());
        to_lower_case(server_name);

        auto ip = current_vhost_->getIp().to_string();
        to_lower_case(ip);

        if (host_processed.find(':') == std::string::npos)
            return host_processed == server_name || host_processed == ip;

        auto uri_and_port = get_key_and_value(host_processed);

        auto uri = uri_and_port.first;
        to_lower_case(uri);

        return (uri == server_name || uri == ip)
            && uri_and_port.second == std::to_string(current_vhost_->getPort());
    }

    bool Request::is_mandatory_headers_present_and_valid()
    {
        /*
         * You must handle only one vhost for this step. If the Host header is
         * absent or invalid, respond with the status code 400. Keep in mind
         * that spider will have multiple vhosts afterward.
         */

        auto host_key = header_.find("Host");
        if (host_key == header_.end())
            return set_status_code(BAD_REQUEST, false,
                                   "is_mandatory_headers_present_and_valid",
                                   "host header is mandatory");

        auto content_length_key = header_.find("Content-Length");
        if (content_length_key != header_.end())
        {
            std::cout << content_length_key->second << "\n";
            std::regex digitRegex("\\d+");
            if (!std::regex_match(content_length_key->second, digitRegex))
                return set_status_code(BAD_REQUEST, false,
                                       "is_mandatory_headers_present_and_valid",
                                       "http version is not correct");

            content_length_ = std::stoul(content_length_key->second);
        }

        return true;
    }

    bool Request::check_http_version()
    {
        if (http_version_ != HTTP_VERSION)
        {
            // HTTP-version  = HTTP-name "/" DIGIT "." DIGIT, check precisely
            // the version (DIGIT)
            std::regex reg("HTTP/[0-9]{1}[.][0-9]{1}");
            if (std::regex_match(http_version_, reg))
                return set_status_code(UPGRADE_REQUIRED, false,
                                       "check_http_version",
                                       "http version is incorrect");
            // otherwise, the request is not well formatted
            else
                return set_status_code(BAD_REQUEST, false, "check_http_version",
                                       "http version is incorrect");
        }
        return true;
    }

    void Request::print_request()
    {
        std::clog
            << "===================== Request Info =====================\n";
        std::clog << "Request(print_request): \n";
        std::clog << "Status code: " << status_code_ << "\n";

        std::clog << "Method: " << method_ << "\n";
        std::clog << "Request URI: " << request_uri_ << "\n";
        std::clog << "HTTP Version: " << http_version_ << "\n";

        for (auto& head : header_)
            std::clog << head.first << ": " << head.second << "\n";

        std::clog << "Resource Path: " << resource_path_ << "\n";
        std::clog << "Content-Length: " << content_length_ << "\n";
    }

    bool Request::set_status_code(int status, bool success,
                                  const std::string& function,
                                  const std::string& error_message)
    {
        status_code_ = status;
        std::clog << function << "(): " << error_message << "\n";
        return success;
    }

    // ==================== REQUEST_LINE ====================

    bool Request::is_absolute_form(const std::string& request_uri)
    {
        size_t cur_pos = 0;
        auto protocol = get_next_token(request_uri, &cur_pos, ":");
        if (protocol != "https" && protocol != "http")
            return false;

        auto slash = get_next_token(request_uri, &cur_pos, "/");
        if (!slash.empty())
            return set_status_code(BAD_REQUEST, false, "is_absolute_form",
                                   "bad request form");

        slash = get_next_token(request_uri, &cur_pos, "/");
        if (!slash.empty())
            return set_status_code(BAD_REQUEST, false, "is_absolute_form",
                                   "bad request form");

        auto host = request_uri.substr(cur_pos, request_uri.length());
        auto host_get = get_next_token(request_uri, &cur_pos, "/");
        if (!host_get.empty())
            host = host_get;

        host_ = host;
        resource_path_ =
            "/" + request_uri.substr(cur_pos, request_uri.length());
        return true;
    }

    bool Request::is_origin_form(const std::string& request_uri)
    {
        if (request_uri[0] != '/')
            return false;

        size_t cur_pos = 0;
        auto path = get_next_token(request_uri, &cur_pos, "?");
        if (path.empty())
            resource_path_ = std::string(request_uri);
        else
            resource_path_ = path;

        return true;
    }

    bool Request::request_form(const std::string& request_uri)
    {
        std::string uri_cpy = request_uri;
        size_t i = 0;

        // we need to remove the fragment in the uri
        if (request_uri.find('#') != std::string::npos)
            uri_cpy = get_next_token(uri_cpy, &i, "#");

        if (is_absolute_form(uri_cpy))
        {
            request_form_ = ABSOLUTE_FORM;
            return true;
        }
        else if (is_origin_form(uri_cpy))
        {
            request_form_ = ORIGIN_FORM;
            return true;
        }
        return false;
    }

    void Request::check_after_dispatch(const std::shared_ptr<VHost>& vhost)
    {
        current_vhost_ = std::shared_ptr<VHost>(vhost);
        auto host_key = header_.find("Host");
        if (!current_vhost_->isCudFiles()
            && (method_ == PUT || method_ == DELETE))
            set_status_code(METHOD_NOT_ALLOWED, false, "parse_request_line",
                            "cud_file is set to false, you cannot use "
                            "PUT or DELETE method");
        if (host_key == header_.end() || !check_host_format(host_key->second))
            set_status_code(BAD_REQUEST, false,
                            "is_mandatory_headers_present_and_valid",
                            "host header is mandatory");
        is_operation_authorised(*this, resource_path_);
        if (method_ == PUT && status_code_ == OK)
        {
            // check if the resource is created
            std::cout << resource_path_ << "\n";
            if (!std::filesystem::exists(resource_path_))
                status_code_ = CREATED;
        }
        if (!is_status_correct())
            keep_alive_ = false;
        print_request();
    }

    bool Request::parse_request_line(const std::string& request_line)
    {
        size_t cur_pos = 0;
        method_ = get_next_token(request_line, &cur_pos, SP);

        // cud_file false means that we cannot modify server file
        if (!valid_method(method_))
            return set_status_code(METHOD_NOT_ALLOWED, false,
                                   "parse_request_line",
                                   method_ + "is not allowed");

        request_uri_ = get_next_token(request_line, &cur_pos, SP);
        if (!request_form(request_uri_))
            return set_status_code(BAD_REQUEST, false, "parse_request_line",
                                   "bad request form");

        http_version_ = request_line.substr(cur_pos, request_line.length());

        return check_http_version();
    }

    // ========================= HEADERS =========================

    bool Request::parse_header_line(const std::string& request, size_t* cur_pos)
    {
        std::string header_line = get_next_token(request, cur_pos, CRLF);
        while (!header_line.empty())
        {
            auto key_value = get_key_and_value(header_line);
            if (key_value.first.empty() || key_value.second.empty()
                || contains_space(key_value.first))
                return set_status_code(BAD_REQUEST, false, "parse_header_line",
                                       "one of the header is incorrect");

            // if we have the same headers
            if (header_.find(key_value.first) != header_.end())
                return set_status_code(BAD_REQUEST, false, "parse_header_line",
                                       "the header " + key_value.first
                                           + " is already present");

            header_[key_value.first] = key_value.second;

            if (*cur_pos + 2 > request.length()
                || (request[*cur_pos] == '\r' && request[*cur_pos + 1] == '\n'))
                break;

            header_line = get_next_token(request, cur_pos, CRLF);
        }
        return true;
    }

    void Request::setCurrentVhost(const std::shared_ptr<VHost>& currentVhost)
    {
        current_vhost_ = currentVhost;
    }

    bool Request::isErrorPreDispatch() const
    {
        return error_pre_dispatch;
    }
    bool Request::is_status_correct() const
    {
        return status_code_ == 200 || status_code_ == 201;
    }

} // namespace request