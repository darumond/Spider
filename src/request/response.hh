#pragma once

#include <map>

#include "io_operation.hh"
#include "request.hh"
#include "utils/types.hh"

namespace request
{

    template <class value_type>
    struct header
    {
        std::string field;
        value_type value;

        const std::string header_line() const
        {
            std::stringstream header;
            header << field << ": " << value << CRLF;
            return header.str();
        }
    };

    class Response
    {
    public:
        typedef boost::function<std::string(Response&)> header_func;

        Response() = default;

        Response(int status_code, const std::string& method,
                 const std::string& uri, const std::string& request_body,
                 const bool& isKeepAlive);

        std::string response();

        std::string status_line();

        std::string date();

        std::string connection();

        std::string specific_headers();

        std::string get_200_header();

        static std::string get_reason_phrase(int status_code);

        std::string generate_header(const std::string& field,
                                    const std::string& value);

        static std::string format_date(tm& tm);

        void set_file_info();

        const header<off_t>& getContentLength() const;

        const header<std::string>& getContentLocation() const;

        const header<std::string>& getLastModified() const;

        std::string head_200_header();

        std::string put_header();

        std::string delete_200_header();

        std::string put_200_header();

        std::string put_201_header();

        std::string two_status();

        std::string four_status();

        std::string allow_header();

        std::string five_status();

        void set_body();

        bool isSendBody() const;

    private:
        std::map<std::string, std::string> header_;
        std::string http_version_;
        int status_code_;
        std::string method_;
        std::string uri_;
        std::string reason_phrase_;
        bool isKeepAlive_;
        header<off_t> content_length;
        header<std::string> last_modified;
        header<std::string> content_location;
        bool send_body_;
        std::string request_body_;
    };
} // namespace request
