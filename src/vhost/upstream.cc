#include "upstream.hh"

void Upstream::setMethod(const std::string& method)
{
    method_ = method;
}

void Upstream::setHost(const std::unordered_map<std::string, std::string>& host)
{
    host_ = host;
}

void Upstream::setHosts(
    const std::vector<std::unordered_map<std::string, std::string>>& hosts)
{
    hosts_ = hosts;
}

const std::string& Upstream::getMethod() const
{
    return method_;
}

const std::unordered_map<std::string, std::string>& Upstream::getHost() const
{
    return host_;
}

const std::vector<std::unordered_map<std::string, std::string>>&
Upstream::getHosts() const
{
    return hosts_;
}

Upstream::Upstream() = default;
