#pragma once

namespace request
{
    // ======================== GETTERS ========================
    inline int Request::getStatusCode() const
    {
        return status_code_;
    }
    inline const std::string& Request::getMethod() const
    {
        return method_;
    }
    inline const std::string& Request::getRequestUri() const
    {
        return request_uri_;
    }
    inline const std::map<std::string, std::string>& Request::getHeader() const
    {
        return header_;
    }
    inline const std::string& Request::getHttpVersion() const
    {
        return http_version_;
    }
    inline const std::string& Request::getResourcePath() const
    {
        return resource_path_;
    }
    inline const VHost& Request::getCurrentVhost() const
    {
        return *current_vhost_;
    }
    inline void Request::setResourcePath(const std::string& resourcePath)
    {
        resource_path_ = resourcePath;
    }
    inline bool Request::isDirectory() const
    {
        return is_directory;
    }
    inline void Request::setIsDirectory(bool isDirectory)
    {
        is_directory = isDirectory;
    }
    inline size_t Request::getContentLength() const
    {
        return content_length_;
    }
    inline bool Request::isKeepAlive() const
    {
        return keep_alive_;
    }
} // namespace request