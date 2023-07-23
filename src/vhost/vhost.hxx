#pragma once

inline const boost::asio::ip::address& VHost::getIp() const
{
    return ip_;
}
inline unsigned short VHost::getPort() const
{
    return port_;
}
inline const std::string& VHost::getServerName() const
{
    return server_name_;
}
inline const std::string& VHost::getRoot() const
{
    return root_;
}
inline bool VHost::isCudFiles() const
{
    return cud_files_;
}
inline const std::string& VHost::getDefaultFile() const
{
    return default_file_;
}
inline bool VHost::isDefaultVhost() const
{
    return is_default_vhost_;
}
inline const std::string& VHost::getSslCert() const
{
    return ssl_cert_;
}
inline const std::string& VHost::getSslKey() const
{
    return ssl_key_;
}

inline bool VHost::isCrypted() const
{
    return is_crypted_;
}
