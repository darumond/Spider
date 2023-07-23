#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
#include <iostream>
#include <optional>
#include <utility>

class Proxy_pass
{
public:
    Proxy_pass();

    void setIpProxy(const boost::asio::ip::address& ipProxy);
    void setPortProxy(const std::optional<int>& portProxy);
    void setUpstream(const std::string& upstream);
    void setProxyRemoveHeader(
        const std::optional<std::vector<std::string>>& proxyRemoveHeader);
    void setProxySetHeader(
        const std::optional<std::unordered_map<std::string, std::string>>&
            proxySetHeader);
    void setSetHeader(
        const std::optional<std::unordered_map<std::string, std::string>>&
            setHeader);
    void setRemoveHeader(
        const std::optional<std::vector<std::string>>& removeHeader);

    const boost::asio::ip::address& getIpProxy() const;
    const std::optional<int>& getPortProxy() const;
    const std::string& getUpstream() const;
    const std::optional<std::vector<std::string>>& getProxyRemoveHeader() const;
    const std::optional<std::unordered_map<std::string, std::string>>&
    getProxySetHeader() const;
    const std::optional<std::unordered_map<std::string, std::string>>&
    getSetHeader() const;
    const std::optional<std::vector<std::string>>& getRemoveHeader() const;

    boost::asio::ip::address ip_proxy_;
    std::optional<int> port_proxy_;
    std::string upstream_;

    std::optional<std::vector<std::string>> proxy_remove_header_;

    std::optional<std::unordered_map<std::string, std::string>>
        proxy_set_header_;
    std::optional<std::unordered_map<std::string, std::string>> set_header_;
    std::optional<std::vector<std::string>> remove_header_;
};