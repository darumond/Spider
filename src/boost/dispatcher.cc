#include "dispatcher.hh"

Dispatcher::Dispatcher(const std::string& host_header, const std::string& ip,
                       const unsigned short& port)
    : port_(port)
{
    /*
     * check if host have the following format:  uri-host [ ":" port ]
     *  uri-host == server_name or ip et port == port
     */

    // Yaka: ipv6 can be "[" ip "]"
    std::string host_processed = remove_bracket(host_header);
    to_lower_case(host_processed);
    ip_ = ip;
    auto boost_ip = boost::asio::ip::address::from_string(ip_);
    boost::system::error_code error_code;
    if (host_processed.find(':') == std::string::npos)
    {
        boost::asio::ip::address::from_string(host_processed, error_code);
        if (error_code.failed())
            server_name_ = host_processed;
        else if (!is_wildcard_ip(boost_ip) && ip_ != host_processed)
            error_ = true;
        return;
    }
    auto uri_and_port = get_key_and_value(host_processed);
    boost::asio::ip::address::from_string(uri_and_port.first, error_code);
    if (error_code)
        server_name_ = uri_and_port.first;
    else
    {
        if (ip != uri_and_port.first && !is_wildcard_ip(boost_ip))
            error_ = true;
    }
    try
    {
        if (port_ != std::stoi(uri_and_port.second))
            error_ = true;
    }
    catch (...)
    {
        error_ = true;
    }
}

bool Dispatcher::need_to_dispatch()
{
    /*
     * Host: server_name | IP | server_name:port | IP:port
     */

    // ===================== Find Vhosts with Host =====================
    auto vhosts = Config::getInstance().get_vhost();
    auto vhosts_to_dispatch = std::vector<std::shared_ptr<VHost>>();
    for (const auto& vhost : vhosts)
    {
        if ((ip_ == vhost->getIp().to_string()
             || vhost->getIp().to_string() == "0.0.0.0"
             || vhost->getIp().to_string() == "0::0"
             || is_wildcard_ip(vhost->getIp()))
            && (port_ == vhost->getPort()))
        {
            if (!server_name_.empty())
            {
                if (server_name_ == vhost->getServerName())
                    vhosts_to_dispatch.push_back(vhost);
            }
            else
            {
                if (ip_
                    == vhost->getIp()
                           .to_string()) // If there is wildcard this
                                         // get priority to specific ip
                    vhosts_to_dispatch.push_back(vhost);
            }
        }
    }

    // ===================== No Vhost to dispatch =====================
    if (vhosts_to_dispatch.empty())
    {
        std::clog << "Dispatcher(): cannot find the vhost to dispatch\n";
        error_ = true;
        return false;
    }

    // ========================= No ambiguity =========================
    if (vhosts_to_dispatch.size() == 1)
    {
        vhost_ = vhosts_to_dispatch[0];
        return true;
    }

    // ===================== Find the default vhost =====================
    for (const auto& vhost : vhosts_to_dispatch)
    {
        if (vhost->isDefaultVhost())
        {
            vhost_ = vhost;
            return true;
        }
    }

    // ===================== No default Vhost =====================
    error_ = true;
    return false;
}

std::shared_ptr<VHost> Dispatcher::find_vhost()
{
    return vhost_;
}
bool Dispatcher::isError() const
{
    return error_;
}
const std::string& Dispatcher::getServerName() const
{
    return server_name_;
}
