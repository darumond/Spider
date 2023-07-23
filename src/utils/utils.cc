#include "utils.hh"

// =================== main utils =================== //

bool invalid_server_configuration()
{
    if (!Config::getInstance().is_valid_config())
        return true;
    if (!check_vhosts_validity())
        return true;
    return false;
}

bool is_wildcard_ip(const boost::asio::ip::address& vhost_ip)
{
    if (vhost_ip.is_v4())
        return vhost_ip.to_string() == "0.0.0.0";
    return vhost_ip.to_string() == "::";
}

bool is_same_ip_type(const VHost& v1, const VHost& v2)
{
    return (v1.getIp().is_v4() && v2.getIp().is_v4())
        || (v1.getIp().is_v6() && v2.getIp().is_v6());
}

bool check_vhosts_validity()
{
    auto& vhosts = Config::getInstance().get_vhost();
    for (size_t i = 0; i < vhosts.size(); i++)
    {
        for (size_t j = i + 1; j < vhosts.size(); j++)
        {
            // check if a there is not duplicat for vhost
            if (is_same_ip_type(*vhosts[i], *vhosts[j])
                && (vhosts[i]->getIp() == vhosts[j]->getIp()
                    || is_wildcard_ip(vhosts[i]->getIp())
                    || is_wildcard_ip(vhosts[j]->getIp()))
                && vhosts[i]->getPort() == vhosts[j]->getPort()
                && vhosts[i]->getServerName() == vhosts[j]->getServerName())
            {
                std::clog << "In File => utils.cc: ";
                std::clog << "check_vhosts_validity(): VHosts must be "
                             "differentiable.\n";
                return false;
            }

            // check if a default vhost has not duplicat
            if (!is_default_vhost_unique(*vhosts[i], *vhosts[j]))
            {
                std::clog << "In File => utils.cc: ";
                std::clog
                    << "check_vhosts_validity(): two vhost with the same ip "
                       "and port are the default vhost\n";
                return false;
            }
        }

        // check if the default vhost does not have the wildcard ip
        if (vhosts[i]->isDefaultVhost() && is_wildcard_ip(vhosts[i]->getIp()))
        {
            std::clog << "In File => utils.cc: ";
            std::clog << "check_vhosts_validity(): the default vhost cannot "
                         "have the wildcard ip\n";
            return false;
        }
    }
    return true;
}

bool is_default_vhost_unique(const VHost& v1, VHost& v2)
{
    if (!v1.isDefaultVhost() || !v2.isDefaultVhost())
        return true;

    if (v1.getIp() == v2.getIp() && v1.getPort() == v2.getPort())
        return false;

    return true;
}

// =================== request utils =================== //

bool valid_method(const std::string& method)
{
    return method == GET || method == PUT || method == HEAD || method == DELETE;
}

std::string get_next_token(const std::string& request, size_t* cur_pos,
                           const std::string& delimiter)
{
    std::string sub_string = request.substr(*cur_pos, request.length());

    auto delimiter_pos = sub_string.find(delimiter);
    if (delimiter_pos == std::string::npos)
        return "";

    std::string token = sub_string.substr(0, delimiter_pos);
    *cur_pos += token.length() + delimiter.length();
    return token;
}

std::string remove_bracket(const std::string& word)
{
    std::string result;
    for (char ch : word)
    {
        if (ch != '[' && ch != ']')
            result += ch;
    }
    return result;
}

void to_lower_case(std::string& word)
{
    std::transform(word.begin(), word.end(), word.begin(),
                   [](unsigned char c) { return std::tolower(c); });
}

std::string remove_space_at_begin_and_at_the_end(const std::string& word)
{
    size_t i = 0;
    while (i < word.length() && (word[i] == ' ' || word[i] == '\t'))
        i++;

    int j = word.length() - 1;
    while (j >= 0 && (word[j] == ' ' || word[j] == '\t'))
        j--;

    return word.substr(i, j - i + 1);
}

bool contains_space(const std::string& word)
{
    for (auto c : word)
    {
        if (isspace(c))
            return true;
    }
    return false;
}

std::string get_access_path(const std::string& root, const std::string& path)
{
    if (root.empty() || path.empty())
        return std::string(root + path);

    if (root[root.length() - 1] != '/' && path[0] != '/')
        return std::string(root + "/" + path);

    auto root_path = std::string(root + path);
    std::string res;
    bool prev = false;
    for (size_t i = 0; i < root_path.length(); i++)
    {
        if (root_path[i] == '/')
        {
            if (!prev)
                res += root_path[i];
            prev = true;
        }
        else
        {
            res += root_path[i];
            prev = false;
        }
    }
    return res;
}

std::pair<std::string, std::string> get_key_and_value(const std::string& header)
{
    size_t cur_pos = 0;
    std::string colon = ":";
    std::string key = get_next_token(header, &cur_pos, colon);
    std::string cpy = key;

    to_lower_case(key);

    if (key == "host")
        cpy = "Host";

    if (key == "content-length")
        cpy = "Content-Length";

    std::string value_with_space = header.substr(cur_pos, header.length());
    std::string value = remove_space_at_begin_and_at_the_end(value_with_space);

    auto res = std::pair<std::string, std::string>(cpy, value);
    return res;
}