#pragma once

#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <boost/function.hpp>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

#include "json.hpp"
#include "vhost/vhost.hh"

#define NB_REQUIRED_KEYS 4
#define NB_MAX_OPTIONAL_KEYS 3

using json = nlohmann::json;
typedef boost::function<bool(const json::value_type& value)> function;

class Config
{
public:
    // ================= Instance =================
    static Config& getInstance(const std::string& path = "")
    {
        static auto instance_ = Config(path);
        return instance_;
    }

    // ================= Getter =================
    bool is_valid_config() const;
    std::vector<std::shared_ptr<VHost>>& get_vhost();

    // ================= Step 1 =================
    bool check_vhosts(const json& vhosts_data);
    bool check_required_keys(const json& vhost);
    bool check_optional_keys(const json& vhost);
    static bool check_ip(const json::value_type& ip);
    static bool check_port(const json::value_type& port);
    static bool check_server_name(const json::value_type& server_name);
    static bool check_root(const json::value_type& root);
    bool check_cud_files(const json::value_type& cud_files);
    bool check_default_file(const json::value_type& default_file);

    // ================= Step 2 =================
    bool check_is_default_vhost(const json::value_type& is_default_vhost);
    bool check_ssl_cert(const json::value_type& ssl_cert);
    bool check_ssl_key(const json::value_type& ssl_key);

    // ================= Logger =================
    void display_vhosts();
    static bool log_error(const std::string& function, const std::string&);

    // ================= Step 3 ==================
    bool check_proxy_pass(json proxy);
    void init_proxy_pass(json proxy);
    bool check_keys_method(const json& hosts, const std::string& method);
    bool check_name_upstream(const json& json_data);
    bool check_upstream(const json& json_data);
    void init_upstream();

private:
    void init_vhosts();
    explicit Config(const std::string& file_path);

    bool valid_config_ = true;
    json vhosts_data_;
    json upstreams_data_;
    int nb_ssl_key{};
    bool is_proxy_pass_ = false;

    std::vector<std::shared_ptr<VHost>> vhosts;
    Proxy_pass proxyPass_;
    std::unordered_map<std::string, Upstream> upstreams_;
    std::vector<std::string> list_upstreams_;
};

#include "config.hxx"