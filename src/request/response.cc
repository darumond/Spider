#include "response.hh"

std::string request::Response::status_line()
{
    std::stringstream status_line;
    status_line << http_version_ << " " << status_code_ << " " << reason_phrase_
                << CRLF;
    return status_line.str();
}

std::string request::Response::format_date(struct tm& tm)
{
    char formated_date[1000];
    strftime(formated_date, sizeof formated_date, "%a, %d %b %Y %H:%M:%S %Z",
             &tm);
    std::stringstream res;
    res << formated_date;
    return res.str();
}

std::string request::Response::date()
{
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    auto formated_date = format_date(tm);
    std::stringstream date_header;
    date_header << "Date: " << formated_date << CRLF;
    return date_header.str();
}

std::string request::Response::get_reason_phrase(int status_code)
{
    switch (status_code)
    {
    case BAD_REQUEST:
        return "Bad Request";
    case OK:
        return "OK";
    case CREATED:
        return "CREATED";
    case FORBIDDEN:
        return "Forbidden";
    case NOT_FOUND:
        return "Not Found";
    case METHOD_NOT_ALLOWED:
        return "Method Not Allowed";
    case CONFLICT:
        return "Conflict";
    case UPGRADE_REQUIRED:
        return "Upgrade Required";
    case BAD_GATEWAY:
        return "BAD GATEWAY";
    default:
        return "ERROR";
    }
}

std::string request::Response::connection()
{
    std::stringstream connection;
    if (isKeepAlive_)
        connection << "Connection: keep-alive" << CRLF;
    else
        connection << "Connection: close" << CRLF;
    return connection.str();
}

std::string request::Response::response()
{
    std::stringstream response;
    response << status_line() << date() << connection() << specific_headers()
             << CRLF;
    return response.str();
}

std::string request::Response::specific_headers()
{
    std::map<int, header_func> headers{ { 2, &Response::two_status },
                                        { 4, &Response::four_status },
                                        { 5, &Response::five_status } };
    return headers[status_code_ / 100](*this);
}

void request::Response::set_file_info()
{
    struct stat file_info
    {};
    stat(uri_.c_str(), &file_info);
    last_modified.field = "Last-Modified";
    last_modified.value = format_date(*localtime(&file_info.st_mtime));
    content_length.field = "Content-Length";
    content_length.value = file_info.st_size;
    content_location.field = "Content-Location";
    content_location.value = uri_;
}

std::string request::Response::two_status()
{
    set_file_info();
    send_body_ = true;
    std::map<std::string, header_func> headers{
        { "GET", &Response::get_200_header },
        { "HEAD", &Response::head_200_header },
        { "PUT", &Response::put_header },
        { "DELETE", &Response::delete_200_header }
    };
    std::stringstream header_key;
    header_key << method_;
    return headers[header_key.str()](*this);
}

std::string request::Response::four_status()
{
    send_body_ = false;
    std::stringstream header;
    content_length.value = 0;
    content_length.field = "Content-Length";
    if (status_code_ == 405)
        header << allow_header();
    header << content_length.header_line();
    return header.str();
}

std::string request::Response::five_status()
{
    send_body_ = false;
    std::stringstream header;
    content_length.value = 0;
    content_length.field = "Content-Length";
    header << content_length.header_line();
    return header.str();
}

std::string request::Response::put_header()
{
    content_length.value = 0;
    send_body_ = false;
    io_operation::put(this->uri_, this->request_body_);
    std::map<std::string, header_func> headers{
        { "PUT200", &Response::put_200_header },
        { "PUT201", &Response::put_201_header }
    };
    std::stringstream header_key;
    header_key << method_ << status_code_;
    return headers[header_key.str()](*this);
}

std::string request::Response::delete_200_header()
{
    std::stringstream header;
    remove(uri_.c_str());
    content_length.value = 0;
    send_body_ = false;
    header << content_length.header_line() << content_location.header_line();
    return header.str();
}

std::string request::Response::put_200_header()
{
    return content_length.header_line();
}

std::string request::Response::put_201_header()
{
    std::stringstream header;
    header << content_length.header_line() << content_location.header_line();
    return header.str();
}

std::string request::Response::get_200_header()
{
    std::stringstream header;
    header << last_modified.header_line() << content_length.header_line();
    return header.str();
}

std::string request::Response::head_200_header()
{
    std::stringstream header;
    content_length.value = 0;
    send_body_ = false;
    header << last_modified.header_line() << content_length.header_line();
    return header.str();
}

std::string request::Response::allow_header()
{
    std::stringstream allow;
    if (std::filesystem::is_directory(uri_))
        allow << "Allow: GET, HEAD";
    else
        allow << "Allow: GET, HEAD, PUT, DELETE";
    allow << CRLF;
    return allow.str();
}

request::Response::Response(int status_code, const std::string& method,
                            const std::string& uri,
                            const std::string& request_body,
                            const bool& isKeepAlive)
    : http_version_("HTTP/1.1")
    , status_code_(status_code)
    , method_(method)
    , uri_(uri.c_str())
    , reason_phrase_(get_reason_phrase(status_code))
    , send_body_(false)
    , request_body_(request_body)
    , isKeepAlive_(isKeepAlive)
{}

const request::header<off_t>& request::Response::getContentLength() const
{
    return content_length;
}

const request::header<std::string>& request::Response::getLastModified() const
{
    return last_modified;
}

const request::header<std::string>&
request::Response::getContentLocation() const
{
    return content_location;
}

bool request::Response::isSendBody() const
{
    return send_body_;
}
