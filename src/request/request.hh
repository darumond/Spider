#pragma once

#include <boost/asio.hpp>
#include <cerrno>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <list>
#include <regex>
#include <utility>

#include "../misc/fd.hh"
#include "../misc/socket.hh"
#include "../misc/sys-wrapper.hh"
#include "../misc/unistd.hh"
#include "json.hpp"
#include "utils/types.hh"
#include "utils/utils.hh"
#include "vhost/vhost.hh"

namespace request
{
    using boost::asio::ip::tcp;

    class Request
    {
    public:
        Request() = default;
        Request(std::string& request);

        int getStatusCode() const;
        const std::string& getMethod() const;
        const std::string& getRequestUri() const;
        const std::map<std::string, std::string>& getHeader() const;
        const std::string& getHttpVersion() const;
        const std::string& getResourcePath() const;
        const VHost& getCurrentVhost() const;
        bool isDirectory() const;

        void setIsDirectory(bool isDirectory);
        void setResourcePath(const std::string& resourcePath);
        bool set_status_code(int status, bool success,
                             const std::string& function,
                             const std::string& error_message);
        size_t getContentLength() const;
        void check_after_dispatch(const std::shared_ptr<VHost>& vhost);
        bool isErrorPreDispatch() const;
        bool is_status_correct() const;
        bool keep_alive_;
        bool isKeepAlive() const;
        void setCurrentVhost(const std::shared_ptr<VHost>& currentVhost);

    private:
        std::shared_ptr<VHost> current_vhost_;
        int status_code_ = OK;
        request_target request_form_ = ERROR_FORM;
        std::string method_;
        std::string request_uri_;
        std::string http_version_;
        std::string host_;
        std::string resource_path_;
        bool is_directory{};

    private:
        size_t content_length_ = 0;

        bool error_pre_dispatch = false;

    private:
        std::map<std::string, std::string> header_;

    private:
        // main function to parse request
        bool parse_request(std::string& request);

        // utils
        bool check_host_format(std::string& host);
        bool is_mandatory_headers_present_and_valid();
        bool check_http_version();
        void print_request();

        bool is_absolute_form(const std::string& request_uri);
        bool is_origin_form(const std::string& request_uri);

        // parse request sub-functions
        bool request_form(const std::string& request_uri);
        bool parse_request_line(const std::string& request_line);
        bool parse_header_line(const std::string& request, size_t* cur_pos);
        void update_keep_alive();
    };

    // handle logic part
    bool is_operation_authorised(request::Request& req,
                                 const std::string& path);
} // namespace request

#include "request.hxx"