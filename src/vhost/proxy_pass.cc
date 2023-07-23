#include "proxy_pass.hh"

#include <utility>
Proxy_pass::Proxy_pass() = default;

void Proxy_pass::setIpProxy(const boost::asio::ip::address& ipProxy)
{
    ip_proxy_ = ipProxy;
}

void Proxy_pass::setPortProxy(const std::optional<int>& portProxy)
{
    port_proxy_ = portProxy;
}

void Proxy_pass::setUpstream(const std::string& upstream)
{
    upstream_ = upstream;
}

void Proxy_pass::setProxyRemoveHeader(
    const std::optional<std::vector<std::string>>& proxyRemoveHeader)
{
    proxy_remove_header_ = proxyRemoveHeader;
}

void Proxy_pass::setProxySetHeader(
    const std::optional<std::unordered_map<std::string, std::string>>&
        proxySetHeader)
{
    proxy_set_header_ = proxySetHeader;
}

void Proxy_pass::setSetHeader(
    const std::optional<std::unordered_map<std::string, std::string>>&
        setHeader)
{
    set_header_ = setHeader;
}

void Proxy_pass::setRemoveHeader(
    const std::optional<std::vector<std::string>>& removeHeader)
{
    remove_header_ = removeHeader;
}

const boost::asio::ip::address& Proxy_pass::getIpProxy() const
{
    return ip_proxy_;
}

const std::optional<int>& Proxy_pass::getPortProxy() const
{
    return port_proxy_;
}

const std::string& Proxy_pass::getUpstream() const
{
    return upstream_;
}

const std::optional<std::vector<std::string>>&
Proxy_pass::getProxyRemoveHeader() const
{
    return proxy_remove_header_;
}

const std::optional<std::unordered_map<std::string, std::string>>&
Proxy_pass::getProxySetHeader() const
{
    return proxy_set_header_;
}

const std::optional<std::unordered_map<std::string, std::string>>&
Proxy_pass::getSetHeader() const
{
    return set_header_;
}

const std::optional<std::vector<std::string>>&
Proxy_pass::getRemoveHeader() const
{
    return remove_header_;
}