#include "config.hh"

#include <regex>
#include <stdexcept>
#include <utility>
#include <vector>

// ========================== Constructor ==========================
Config::Config(const std::string& file_path)
{
    std::ifstream json_file_reader(file_path);
    if (!json_file_reader.is_open())
    {
        std::clog << "Config(constructor): failed to open " << file_path
                  << '\n';
        valid_config_ = false;
        return;
    }

    json file = json::parse(json_file_reader);

    if (!check_vhosts(file))
    {
        valid_config_ = false;
        return;
    }

    init_vhosts();

    if (!check_upstream(file))
    {
        valid_config_ = false;
        return;
    }

    init_upstream();
}

std::vector<std::string> get_vector(const json& element)
{
    std::vector<std::string> vector;
    for (const auto& el : element)
    {
        vector.push_back(el.get<std::string>());
    }
    return vector;
}

bool check_map_headers(const json& element)
{
    for (const auto& itr : element)
    {
        if (!itr.is_string())
            return false;
    }
    return true;
}

bool check_list_headers(const json& element)
{
    for (const auto& itr : element)
    {
        if (!itr.is_string())
            return false;
    }
    return true;
}

void set_list_headers(std::vector<std::string>& list, const json& element)
{
    for (const auto& itr : element)
    {
        list.emplace_back(itr);
    }
}

void set_map_headers(
    std::unordered_map<std::string, std::string>& proxy_set_header,
    const json& element)
{
    for (auto itr = element.begin(); itr != element.end(); itr++)
    {
        proxy_set_header.insert(
            std::make_pair<std::string, std::string>(itr.key().c_str(), *itr));
    }
}

bool check_args(const json& proxy)
{
    std::vector<std::string> possible = {
        "upstream",         "ip",         "port",         "proxy_remove_header",
        "proxy_set_header", "set_header", "remove_header"
    };

    for (auto itr = proxy.begin(); itr != proxy.end(); itr++)
    {
        if (std::find(possible.begin(), possible.end(), itr.key().c_str())
            == possible.end())
            return false;
    }
    return true;
}

bool Config::check_proxy_pass(json proxy)
{
    if (!check_args(proxy))
        return false;

    bool is_upstream;
    bool no_good_port_ip =
        ((!proxy.contains("ip") && !proxy.contains("port"))
         || (proxy.contains("ip") && !proxy.contains("port"))
         || (!proxy.contains("ip") && proxy.contains("port")));

    if (!proxy.contains("upstream") && no_good_port_ip
        || (proxy.contains("upstream")
            && (proxy.contains("ip") || proxy.contains("port"))))
        return false;
    is_upstream = proxy.contains("upstream");
    std::cout << "size of json: " << proxy.size() << std::endl;
    if (!is_upstream)
    {
        if (!check_ip(proxy["ip"]))
            return false;
        if (!check_port(proxy["port"]))
            return false;
        if (proxy.size() > 6)
            return false;
    }
    if (is_upstream && proxy.size() > 5)
        return false;
    if (proxy.contains("proxy_set_header")
        && !check_map_headers(proxy["proxy_set_header"]))
        return false;
    if (proxy.contains("set_header") && !check_map_headers(proxy["set_header"]))
        return false;
    if (proxy.contains("proxy_remove_header")
        && !check_list_headers(proxy["proxy_remove_header"]))
        return false;
    if (proxy.contains("remove_header")
        && !check_list_headers(proxy["remove_header"]))
        return false;
    return true;
}

void Config::init_proxy_pass(json proxy)
{
    proxyPass_ = Proxy_pass();

    if (!check_proxy_pass(proxy))
    {
        valid_config_ = false;
        Config::log_error("init vhost", "proxy_pass not valid");
        return;
    }

    auto is_upstream = proxy.contains("upstream");
    std::unordered_map<std::string, std::string> proxy_set_header;
    set_map_headers(proxy_set_header, proxy["proxy_set_header"]);
    std::unordered_map<std::string, std::string> set_header;
    set_map_headers(set_header, proxy["set_header"]);

    std::vector<std::string> proxy_remove_header;
    set_list_headers(proxy_remove_header, proxy["proxy_remove_header"]);
    std::vector<std::string> remove_header;
    set_list_headers(remove_header, proxy["remove_header"]);

    proxyPass_.setProxySetHeader(proxy_set_header);
    proxyPass_.setSetHeader(set_header);
    proxyPass_.setProxyRemoveHeader(proxy_remove_header);
    proxyPass_.setRemoveHeader(remove_header);

    if (!valid_config_)
    {
        Config::log_error("init vhost", "proxy_pass not valid");
        return;
    }

    if (is_upstream)
    {
        proxyPass_.setUpstream(proxy["upstream"]);
        list_upstreams_.push_back(proxy["upstream"]);
    }
    else
    {
        proxyPass_.setIpProxy(
            boost::asio::ip::address::from_string(proxy["ip"]));
        proxyPass_.setPortProxy(proxy["port"]);
    }
}

void Config::init_vhosts()
{
    for (auto vhost : vhosts_data_)
    {
        boost::system::error_code error_code;
        auto ip =
            boost::asio::ip::address::from_string(vhost["ip"], error_code);
        bool cud_file = false;
        if (vhost["cud_files"] != nullptr)
            cud_file = vhost["cud_files"];

        std::string defaultFiles;
        if (vhost["default_file"] != nullptr)
        {
            std::string file_path = vhost["root"].get<std::string>()
                + vhost["default_file"].get<std::string>();
            const std::filesystem::path path(file_path);
            std::error_code ec;
            if (std::filesystem::is_directory(path, ec))
                defaultFiles = "index.html";
            else
                defaultFiles = vhost["default_file"];
        }
        else
            defaultFiles = "index.html";

        bool is_default_vhost = false;
        std::string ssl_cert = std::string();
        std::string ssl_key = std::string();

        if (vhost.contains("is_default_vhost"))
            is_default_vhost = vhost["is_default_vhost"];

        vhost.contains("proxy_pass") ? is_proxy_pass_ = true
                                     : is_proxy_pass_ = false;
        if (is_proxy_pass_)
            init_proxy_pass(vhost["proxy_pass"]);

        if (vhost.contains("ssl_cert"))
        {
            ssl_cert = vhost["ssl_cert"];
            ssl_key = vhost["ssl_key"];
            try
            {
                if (is_proxy_pass_)
                    vhosts.emplace_back(std::make_shared<VHost>(
                        ip, vhost["port"], vhost["server_name"], "", cud_file,
                        defaultFiles, is_default_vhost, ssl_cert, ssl_key,
                        proxyPass_));
                else
                    vhosts.emplace_back(std::make_shared<VHost>(
                        ip, vhost["port"], vhost["server_name"], vhost["root"],
                        cud_file, defaultFiles, is_default_vhost, ssl_cert,
                        ssl_key, std::nullopt));
            }
            catch (const std::exception& e)
            {
                valid_config_ = false;
                Config::log_error("init vhost", e.what());
            }
        }
        else
        {
            if (is_proxy_pass_)
                vhosts.emplace_back(std::make_shared<VHost>(
                    ip, vhost["port"], vhost["server_name"], "", cud_file,
                    defaultFiles, is_default_vhost, proxyPass_));
            else
                vhosts.emplace_back(std::make_shared<VHost>(
                    ip, vhost["port"], vhost["server_name"], vhost["root"],
                    cud_file, defaultFiles, is_default_vhost, std::nullopt));
        }
    }
}

void Config::init_upstream()
{
    for (const std::string& upstream_str : list_upstreams_)
    {
        Upstream up = Upstream();
        json upstream = upstreams_data_[upstream_str];
        up.setMethod(upstream["method"]);

        std::vector<std::unordered_map<std::string, std::string>> list_up_map;

        for (json& item : upstream["hosts"])
        {
            std::unordered_map<std::string, std::string> up_map;
            up_map["ip"] = item["ip"];
            up_map["port"] = to_string(item["port"]);
            if (item.contains("health"))
                up_map["health"] = item["health"];
            if (item.contains("weight"))
                up_map["weight"] = to_string(item["weight"]);
            list_up_map.push_back(up_map);
        }

        up.setHosts(list_up_map);
        upstreams_[upstream_str] = up;
    }
}

bool Config::check_keys_method(const json& hosts, const std::string& method)
{
    for (const auto& host : hosts)
    {
        if (!host.contains("ip") || !host.contains("port"))
            return false;
        if (host.contains("weight")
            && (!host["weight"].is_number()
                || (host["weight"].is_number() && host["weight"] < 1)))
            return false;
        if ((method == "fail-over" || method == "fail-robin")
            && !host.contains("health"))
            return false;
    }
    return true;
}

bool Config::check_name_upstream(const json& json_data)
{
    for (const std::string& item : list_upstreams_)
    {
        if (json_data.contains(item))
            return json_data.size() == list_upstreams_.size();
    }
    return false;
}

bool Config::check_upstream(const json& json_data)
{
    if (!json_data.contains("upstreams") && list_upstreams_.empty())
        return true;

    bool is_upstreams = json_data.contains("upstreams");
    if ((is_upstreams && list_upstreams_.empty())
        || (!is_upstreams && !list_upstreams_.empty()))
        return Config::log_error("check_upstreams",
                                 "no upstreams or no upstream args");

    json upstream_data = json_data["upstreams"];
    upstreams_data_ = json_data["upstreams"];

    if (!check_name_upstream(upstream_data))
        return false;

    for (const std::string& upstream_str : list_upstreams_)
    {
        json upstream = upstream_data[upstream_str];
        if (!upstream.contains("method")
            || (!upstream.contains("hosts")
                || (upstream.contains("hosts") && upstream["hosts"].empty())))
            return Config::log_error("check_upstreams",
                                     "method or hosts missing");

        if (upstream["hosts"].empty())
            return false;

        if (!(check_keys_method(upstream["hosts"], upstream["method"])))
            return false;
    }
    return true;
}

// ========================== Step 1 ==========================

bool Config::check_vhosts(const json& json_data)
{
    if (!json_data.contains("vhosts"))
        return Config::log_error("check_vhosts", "No Vhost");

    auto vhost_data = json_data["vhosts"];
    vhosts_data_ = vhost_data;

    for (const auto& vhost : vhost_data)
    {
        if (vhost.contains("ssl_cert") && vhost.contains("ssl_key"))
            nb_ssl_key = 2;
        else if (!(vhost.contains("ssl_cert") && vhost.contains("ssl_key")))
            nb_ssl_key = 0;
        else
            return Config::log_error("check_vhosts",
                                     "ssl_cert or ssl_key is missing");

        if (!(check_required_keys(vhost) && check_optional_keys(vhost)))
            return false;
    }
    return true;
}

bool Config::check_required_keys(const json& vhost)
{
    if (vhost.size() < NB_REQUIRED_KEYS + nb_ssl_key)
        return false;

    std::vector<std::string> required_keys{ "ip", "port", "server_name" };
    std::vector<function> check_keys{ Config::check_ip, Config::check_port,
                                      Config::check_server_name };
    int i = 0;
    for (const auto& key : required_keys)
    {
        if (!vhost.contains(key))
            return Config::log_error("check_required_keys",
                                     "no " + key + " key");
        if (!check_keys[i](vhost[key]))
            return false;
        i++;
    }

    if (vhost.contains("root") && vhost.contains("proxy_pass"))
        return Config::log_error("check_required_keys",
                                 "root and upstream are exclusive");

    if (!vhost.contains("root") && !vhost.contains("proxy_pass"))
        return Config::log_error("check_required_keys",
                                 "neither root or proxy_pass is present");
    if (!vhost.contains("proxy_pass") && vhost.contains("root"))
    {
        if (!check_root(vhost["root"]))
            return false;
    }
    if (vhost.contains("ssl_cert")
        && !(check_ssl_cert(vhost["ssl_cert"])
             && check_ssl_key(vhost["ssl_key"])))
        return Config::log_error("check_required_key",
                                 "ssl_cert or ssl_key does not exist");
    return true;
}

bool Config::check_optional_keys(const json& vhost)
{
    if (vhost.size() > NB_REQUIRED_KEYS + NB_MAX_OPTIONAL_KEYS + nb_ssl_key)
        return false;

    int count_nb_optional_key = 0;

    if (vhost.contains("cud_files"))
    {
        if (!check_cud_files(vhost["cud_files"]))
            return false;
        count_nb_optional_key++;
    }

    if (vhost.contains("default_file"))
    {
        if (!check_default_file(vhost["default_file"]))
            return false;
        count_nb_optional_key++;
    }

    if (vhost.contains("is_default_vhost"))
    {
        if (!check_is_default_vhost(vhost["is_default_vhost"]))
            return false;
        count_nb_optional_key++;
    }

    int nb_keys = NB_REQUIRED_KEYS + count_nb_optional_key + nb_ssl_key;
    if (nb_keys != vhost.size())
        return Config::log_error("check_optional_key",
                                 "nb key incorrect, expected: "
                                     + std::to_string(vhost.size())
                                     + " got: " + std::to_string(nb_keys));
    return true;
}

bool Config::check_ip(const json::value_type& ip)
{
    if (!ip.is_string())
        return Config::log_error("check_ip", "Ip error type");

    boost::system::error_code error_code;
    boost::asio::ip::address::from_string(ip, error_code);
    if (error_code)
    {
        std::clog << "Config(check_ip): bad ip " << ip << " ,"
                  << error_code.message() << std::endl;
        return false;
    }
    return true;
}

bool Config::check_port(const json::value_type& port)
{
    if (!port.is_number_integer())
        return Config::log_error("check_port", "Port error type");
    if (!(port >= 0 && port <= UINT16_MAX))
        return Config::log_error("check_port", "Port not valid");
    return true;
}

bool Config::check_server_name(const json::value_type& server_name)
{
    // Yaka: "There is not restriction for server name" (ex: fsd$*fs_@2)
    if (!(server_name.is_string()))
        return Config::log_error("check_server_name",
                                 "Server name error\n Server Name: "
                                     + to_string(server_name));
    return true;
}

bool Config::check_root(const json::value_type& root)
{
    if (!root.is_string())
        return Config::log_error("check_root", "Root error type");
    return true;
}

bool Config::check_cud_files(const json::value_type& cud_files)
{
    if (!cud_files.is_boolean())
        return Config::log_error("check_cud_file", "Cud file error type");
    return true;
}

bool Config::check_default_file(const json::value_type& default_file)
{
    if (!default_file.is_string())
        return Config::log_error("check_default_file",
                                 "default file error type");
    // path is not accepted
    if (to_string(default_file).find('/') != std::string::npos)
        return Config::log_error("check_default_file",
                                 "default file does not support path");
    return true;
}

// ========================== Step 2 ==========================

bool Config::check_is_default_vhost(const json::value_type& is_default_vhost)
{
    if (!is_default_vhost.is_boolean())
        return Config::log_error("check_is_default_vhost",
                                 "is_default_vhost error type");
    return true;
}

bool Config::check_ssl_cert(const json::value_type& ssl_cert)
{
    if (!ssl_cert.is_string())
        return Config::log_error("check_ssl_cert", "ssl_cert error type");
    try
    {
        std::ifstream ssl_cert_file(ssl_cert);
        if (!ssl_cert_file.good())
        {
            return Config::log_error("check_ssl_cert", "ssl_cert no file");
        }
    }
    catch (std::exception& e)
    {
        return Config::log_error("check_ssl_cert", e.what());
    }
    /* if (!std::filesystem::exists(ssl_cert))
     *   return Config::log_error("check_ssl_cert",
     *                            "ssl_cert file does not exist");
     */
    return true;
}

bool Config::check_ssl_key(const json::value_type& ssl_key)
{
    if (!ssl_key.is_string())
        return Config::log_error("check_ssl_key", "ssl_key error type");
    try
    {
        std::ifstream ssl_key_file(ssl_key);
        if (!ssl_key_file.good())
        {
            return Config::log_error("check_ssl_key", "ssl_key no file");
        }
    }
    catch (std::exception& e)
    {
        return Config::log_error("check_ssl_key", e.what());
    }
    /* if (!std::filesystem::exists(ssl_key))
     *   return Config::log_error("check_ssl_key",
     *                            "ssl_key file does not exist");
     */
    return true;
}

// ========================== Logger ==========================

void Config::display_vhosts()
{
    for (auto& vhost : vhosts)
        vhost->print_vhost();
}

bool Config::log_error(const std::string& function, const std::string& msg)
{
    std::clog << function << "(): " << msg << "\n";
    return false;
}